#include "supervisor_fsm.h"
#include "States/state_handlers.h"
#include "arm_fsm.h"
#include "osal.h"
#include "robot_state.h"
#include "debug_module.h"
#include "usb_joystick.h"
#include "mobility_fsm.h"
#include "arm_fsm.h"
#include "app_rtos.h"
#include "supervisor_hw_joy_input.h"

/* Internal Private State */
static SystemState_t current_state = STATE_SUPERVISOR_INIT;
static SystemState_t previous_mode = STATE_SUPERVISOR_INIT;  /* To restore after Pause */
static uint8_t pause_auth_level = 0;                          /* Authority level that triggered the pause */

/**
 * @brief Unified interface to request a state change (Asynchronous).
 */
void Supervisor_SendEvent(SystemEvent_t event, uint8_t source) {
    /* 1. Basic Business Rules / Filtering */
    
    /* Gamepad (SRC_PHYSICAL) is only allowed in MANUAL/IDLE/FAULT */
    if (source == SRC_PHYSICAL) {
        if (current_state == STATE_SUPERVISOR_AUTO) {
            LOG_WARNING(LOG_TAG, "Joystick command ignored while in AUTO mode\r\n");
            return;
        }
    }

    /* External Client (SRC_EXT_CLIENT) logic */
    if (source == SRC_EXT_CLIENT) {
        /* Absolute Isolation Check: Is Autonomous Mode allowed by HW? */
        if (!RobotState_GetEnableAutonomous()) {
            LOG_WARNING(LOG_TAG, "EXTERNAL COMMAND REJECTED: Autonomous Mode is HARDWARE ISOLATED\r\n");
            return;
        }

        if (current_state == STATE_SUPERVISOR_MANUAL) {
            LOG_WARNING(LOG_TAG, "External command ignored while in MANUAL mode\r\n");
            return;
        }
    }

    /* 2. Enqueue the event */
    StateChangeMsg_t msg;
    msg.event = event;
    msg.timestamp = osal_get_tick();
    msg.source = source;

    if (osal_queue_put(stateMsgQueueHandle, &msg, 0U) != OSAL_OK) {
        LOG_ERROR(LOG_TAG, "Supervisor: Failed to queue event %d from source %d\r\n", event, source);
    }
}

/**
 * @brief Convert state enum to string for logging.
 */
const char* Supervisor_StateToStr(SystemState_t state) {
    switch(state) {
        case STATE_SUPERVISOR_INIT:   return "INIT";
        case STATE_SUPERVISOR_IDLE:   return "IDLE";
        case STATE_SUPERVISOR_MANUAL: return "MANUAL";
        case STATE_SUPERVISOR_AUTO:   return "AUTO";
        case STATE_SUPERVISOR_PAUSED: return "PAUSED";
        case STATE_SUPERVISOR_FAULT:  return "FAULT";
        case STATE_SUPERVISOR_TESTING:return "TESTING";
        default:           return "UNKNOWN";
    }
}

/**
 * @brief Standard safety checks (Watchdogs and Global Errors).
 * This logic has been centralized here from the legacy task_manager.c.
 */
void Supervisor_RunStandardChecks(void) {
    uint32_t now = osal_get_tick();

    /* 1. Mobility Subsystem Check (Passive Timestamp Watchdog) */
    // Check if the mobility task is alive
    if (now - RobotState_GetMobilityHeartbeat() > TIMEOUT_SUPERVISOR_ERROR_MS) {
        RobotState_SetErrorFlag(ERR_MOB_STALL);
        LOG_ERROR(LOG_TAG, "Mobility Task STALL! No heartbeat for >%d ms\r\n", TIMEOUT_SUPERVISOR_ERROR_MS);
    }
    // Check if the mobility state is fault
    MobilityState_t mobility_state = RobotState_GetMobilityState();
    if (mobility_state == STATE_MOB_FAULT) {
        RobotState_SetErrorFlag(ERR_MOB_FAULT);
    }

    /* 2. Arm Subsystem Check (Passive Timestamp Watchdog) */
    // Check if the arm task is alive
    if (now - RobotState_GetArmHeartbeat() > TIMEOUT_SUPERVISOR_ERROR_MS) {
        RobotState_SetErrorFlag(ERR_ARM_STALL);
        LOG_ERROR(LOG_TAG, "Arm Task STALL! No heartbeat for >%d ms\r\n", TIMEOUT_SUPERVISOR_ERROR_MS);
    }
    // Check if the arm state is fault
    ArmState_t arm_state = RobotState_GetArmState();
    if (arm_state == STATE_ARM_FAULT) {
        RobotState_SetErrorFlag(ERR_ARM_FAULT);
    }

    /* 3. Global Error Registry Evaluation */
    uint64_t current_errors = RobotState_GetErrorFlags();
    if (ERR_IS_ANY(current_errors)) {
        LOG_ERROR(LOG_TAG, "FAULT (Flags: 0x%08lX%08lX). Triggering System FAULT.\r\n",
               (unsigned long)(current_errors >> 32), (unsigned long)(current_errors & 0xFFFFFFFF));
        Supervisor_ProcessEvent(EVENT_SUPERVISOR_ERROR, SRC_INTERNAL_SUPERVISOR);
    }

    /* 4. Cross-Subsystem Abort Logic */
    // If any system error is present, command healthy subsystems to stop safely.
    if (ERR_IS_ANY(current_errors)) {
        if (mobility_state != STATE_MOB_FAULT && mobility_state != STATE_MOB_ABORT) {
            FSM_Mobility_ProcessEvent(EVENT_MOB_ABORT);
        }
        if (arm_state != STATE_ARM_FAULT && arm_state != STATE_ARM_ABORT) {
            FSM_Arm_ProcessEvent(EVENT_ARM_ABORT);
        }
    }
}

