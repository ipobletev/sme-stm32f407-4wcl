#include "arm_fsm.h"
#include "supervisor_fsm.h" /* For Supervisor FSM interaction */
#include "robot_state.h"
#include <stdio.h>

const char* Arm_StateToStr(ArmState_t state) {
    switch(state) {
        case ARM_DISABLED: return "DISABLED";
        case ARM_HOMING:   return "HOMING";
        case ARM_IDLE:     return "IDLE";
        case ARM_MOVING:   return "MOVING";
        case ARM_FAULT:    return "FAULT";
        default:           return "UNKNOWN";
    }
}

static ArmState_t arm_state = ARM_DISABLED;

/* Joint targets */
static float target_j1 = 0.0f;
static float target_j2 = 0.0f;
static float target_j3 = 0.0f;

/* Homing simulation variables */
static uint8_t homing_progress = 0;

void Arm_Init(void) {
    arm_state = ARM_DISABLED;
    homing_progress = 0;
    printf("ARM: Initialized (Disabled)\r\n");
}

ArmState_t Arm_GetCurrentState(void) {
    return arm_state;
}

void Arm_SetJointTarget(float j1, float j2, float j3) {
    target_j1 = j1;
    target_j2 = j2;
    target_j3 = j3;
}

/**
 * @brief Main logic loop for Arm FSM. Called periodically by its RTOS Task.
 */
void Arm_ProcessLogic(void) {
    SystemState_t master_state = Supervisor_GetCurrentState();

    /* 1. TOP-DOWN Override: React to Master FSM */
    if (master_state == STATE_FAULT || master_state == STATE_INIT) {
        if (arm_state != ARM_DISABLED) {
            printf("ARM: Master Fault/Init -> Forcing DISABLED\r\n");
            arm_state = ARM_DISABLED;
            /* Cut servo power */
        }
        return; /* Block further execution */
    }

    if (master_state == STATE_PAUSED || master_state == STATE_IDLE) {
        if (arm_state == ARM_MOVING || arm_state == ARM_HOMING) {
            printf("ARM: Master Paused/Idle -> Forcing IDLE (Holding Position)\r\n");
            arm_state = ARM_IDLE;
            /* Keep servo power, but stop trajectory */
        }
        return;
    }

    /* 2. Standard State Machine Logic (assuming Master is MANUAL or AUTO) */
    switch (arm_state) {
        case ARM_DISABLED:
            /* If master is active, we must home first */
            arm_state = ARM_HOMING;
            homing_progress = 0;
            printf("ARM: Transitioning to HOMING (Seeking Zero)\r\n");
            break;

        case ARM_HOMING:
            /* Simulate Homing Routine */
            homing_progress++;
            if (homing_progress > 10) { // Simulate time passed
                arm_state = ARM_IDLE;
                printf("ARM: Homing Complete. Transitioning to IDLE\r\n");
            }
            break;

        case ARM_IDLE:
            /* Note: In a real arm, we check if target differs from current pos */
            if (target_j1 != 0.0f || target_j2 != 0.0f || target_j3 != 0.0f) {
                arm_state = ARM_MOVING;
                printf("ARM: Transitioning to MOVING\r\n");
            }
            break;

        case ARM_MOVING:
            if (target_j1 == 0.0f && target_j2 == 0.0f && target_j3 == 0.0f) {
                arm_state = ARM_IDLE;
                printf("ARM: Transitioning to IDLE (Target Reached)\r\n");
            } else {
                /* Execute IK & Servo Control Here */
            }
            break;

        case ARM_FAULT:
            /* Hardware failure (stall, overheat) handled here. */
            break;
    }
}
