#include "supervisor_fsm.h"
#include "States/state_handlers.h"
#include "arm_fsm.h"
#include "robot_state.h"
#include "debug_module.h"
#include "osal.h"
#include <stdio.h>
#include "error_codes.h"
//Subsystem
#include "mobility_fsm.h"
#include "arm_fsm.h"

/* Watchdog Variables (Moved from task_manager.c) */
static uint8_t prev_mob_wdg = 0;
static uint8_t prev_arm_wdg = 0;
static uint32_t last_mob_tick = 0;
static uint32_t last_arm_tick = 0;

/* Internal Private State */
static SystemState_t currentState = STATE_SUPERVISOR_INIT;
static SystemState_t previousMode = STATE_SUPERVISOR_IDLE; /* To restore after Pause */
static uint8_t pauseAuthLevel = 0;              /* Authority level that triggered the pause */

const char* Supervisor_StateToStr(SystemState_t state) {
    switch(state) {
        case STATE_SUPERVISOR_INIT:   return "INIT";
        case STATE_SUPERVISOR_IDLE:   return "IDLE";
        case STATE_SUPERVISOR_MANUAL: return "MANUAL";
        case STATE_SUPERVISOR_AUTO:   return "AUTO";
        case STATE_SUPERVISOR_PAUSED: return "PAUSED";
        case STATE_SUPERVISOR_FAULT:  return "FAULT";
        default:           return "UNKNOWN";
    }
}
/**
 * @brief Standard safety checks (Watchdogs and Global Errors).
 * This logic has been centralized here from the legacy task_manager.c.
 */
void Supervisor_RunStandardChecks(void) {
    uint32_t now = osal_get_tick();

    /* 1. Mobility Subsystem Check */
    // Get mobility watchdog
    uint8_t current_mob_wdg = RobotState_GetWatchdogMobility();
    if (current_mob_wdg != prev_mob_wdg) {
        last_mob_tick = now;
        prev_mob_wdg = current_mob_wdg;
    } else {
        if (now - last_mob_tick >= TIMEOUT_SUPERVISOR_ERROR_MS) {
            RobotState_SetErrorFlag(ERR_MOB_STALL);
        }
    }
    // Get mobility state
    MobilityState_t mobilityState = RobotState_GetMobilityState();
    if (mobilityState == STATE_MOB_FAULT) {
        RobotState_SetErrorFlag(ERR_MOB_FAULT);
    }

    /* 2. Arm Subsystem Check */
    // Get arm watchdog
    uint8_t current_arm_wdg = RobotState_GetWatchdogArm();
    if (current_arm_wdg != prev_arm_wdg) {
        last_arm_tick = now;
        prev_arm_wdg = current_arm_wdg;
    } else {
        if (now - last_arm_tick >= TIMEOUT_SUPERVISOR_ERROR_MS) {
            RobotState_SetErrorFlag(ERR_ARM_STALL);
        }
    }
    // Get arm state
    ArmState_t armState = RobotState_GetArmState();
    if (armState == STATE_ARM_FAULT) {
        RobotState_SetErrorFlag(ERR_ARM_FAULT);
    }

    /* 3. Global Error Registry Evaluation */
    uint64_t current_errors = RobotState_GetErrorFlags();
    if (ERR_IS_ANY(current_errors)) {
        LOG_ERROR(LOG_TAG, "FAULT (Flags: 0x%08lX%08lX). Triggering System FAULT.\r\n",
               (unsigned long)(current_errors >> 32), (unsigned long)(current_errors & 0xFFFFFFFF));
        Supervisor_ProcessEvent(EVENT_SUPERVISOR_ERROR, SRC_INTERNAL_SUPERVISOR);
    }
}

/**
 * @brief Helper to handle state transitions with entry/exit actions.
 */
static void TransitionToState(SystemState_t newState) {
    if (currentState == newState) return;

    /* Call Exit Handler of current state */
    switch (currentState) {
        case STATE_SUPERVISOR_INIT:   State_Init_OnExit();   break;
        case STATE_SUPERVISOR_IDLE:   State_Idle_OnExit();   break;
        case STATE_SUPERVISOR_MANUAL: State_Manual_OnExit(); break;
        case STATE_SUPERVISOR_AUTO:   State_Auto_OnExit();   break;
        case STATE_SUPERVISOR_PAUSED: State_Paused_OnExit(); break;
        case STATE_SUPERVISOR_FAULT:  State_Fault_OnExit();  break;
        default: break;
    }

    currentState = newState;
    RobotState_UpdateSystemState(currentState);

    /* Call Enter Handler of new state */
    switch (currentState) {
        case STATE_SUPERVISOR_INIT:   State_Init_OnEnter();   break;
        case STATE_SUPERVISOR_IDLE:   State_Idle_OnEnter();   break;
        case STATE_SUPERVISOR_MANUAL: State_Manual_OnEnter(); break;
        case STATE_SUPERVISOR_AUTO:   State_Auto_OnEnter();   break;
        case STATE_SUPERVISOR_PAUSED: State_Paused_OnEnter(); break;
        case STATE_SUPERVISOR_FAULT:  State_Fault_OnEnter();  break;
        default: break;
    }
}

