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

    MobilityMode_t mode = RobotState_GetTargetMobilityMode();
    float velocity_wheels[4] = {0, 0, 0, 0};
    
    /* 3. Apply Kinematic Model */
    switch (mode) {
        case MOB_MODE_MECANUM:
            Kinematics_Mecanum(target_linear_x, 0.0f, target_angular_z, velocity_wheels);
            break;

        case MOB_MODE_DIFF:
            Kinematics_Differential(target_linear_x, target_angular_z, velocity_wheels);
            break;

        case MOB_MODE_ACKERMANN:
            Kinematics_Ackermann(target_linear_x, target_angular_z, velocity_wheels);
            break;

        case MOB_MODE_DIRECT:
            Kinematics_Direct(target_linear_x, target_angular_z, velocity_wheels);
            break;

        default:
            /* Safety default: Stop is already 0 */
            break;
    }

    /* 4. Convert linear velocity (m/s) to RPS: rps = v / (PI * D) */
    float rps_conv = 1.0f / (M_PI * ROBOT_WHEEL_DIAMETER);
    
    /* 5. Set targets for individual PID controllers */
    /* Note: Right side motors (3 and 4) are physically flipped, hence the minus sign */
    encoder_motor_set_speed(motors[0], -velocity_wheels[0] * rps_conv);
    encoder_motor_set_speed(motors[1], velocity_wheels[1] * rps_conv);
    encoder_motor_set_speed(motors[2], -velocity_wheels[2] * rps_conv); 
    encoder_motor_set_speed(motors[3], velocity_wheels[3] * rps_conv); 

    /* Run Motors. Run PID loop (if ROBOT_STATE_DEFAULT_PID_ENABLED enabled) for each motor */
    for (int i = 0; i < 4; i++) {
        encoder_motor_control(i, motors[i], 0.02f);
    }
}

void MobState_Moving_OnExit(void) {
    /* Reset commands when exiting MOVING state */
    RobotState_ResetMobilityCommands();
    LOG_INFO(LOG_TAG, "Exiting MOVING State. Commands reset.\r\n");
}
