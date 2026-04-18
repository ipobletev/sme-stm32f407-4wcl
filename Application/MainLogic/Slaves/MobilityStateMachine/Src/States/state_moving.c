#include "States/mob_state_handlers.h"
#include "mobility_fsm_internal.h"
#include "robot_state.h"
#include "debug_module.h"
#include "kinematics.h"
#include "config.h"
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

void MobState_Moving_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering MOVING State.\r\n");
}

void MobState_Moving_Run(void) {
    /* 1. Sync targets from shared state */
    RobotState_GetTargetVelocity(&target_linear_x, &target_angular_z);

    /* 2. State Transition Check: If targets return to zero, transition back to IDLE */
    if (target_linear_x == 0.0f && target_angular_z == 0.0f) {
        FSM_Mobility_ProcessEvent(EVENT_MOB_IDLE);
        return;
    }

    uint8_t mode = RobotState_GetTargetMobilityMode();
    float vw[4] = {0, 0, 0, 0};
    
    /* 3. Apply Kinematic Model */
    switch (mode) {
        case MOB_MODE_MECANUM:
            Kinematics_Mecanum(target_linear_x, 0.0f, target_angular_z, vw);
            break;

        case MOB_MODE_DIFF:
            Kinematics_Differential(target_linear_x, target_angular_z, vw);
            break;

        case MOB_MODE_ACKERMANN:
            Kinematics_Ackermann(target_linear_x, target_angular_z, vw);
            break;

        case MOB_MODE_DIRECT:
            Kinematics_Direct(target_linear_x, target_angular_z, vw);
            break;

        default:
            /* Safety default: Stop is already 0 */
            break;
    }

    /* 4. Convert linear velocity (m/s) to RPS: rps = v / (PI * D) */
    float rps_conv = 1.0f / (M_PI * ROBOT_WHEEL_DIAMETER);
    
    /* 5. Set targets for individual PID controllers */
    /* Note: Right side motors (3 and 4) are physically flipped, hence the minus sign */
    encoder_motor_set_speed(motors[0], vw[0] * rps_conv);
    encoder_motor_set_speed(motors[1], vw[1] * rps_conv);
    encoder_motor_set_speed(motors[2], -vw[2] * rps_conv); 
    encoder_motor_set_speed(motors[3], -vw[3] * rps_conv); 
}

void MobState_Moving_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting MOVING State.\r\n");
}