/**
 * @brief Initialize the supervisor state machine.
 */
void Supervisor_Init(void) {
    currentState = STATE_SUPERVISOR_INIT;
    uint32_t now = osal_get_tick();
    last_mob_tick = now;
    last_arm_tick = now;
    
    RobotState_UpdateSystemState(currentState);
    State_Init_OnEnter();
    LOG_INFO(LOG_TAG, "Supervisor: Initialized\r\n");
}

/**
 * @brief Get the current state.
 */
SystemState_t Supervisor_GetCurrentState(void) {
    return currentState;
}

/**
 * @brief Process an incoming event and update the supervisor state machine.
 */
void Supervisor_ProcessEvent(SystemEvent_t event, uint8_t source) {
    SystemState_t nextState = currentState;

    /* Transition Table Logic */
    switch (currentState)
    {
        case STATE_SUPERVISOR_INIT:
            if (event == EVENT_SUPERVISOR_ERROR) nextState = STATE_SUPERVISOR_FAULT;
            else if (event == EVENT_SUPERVISOR_READY) nextState = STATE_SUPERVISOR_IDLE;
            break;

        case STATE_SUPERVISOR_IDLE:
            if (event == EVENT_SUPERVISOR_ERROR) nextState = STATE_SUPERVISOR_FAULT;
            else if (event == EVENT_SUPERVISOR_START) nextState = STATE_SUPERVISOR_MANUAL;
            break;
        
        case STATE_SUPERVISOR_MANUAL:
            if (event == EVENT_SUPERVISOR_STOP) nextState = STATE_SUPERVISOR_IDLE;
            else if (event == EVENT_SUPERVISOR_MODE_AUTO) nextState = STATE_SUPERVISOR_AUTO;
            else if (event == EVENT_SUPERVISOR_ERROR) nextState = STATE_SUPERVISOR_FAULT;
            else if (event == EVENT_SUPERVISOR_PAUSE) {
                previousMode = STATE_SUPERVISOR_MANUAL;
                pauseAuthLevel = source;
                nextState = STATE_SUPERVISOR_PAUSED;
            }
            break;

        case STATE_SUPERVISOR_AUTO:
            if (event == EVENT_SUPERVISOR_STOP) nextState = STATE_SUPERVISOR_IDLE;
            else if (event == EVENT_SUPERVISOR_MODE_MANUAL) nextState = STATE_SUPERVISOR_MANUAL;
            else if (event == EVENT_SUPERVISOR_ERROR) nextState = STATE_SUPERVISOR_FAULT;
            else if (event == EVENT_SUPERVISOR_PAUSE) {
                previousMode = STATE_SUPERVISOR_AUTO;
                pauseAuthLevel = source;
                nextState = STATE_SUPERVISOR_PAUSED;
            }
            break;

        case STATE_SUPERVISOR_PAUSED:
            if (event == EVENT_SUPERVISOR_STOP) nextState = STATE_SUPERVISOR_IDLE;
            else if (event == EVENT_SUPERVISOR_ERROR) nextState = STATE_SUPERVISOR_FAULT;
            else if (event == EVENT_SUPERVISOR_RESUME) {
                // Hierarchical Authority Check
                if (source >= pauseAuthLevel) {
                    nextState = previousMode; // Restore where we were
                } else {
                    LOG_WARNING(LOG_TAG, "Supervisor: Resume REJECTED. Source (%d) lower than Auth (%d)\r\n", source, pauseAuthLevel);
                }
            }
            break;

        case STATE_SUPERVISOR_FAULT:
            if (event == EVENT_SUPERVISOR_RESET) nextState = STATE_SUPERVISOR_INIT;
            break;

        default:
            LOG_ERROR(LOG_TAG, "Supervisor: Invalid state transition\r\n");
            nextState = STATE_SUPERVISOR_FAULT;
            RobotState_SetErrorFlag(ERR_INVALID_SUPERVISOR_STATE);
            break;
    }

    if (nextState != currentState) {
        LOG_INFO(LOG_TAG, "Supervisor: Transitioning from %s to %s\r\n", Supervisor_StateToStr(currentState), Supervisor_StateToStr(nextState));
        TransitionToState(nextState);
    }
}

/**
 * @brief Periodic logic for the supervisor. Called by RTOS Task.
 */
void Supervisor_ProcessLogic(void) {
    /* 1. Global Safety Checks (Standard for all operational states) */
    if (currentState != STATE_SUPERVISOR_INIT && currentState != STATE_SUPERVISOR_FAULT) {
        Supervisor_RunStandardChecks();
    }

    /* 2. Execute Current State Periodic Logic */
    switch (currentState) {
        case STATE_SUPERVISOR_INIT:   State_Init_Run();   break;
        case STATE_SUPERVISOR_IDLE:   State_Idle_Run();   break;
        case STATE_SUPERVISOR_MANUAL: State_Manual_Run(); break;
        case STATE_SUPERVISOR_AUTO:   State_Auto_Run();   break;
        case STATE_SUPERVISOR_PAUSED: State_Paused_Run(); break;
        case STATE_SUPERVISOR_FAULT:  State_Fault_Run();  break;
        default: break;
    }
}