/**
 * @brief Helper to handle state transitions with entry/exit actions.
 */
static void TransitionToState(SystemState_t new_state) {
    if (current_state == new_state) return;

    /* Call Exit Handler of current state */
    switch (current_state) {
        case STATE_SUPERVISOR_INIT:     State_Init_OnExit();   break;
        case STATE_SUPERVISOR_IDLE:     State_Idle_OnExit();   break;
        case STATE_SUPERVISOR_MANUAL:   State_Manual_OnExit(); break;
        case STATE_SUPERVISOR_AUTO:     State_Auto_OnExit();   break;
        case STATE_SUPERVISOR_PAUSED:   State_Paused_OnExit(); break;
        case STATE_SUPERVISOR_FAULT:    State_Fault_OnExit();  break;
        case STATE_SUPERVISOR_TESTING:  State_Testing_OnExit();break;
        default: break;
    }

    current_state = new_state;
    RobotState_UpdateSystemState(current_state);

    /* Call Enter Handler of new state */
    switch (current_state) {
        case STATE_SUPERVISOR_INIT:     State_Init_OnEnter();   break;
        case STATE_SUPERVISOR_IDLE:     State_Idle_OnEnter();   break;
        case STATE_SUPERVISOR_MANUAL:   State_Manual_OnEnter(); break;
        case STATE_SUPERVISOR_AUTO:     State_Auto_OnEnter();   break;
        case STATE_SUPERVISOR_PAUSED:   State_Paused_OnEnter(); break;
        case STATE_SUPERVISOR_FAULT:    State_Fault_OnEnter();  break;
        case STATE_SUPERVISOR_TESTING:  State_Testing_OnEnter();break;
        default: break;
    }
}

/**
 * @brief Initialize the supervisor state machine.
 */
void Supervisor_Init(void) {
    
    /* Initialize heartbeats to current time to avoid immediate stall detection */
    RobotState_UpdateMobilityHeartbeat();
    RobotState_UpdateArmHeartbeat();
    
    LOG_INFO(LOG_TAG, "Supervisor: Initialized\r\n");
}

/**
 * @brief Get the current state.
 */
SystemState_t Supervisor_GetCurrentState(void) {
    return current_state;
}

/**
 * @brief Process an incoming event and update the supervisor state machine.
 */
