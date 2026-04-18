#include "arm_fsm.h"
#include "arm_fsm_internal.h"
#include "States/arm_state_handlers.h"
#include "supervisor_fsm.h"
#include "robot_state.h"
#include "debug_module.h"
#include "arm_fsm_internal.h"

/* Internal State */
static ArmState_t arm_state = STATE_ARM_UNKNOWN;

/* Shared targets (declared extern in arm_fsm_internal.h) */
float target_j1 = 0.0f;
float target_j2 = 0.0f;
float target_j3 = 0.0f;
uint8_t homing_progress = 0;

const char* FSM_Arm_StateToStr(ArmState_t state) {
    switch(state) {
        case STATE_ARM_INIT:     return "INIT";
        case STATE_ARM_HOMING:   return "HOMING";
        case STATE_ARM_IDLE:     return "IDLE";
        case STATE_ARM_MOVING:   return "MOVING";
        case STATE_ARM_TESTING:  return "TESTING";
        case STATE_ARM_FAULT:    return "FAULT";
        default:                 return "UNKNOWN";
    }
}

/**
 * @brief Helper to handle state transitions with entry/exit actions.
 */
void FSM_Arm_TransitionToState(ArmState_t newState) {
    if (arm_state == newState) return;

    /* Call Exit Handler of current state */
    switch (arm_state) {
        case STATE_ARM_INIT:     ArmState_Init_OnExit();     break;
        case STATE_ARM_HOMING:   ArmState_Homing_OnExit();   break;
        case STATE_ARM_IDLE:     ArmState_Idle_OnExit();     break;
        case STATE_ARM_MOVING:   ArmState_Moving_OnExit();   break;
        case STATE_ARM_TESTING:  ArmState_Testing_OnExit();  break;
        case STATE_ARM_FAULT:    ArmState_Fault_OnExit();    break;
        default: break;
    }

    arm_state = newState;
    RobotState_SetArmState(arm_state);

    /* Call Enter Handler of new state */
    switch (arm_state) {
        case STATE_ARM_INIT:     ArmState_Init_OnEnter();     break;
        case STATE_ARM_HOMING:   ArmState_Homing_OnEnter();   break;
        case STATE_ARM_IDLE:     ArmState_Idle_OnEnter();     break;
        case STATE_ARM_MOVING:   ArmState_Moving_OnEnter();   break;
        case STATE_ARM_TESTING:  ArmState_Testing_OnEnter();  break;
        case STATE_ARM_FAULT:    ArmState_Fault_OnEnter();    break;
        default: break;
    }
}

void FSM_Arm_Init(void) {
    arm_state = STATE_ARM_INIT;
    target_j1 = 0.0f;
    target_j2 = 0.0f;
    target_j3 = 0.0f;
    homing_progress = 0;
    
    RobotState_SetArmState(arm_state);
    
    /* Start in INIT state */
    FSM_Arm_TransitionToState(arm_state);
    
    LOG_INFO(LOG_TAG, "Initialized (Standardized FSM)\r\n");
}

ArmState_t FSM_Arm_GetCurrentState(void) {
    return arm_state;
}

void FSM_Arm_SetJointTarget(float j1, float j2, float j3) {
    target_j1 = j1;
    target_j2 = j2;
    target_j3 = j3;
}

void FSM_Arm_SetRawServoPulse(uint8_t servo_id, int16_t pulse) {
    /* Auto-transition to TESTING state if not already there */
    if (arm_state != STATE_ARM_TESTING) {
        FSM_Arm_ProcessEvent(EVENT_ARM_TESTING);
    }
    
    /* 
       TODO: Forward pulse command to specific servo driver.
       Since we don't have a shared 'commands' field for raw servos yet,
       we LOG it for now or prepare a global shared flag.
    */
    LOG_INFO(LOG_TAG, "Testing Servo %u -> Pulse %d\r\n", servo_id, pulse);
}

