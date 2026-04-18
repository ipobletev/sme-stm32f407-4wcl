#include "mobility_fsm.h"
#include "mobility_fsm_internal.h"
#include "States/mob_state_handlers.h"
#include "supervisor_fsm.h"
#include "robot_state.h"
#include "debug_module.h"
#include "osal.h"
#include <stdio.h>
#include <math.h>

/* Motor Objects visible to states */
static EncoderMotorObjectTypeDef motor_instances[4];
EncoderMotorObjectTypeDef *motors[4] = {
    &motor_instances[0], 
    &motor_instances[1], 
    &motor_instances[2], 
    &motor_instances[3]
};

/* Shared targets */
float target_linear_x = 0.0f;
float target_angular_z = 0.0f;

/* Internal State */
static MobilityState_t mob_state = STATE_MOB_INIT;

const char* FSM_Mobility_StateToStr(MobilityState_t state) {
    switch(state) {
        case STATE_MOB_INIT:     return "INIT";
        case STATE_MOB_IDLE:     return "IDLE";
        case STATE_MOB_BREAK:    return "BREAK";
        case STATE_MOB_MOVING:   return "MOVING";
        case STATE_MOB_TESTING:  return "TESTING";
        case STATE_MOB_FAULT:    return "FAULT";
        case STATE_MOB_ABORT:    return "ABORT";
        default:                 return "UNKNOWN";
    }
}

const char* FSM_Mobility_ModeToStr(uint8_t mode) {
    switch(mode) {
        case MOB_MODE_DIRECT:    return "DIR";
        case MOB_MODE_DIFF:      return "DIF";
        case MOB_MODE_ACKERMANN: return "ACK";
        case MOB_MODE_MECANUM:   return "MEC";
        default:                 return "UNK";
    }
}

/**
 * @brief Helper to handle state transitions with entry/exit actions.
 */
void FSM_Mobility_TransitionToState(MobilityState_t newState) {
    if (mob_state == newState) return;

    /* Call Exit Handler of current state */
    switch (mob_state) {
        case STATE_MOB_INIT:     MobState_Init_OnExit();     break;
        case STATE_MOB_IDLE:     MobState_Idle_OnExit();     break;
        case STATE_MOB_BREAK:    MobState_Break_OnExit();    break;
        case STATE_MOB_MOVING:   MobState_Moving_OnExit();   break;
        case STATE_MOB_TESTING:  MobState_Testing_OnExit();  break;
        case STATE_MOB_FAULT:    MobState_Fault_OnExit();    break;
        case STATE_MOB_ABORT:    MobState_Abort_OnExit();    break;
        default: break;
    }

    mob_state = newState;
    RobotState_SetMobilityState(mob_state);

    /* Call Enter Handler of new state */
    switch (mob_state) {
        case STATE_MOB_INIT:     MobState_Init_OnEnter();     break;
        case STATE_MOB_IDLE:     MobState_Idle_OnEnter();     break;
        case STATE_MOB_BREAK:    MobState_Break_OnEnter();    break;
        case STATE_MOB_MOVING:   MobState_Moving_OnEnter();   break;
        case STATE_MOB_TESTING:  MobState_Testing_OnEnter();  break;
        case STATE_MOB_FAULT:    MobState_Fault_OnEnter();    break;
        case STATE_MOB_ABORT:    MobState_Abort_OnEnter();    break;
        default: break;
    }
}

void FSM_Mobility_Init(void) {    
    LOG_INFO(LOG_TAG, "FSM Logic Initialized.\r\n");
}

MobilityState_t FSM_Mobility_GetCurrentState(void) {
    return mob_state;
}

void FSM_Mobility_ProcessEvent(MobilityEvent_t event) {
    MobilityState_t nextState = mob_state;

    /* Transition Table Logic */
    switch (mob_state)
    {
        case STATE_MOB_INIT:
            if (event == EVENT_MOB_FAULT) nextState = STATE_MOB_FAULT;
            else if (event == EVENT_MOB_IDLE) nextState = STATE_MOB_IDLE;
            else if (event == EVENT_MOB_ABORT) nextState = STATE_MOB_ABORT;
            break;

        case STATE_MOB_MOVING:
            if (event == EVENT_MOB_IDLE) nextState = STATE_MOB_IDLE;
            else if (event == EVENT_MOB_BREAK) nextState = STATE_MOB_BREAK;
            else if (event == EVENT_MOB_FAULT) nextState = STATE_MOB_FAULT;
            else if (event == EVENT_MOB_ABORT) nextState = STATE_MOB_ABORT;
            break;

        case STATE_MOB_BREAK:
            if (event == EVENT_MOB_IDLE) nextState = STATE_MOB_IDLE;
            else if (event == EVENT_MOB_MOVING) nextState = STATE_MOB_MOVING;
            else if (event == EVENT_MOB_FAULT) nextState = STATE_MOB_FAULT;
            else if (event == EVENT_MOB_ABORT) nextState = STATE_MOB_ABORT;
            break;

        case STATE_MOB_TESTING:
            if (event == EVENT_MOB_IDLE) nextState = STATE_MOB_IDLE;
            else if (event == EVENT_MOB_FAULT) nextState = STATE_MOB_FAULT;
            else if (event == EVENT_MOB_ABORT) nextState = STATE_MOB_ABORT;
            break;

        case STATE_MOB_FAULT:
            if (event == EVENT_MOB_INIT) nextState = STATE_MOB_INIT;
            break;

        case STATE_MOB_ABORT:
            if (event == EVENT_MOB_INIT) nextState = STATE_MOB_INIT;
            else if (event == EVENT_MOB_FAULT) nextState = STATE_MOB_FAULT;
            break;

        default:
            LOG_ERROR(LOG_TAG, "Mobility: Invalid state transition\r\n");
            nextState = STATE_MOB_FAULT;
            RobotState_SetErrorFlag(ERR_INVALID_MOBILITY_EVENT);
            break;
    }

    if (nextState != mob_state) {
        LOG_INFO(LOG_TAG, "Mobility: Transition from %s to %s (Event: %d)\r\n", 
                 FSM_Mobility_StateToStr(mob_state), FSM_Mobility_StateToStr(nextState), event);
        FSM_Mobility_TransitionToState(nextState);
    }
}

void FSM_Mobility_ProcessLogic(void) {

    /* Execute Current State Periodic Logic */
    switch (mob_state) {
        case STATE_MOB_INIT:     MobState_Init_Run();     break;
        case STATE_MOB_IDLE:     MobState_Idle_Run();     break;
        case STATE_MOB_BREAK:    MobState_Break_Run();    break;
        case STATE_MOB_MOVING:   MobState_Moving_Run();   break;
        case STATE_MOB_TESTING:  MobState_Testing_Run();  break;
        case STATE_MOB_FAULT:    MobState_Fault_Run();    break;
        case STATE_MOB_ABORT:    MobState_Abort_Run();    break;
        default: break;
    }

}