void Supervisor_ProcessEvent(SystemEvent_t event, uint8_t source) {
    LOG_INFO(LOG_TAG, "Event Received: %d from Source: %d (Current State: %s)\r\n", event, source, Supervisor_StateToStr(current_state));
    SystemState_t next_state = current_state;

    /* Transition Table Logic */
    switch (current_state)
    {
        case STATE_SUPERVISOR_INIT:
            if (event == EVENT_SUPERVISOR_ERROR) next_state = STATE_SUPERVISOR_FAULT;
            else if (event == EVENT_SUPERVISOR_READY) next_state = STATE_SUPERVISOR_IDLE;
            break;

        case STATE_SUPERVISOR_IDLE:
            if (event == EVENT_SUPERVISOR_ERROR) next_state = STATE_SUPERVISOR_FAULT;
            else if (event == EVENT_SUPERVISOR_START) {
                if (source == SRC_EXT_CLIENT) {
                    next_state = STATE_SUPERVISOR_AUTO;
                } else {
                    next_state = STATE_SUPERVISOR_MANUAL;
                }
            }
            break;
        
        case STATE_SUPERVISOR_MANUAL:
            if (event == EVENT_SUPERVISOR_STOP) next_state = STATE_SUPERVISOR_IDLE;
            else if (event == EVENT_SUPERVISOR_MODE_AUTO) next_state = STATE_SUPERVISOR_AUTO;
            else if (event == EVENT_SUPERVISOR_TESTING) next_state = STATE_SUPERVISOR_TESTING;
            else if (event == EVENT_SUPERVISOR_ERROR) next_state = STATE_SUPERVISOR_FAULT;
            else if (event == EVENT_SUPERVISOR_PAUSE) {
                previous_mode = STATE_SUPERVISOR_MANUAL;
                pause_auth_level = source;
                next_state = STATE_SUPERVISOR_PAUSED;
            }
            break;

        case STATE_SUPERVISOR_AUTO:
            if (event == EVENT_SUPERVISOR_STOP) next_state = STATE_SUPERVISOR_IDLE;
            else if (event == EVENT_SUPERVISOR_MODE_MANUAL) next_state = STATE_SUPERVISOR_MANUAL;
            else if (event == EVENT_SUPERVISOR_TESTING) next_state = STATE_SUPERVISOR_TESTING;
            else if (event == EVENT_SUPERVISOR_ERROR) next_state = STATE_SUPERVISOR_FAULT;
            else if (event == EVENT_SUPERVISOR_PAUSE) {
                previous_mode = STATE_SUPERVISOR_AUTO;
                pause_auth_level = source;
                next_state = STATE_SUPERVISOR_PAUSED;
            }
            break;

        case STATE_SUPERVISOR_PAUSED:
            if (event == EVENT_SUPERVISOR_STOP) next_state = STATE_SUPERVISOR_IDLE;
            else if (event == EVENT_SUPERVISOR_ERROR) next_state = STATE_SUPERVISOR_FAULT;
            else if (event == EVENT_SUPERVISOR_RESUME) {
                // Hierarchical Authority Check
                if (source >= pause_auth_level) {
                    next_state = previous_mode; // Restore where we were
                } else {
                    LOG_WARNING(LOG_TAG, "Supervisor: Resume REJECTED. Source (%d) lower than Auth (%d)\r\n", source, pause_auth_level);
                }
            }
            break;

        case STATE_SUPERVISOR_TESTING:
            if (event == EVENT_SUPERVISOR_STOP) next_state = STATE_SUPERVISOR_IDLE;
            else if (event == EVENT_SUPERVISOR_ERROR) next_state = STATE_SUPERVISOR_FAULT;
            break;

        case STATE_SUPERVISOR_FAULT:
            if (event == EVENT_SUPERVISOR_RESET) next_state = STATE_SUPERVISOR_INIT;
            break;

        default:
            LOG_ERROR(LOG_TAG, "Supervisor: Invalid state transition\r\n");
            next_state = STATE_SUPERVISOR_FAULT;
            RobotState_SetErrorFlag(ERR_INVALID_SUPERVISOR_STATE);
            break;
    }

    if (next_state != current_state) {
        LOG_INFO(LOG_TAG, "Supervisor: Transitioning from %s to %s\r\n", Supervisor_StateToStr(current_state), Supervisor_StateToStr(next_state));
        TransitionToState(next_state);
    }
}

/**
 * @brief periodic logic for the supervisor. Called by RTOS Task.
 */
void Supervisor_ProcessLogic(void) {
    /* Feed Supervisor heartbeat to let subsystems know we are alive */
    RobotState_UpdateSupervisorHeartbeat();

    /* 0. Handle Joystick Inputs */
    Supervisor_HandleJoystickInput();

    /* 1. Global Safety Checks (Standard for all operational states) */
    if (current_state != STATE_SUPERVISOR_INIT && current_state != STATE_SUPERVISOR_FAULT) {
        Supervisor_RunStandardChecks();
    }

    /* 2. Execute Current State Periodic Logic */
    switch (current_state) {
        case STATE_SUPERVISOR_INIT:     State_Init_Run();   break;
        case STATE_SUPERVISOR_IDLE:     State_Idle_Run();   break;
        case STATE_SUPERVISOR_MANUAL:   State_Manual_Run(); break;
        case STATE_SUPERVISOR_AUTO:     State_Auto_Run();   break;
        case STATE_SUPERVISOR_PAUSED:   State_Paused_Run(); break;
        case STATE_SUPERVISOR_FAULT:    State_Fault_Run();  break;
        case STATE_SUPERVISOR_TESTING:  State_Testing_Run();break;
        default: break;
    }
}