void FSM_Arm_ProcessEvent(ArmEvent_t event) {
    ArmState_t nextState = arm_state;

    /* Transition Table Logic */
    switch (arm_state)
    {
        case STATE_ARM_INIT:
            if (event == EVENT_ARM_IDLE) nextState = STATE_ARM_IDLE;
            else if (event == EVENT_ARM_FAULT) nextState = STATE_ARM_FAULT;
            break;

        case STATE_ARM_IDLE:
            if (event == EVENT_ARM_MOVING) nextState = STATE_ARM_MOVING;
            else if (event == EVENT_ARM_HOMING) nextState = STATE_ARM_HOMING;
            else if (event == EVENT_ARM_TESTING) nextState = STATE_ARM_TESTING;
            else if (event == EVENT_ARM_FAULT) nextState = STATE_ARM_FAULT;
            break;

        case STATE_ARM_HOMING:
            if (event == EVENT_ARM_IDLE) nextState = STATE_ARM_IDLE;
            else if (event == EVENT_ARM_MOVING) nextState = STATE_ARM_MOVING;
            else if (event == EVENT_ARM_FAULT) nextState = STATE_ARM_FAULT;
            break;

        case STATE_ARM_MOVING:
            if (event == EVENT_ARM_IDLE) nextState = STATE_ARM_IDLE;
            else if (event == EVENT_ARM_HOMING) nextState = STATE_ARM_HOMING;
            else if (event == EVENT_ARM_TESTING) nextState = STATE_ARM_TESTING;
            else if (event == EVENT_ARM_FAULT) nextState = STATE_ARM_FAULT;
            break;

        case STATE_ARM_TESTING:
            if (event == EVENT_ARM_IDLE) nextState = STATE_ARM_IDLE;
            else if (event == EVENT_ARM_FAULT) nextState = STATE_ARM_FAULT;
            break;

        case STATE_ARM_FAULT:
            if (event == EVENT_ARM_INIT) nextState = STATE_ARM_INIT;
            break;

        default:
            LOG_ERROR(LOG_TAG, "Arm: Invalid state transition\r\n");
            nextState = STATE_ARM_FAULT;
            RobotState_SetErrorFlag(ERR_INVALID_ARM_EVENT);
            break;
    }

    if (nextState != arm_state) {
        LOG_INFO(LOG_TAG, "Arm: Transition from %s to %s (Event: %d)\r\n", 
            FSM_Arm_StateToStr(arm_state), 
            FSM_Arm_StateToStr(nextState), event);
        FSM_Arm_TransitionToState(nextState);
    }
}

/**
 * @brief Main logic loop for Arm FSM. Called periodically by its RTOS Task.
 */
void FSM_Arm_ProcessLogic(void) {
    SystemState_t master_state = Supervisor_GetCurrentState();

    /* 1. TOP-DOWN Override: React to Master FSM via Events */
    if (master_state == STATE_SUPERVISOR_FAULT || master_state == STATE_SUPERVISOR_INIT) {
        if (arm_state != STATE_ARM_INIT) {
            FSM_Arm_ProcessEvent(EVENT_ARM_INIT);
        }
    } else if (master_state == STATE_SUPERVISOR_PAUSED || master_state == STATE_SUPERVISOR_IDLE) {
        if (arm_state == STATE_ARM_MOVING || arm_state == STATE_ARM_HOMING) {
            FSM_Arm_ProcessEvent(EVENT_ARM_IDLE);
        }
    }

    /* 2. Sync targets from shared state */
    RobotState_GetTargetArmPose(&target_j1, &target_j2, &target_j3);

    /* 3. Execute Current State Periodic Logic */
    switch (arm_state) {
        case STATE_ARM_INIT:     ArmState_Init_Run();     break;
        case STATE_ARM_HOMING:   ArmState_Homing_Run();   break;
        case STATE_ARM_IDLE:     ArmState_Idle_Run();     break;
        case STATE_ARM_MOVING:   ArmState_Moving_Run();   break;
        case STATE_ARM_TESTING:  ArmState_Testing_Run();  break;
        case STATE_ARM_FAULT:    ArmState_Fault_Run();    break;
        default: break;
    }

    /* Sync local state to global RobotState */
    RobotState_SetArmState(arm_state);
}
