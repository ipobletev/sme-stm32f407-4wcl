#include "mobility_fsm.h"
#include "mobility_fsm_internal.h"
#include "States/mob_state_handlers.h"
#include "supervisor_fsm.h"
#include "robot_state.h"
#include "debug_module.h"
#include "osal.h"
#include <stdio.h>
#include <math.h>

/* Motor Objects (Visible to states via mobility_fsm_internal.h) */
static EncoderMotorObjectTypeDef mot1, mot2, mot3, mot4;
EncoderMotorObjectTypeDef *motors[4] = {&mot1, &mot2, &mot3, &mot4};

/* Kinematic targets (Visible to states via mobility_fsm_internal.h) */
float target_linear_x = 0.0f;
float target_angular_z = 0.0f;

/* Internal State */
static MobilityState_t mob_state = STATE_MOB_UNKNOWN;

/* Timing */
static uint32_t last_ctrl_tick = 0;
static uint32_t last_meas_tick = 0;

const char* FSM_Mobility_StateToStr(MobilityState_t state) {
    switch(state) {
        case STATE_MOB_INIT:     return "INIT";
        case STATE_MOB_IDLE:     return "IDLE";
        case STATE_MOB_BREAK:    return "BREAK";
        case STATE_MOB_MOVING:   return "MOVING";
        case STATE_MOB_TESTING:  return "TESTING";
        case STATE_MOB_FAULT:    return "FAULT";
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
        default: break;
    }
}

void FSM_Mobility_Init(void) {
    uint32_t now = osal_get_tick();
    last_ctrl_tick = now;
    last_meas_tick = now;
    
    /* Start in INIT state to handle hardware setup */
    FSM_Mobility_TransitionToState(STATE_MOB_INIT);
    
    LOG_INFO(LOG_TAG, "Initializing Module...\r\n");
}

MobilityState_t FSM_Mobility_GetCurrentState(void) {
    return mob_state;
}

void FSM_Mobility_SetCommandTarget(float linear_x, float angular_z) {
    target_linear_x = linear_x;
    target_angular_z = angular_z;
}

void FSM_Mobility_SetRawMotorPulse(uint8_t id, float pulse) {
    if (id < 4) {
        /* Force state to TESTING via Event system */
        FSM_Mobility_ProcessEvent(EVENT_TESTING);
        
        /* Bypass PID by setting pulse directly on the motor hardware */
        motors[id]->set_pulse(motors[id], (int)pulse);
        motors[id]->current_pulse = pulse;
        motors[id]->pid_controller.set_point = 0;

        LOG_INFO(LOG_TAG, "Raw Pulse Received - ID=%u, Pulse=%d (Testing Mode)\r\n", id, (int)pulse);
    }
}

void FSM_Mobility_UpdateMeasurements(void) {
    uint32_t now = osal_get_tick();
    float dt = (float)(now - last_meas_tick) / 1000.0f;
    if (dt <= 0.0f) dt = 0.01f; 
    last_meas_tick = now;

    for(int i=0; i<4; i++) {
        int64_t count = Motor_Hardware_GetEncoderCount(i);
        encoder_update(motors[i], dt, count);
    }

    RobotState_SetEncoderCounts((int32_t)mot1.counter, (int32_t)mot2.counter, 
                               (int32_t)mot3.counter, (int32_t)mot4.counter);
    
    RobotState_SetMeasuredRPS(mot1.rps, mot2.rps, mot3.rps, mot4.rps);

    /* Kinematics Estimation for Telemetry */
    #ifndef M_PI
    #define M_PI 3.14159265358979323846f
    #endif
    float actual_vx = (mot1.rps + mot2.rps - mot3.rps - mot4.rps) / 4.0f * (M_PI * JETAUTO_WHEEL_DIAMETER);
    RobotState_SetMeasuredVelocity(actual_vx, 0.0f);
}

void FSM_Mobility_ProcessEvent(MobilityEvent_t event) {
    MobilityState_t nextState = mob_state;

    /* Transition Table Logic */
    switch (mob_state)
    {
        case STATE_MOB_INIT:
            if (event == EVENT_FAULT) nextState = STATE_MOB_FAULT;
            else if (event == EVENT_IDLE) nextState = STATE_MOB_IDLE;
            break;

        case STATE_MOB_IDLE:
            if (event == EVENT_MOVING) nextState = STATE_MOB_MOVING;
            else if (event == EVENT_BREAK) nextState = STATE_MOB_BREAK;
            else if (event == EVENT_TESTING) nextState = STATE_MOB_TESTING;
            else if (event == EVENT_FAULT) nextState = STATE_MOB_FAULT;
            else if (event == EVENT_INIT) nextState = STATE_MOB_INIT;
            break;

        case STATE_MOB_MOVING:
            if (event == EVENT_IDLE) nextState = STATE_MOB_IDLE;
            else if (event == EVENT_BREAK) nextState = STATE_MOB_BREAK;
            else if (event == EVENT_FAULT) nextState = STATE_MOB_FAULT;
            break;

        case STATE_MOB_BREAK:
            if (event == EVENT_IDLE) nextState = STATE_MOB_IDLE;
            else if (event == EVENT_MOVING) nextState = STATE_MOB_MOVING;
            else if (event == EVENT_FAULT) nextState = STATE_MOB_FAULT;
            break;

        case STATE_MOB_TESTING:
            if (event == EVENT_IDLE) nextState = STATE_MOB_IDLE;
            else if (event == EVENT_FAULT) nextState = STATE_MOB_FAULT;
            break;

        case STATE_MOB_FAULT:
            if (event == EVENT_INIT) nextState = STATE_MOB_INIT;
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
    SystemState_t master_state = Supervisor_GetCurrentState();
    uint32_t now = osal_get_tick();
    float dt = (float)(now - last_ctrl_tick) / 1000.0f;
    if (dt <= 0.0f) dt = 0.02f; /* 50Hz fallback */
    last_ctrl_tick = now;

    /* 1. TOP-DOWN Override & Safety (React to Master FSM via Events) */
    if (master_state == STATE_SUPERVISOR_FAULT || master_state == STATE_SUPERVISOR_INIT) {
        if (mob_state != STATE_MOB_INIT) {
            FSM_Mobility_ProcessEvent(EVENT_INIT);
        }
    } else if (master_state == STATE_SUPERVISOR_IDLE || master_state == STATE_SUPERVISOR_PAUSED) {
        if (mob_state == STATE_MOB_MOVING) {
             FSM_Mobility_ProcessEvent(EVENT_BREAK);
        }
    }

    /* 2. Sync targets from shared state */
    RobotState_GetTargetVelocity(&target_linear_x, &target_angular_z);

    /* 3. Execute Current State Periodic Logic */
    switch (mob_state) {
        case STATE_MOB_INIT:     MobState_Init_Run();     break;
        case STATE_MOB_IDLE:     MobState_Idle_Run();     break;
        case STATE_MOB_BREAK:    MobState_Break_Run();    break;
        case STATE_MOB_MOVING:   MobState_Moving_Run();   break;
        case STATE_MOB_TESTING:  MobState_Testing_Run();  break;
        case STATE_MOB_FAULT:    MobState_Fault_Run();    break;
        default: break;
    }

    /* 4. Execute Motor Control (PID) - Common to all active states except TESTING */
    if (mob_state != STATE_MOB_TESTING && mob_state != STATE_MOB_INIT) {
        for(int i=0; i<4; i++) {
            encoder_motor_control(motors[i], dt);
        }
    }
    
    RobotState_SetMobilityState(mob_state);
}
