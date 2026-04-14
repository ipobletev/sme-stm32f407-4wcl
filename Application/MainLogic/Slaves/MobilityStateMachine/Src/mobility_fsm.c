#include "mobility_fsm.h"
#include "supervisor_fsm.h" /* For Supervisor FSM interaction */
#include "robot_state.h"
#include <stdio.h>

const char* Mobility_StateToStr(MobilityState_t state) {
    switch(state) {
        case MOB_DISABLED: return "DISABLED";
        case MOB_STOPPED:  return "STOPPED";
        case MOB_MOVING:   return "MOVING";
        case MOB_FAULT:    return "FAULT";
        default:           return "UNKNOWN";
    }
}

static MobilityState_t mob_state = MOB_DISABLED;

/* Kinematic targets */
static float target_linear_x = 0.0f;
static float target_angular_z = 0.0f;

void Mobility_Init(void) {
    mob_state = MOB_DISABLED;
    printf("MOBILITY: Initialized (Disabled)\r\n");
}

MobilityState_t Mobility_GetCurrentState(void) {
    return mob_state;
}

void Mobility_SetCommandTarget(float linear_x, float angular_z) {
    target_linear_x = linear_x;
    target_angular_z = angular_z;
}

/**
 * @brief Main logic loop for Mobility FSM. Called periodically by its RTOS Task.
 */
void Mobility_ProcessLogic(void) {
    SystemState_t master_state = Supervisor_GetCurrentState();

    /* 1. TOP-DOWN Override: React to Master FSM */
    if (master_state == STATE_FAULT || master_state == STATE_INIT) {
        if (mob_state != MOB_DISABLED) {
            printf("MOBILITY: Master Fault/Init -> Forcing DISABLED\r\n");
            mob_state = MOB_DISABLED;
            /* Motor PWM = 0, Disable Signals = True */
        }
        return; /* Block further execution */
    }

    if (master_state == STATE_PAUSED || master_state == STATE_IDLE) {
        if (mob_state == MOB_MOVING) {
            printf("MOBILITY: Master Paused/Idle -> Forcing STOPPED\r\n");
            mob_state = MOB_STOPPED;
            /* Motor PWM = 0 */
            target_linear_x = 0.0f;
            target_angular_z = 0.0f;
        }
        /* If we are DISABLED, we stay DISABLED until explicitly activated */
        return;
    }

    /* 2. Standard State Machine Logic (assuming Master is MANUAL or AUTO) */
    switch (mob_state) {
        case MOB_DISABLED:
            /* Enable motors if Master is active */
            mob_state = MOB_STOPPED;
            printf("MOBILITY: Transitioning to STOPPED (Motors Enabled)\r\n");
            break;

        case MOB_STOPPED:
            if (target_linear_x != 0.0f || target_angular_z != 0.0f) {
                mob_state = MOB_MOVING;
                printf("MOBILITY: Transitioning to MOVING\r\n");
            }
            break;

        case MOB_MOVING:
            if (target_linear_x == 0.0f && target_angular_z == 0.0f) {
                mob_state = MOB_STOPPED;
                printf("MOBILITY: Transitioning to STOPPED (Zero Velocity)\r\n");
            } else {
                /* Execute Kinematic Math & PID Control Here */
            }
            break;

        case MOB_FAULT:
            /* Hardware failure handled here. 
               Requires external reset or Master FSM reset logic. */
            break;
    }
}
