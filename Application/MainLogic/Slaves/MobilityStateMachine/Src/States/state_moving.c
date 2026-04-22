#include "States/mob_state_handlers.h"
#include "mobility_fsm_internal.h"
#include "robot_state.h"
#include "debug_module.h"
#include "kinematics.h"
#include "app_config.h"
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
        FSM_Mobility_ProcessEvent(EVENT_MOB_BREAK);
        return;
    }

    MobilityMode_t mode = RobotState_GetTargetMobilityMode();
    float velocity_wheels[4] = {0, 0, 0, 0};
    
    /* 3. Apply Kinematic Model (Inject dimensions from config) */
    float wheelbase = AppConfig->wheelbase_length;
    float track_width = AppConfig->shaft_width;

    switch (mode) {
        case MOB_MODE_MECANUM:
            Kinematics_Mecanum(target_linear_x, 0.0f, target_angular_z, wheelbase, track_width, velocity_wheels, 4);
            break;

        case MOB_MODE_DIFF:
            Kinematics_Differential(target_linear_x, target_angular_z, track_width, velocity_wheels, 4);
            break;

        case MOB_MODE_ACKERMANN:
            Kinematics_Ackermann(target_linear_x, target_angular_z, track_width, velocity_wheels, 4);
            break;

        case MOB_MODE_DIRECT:
            Kinematics_Direct(target_linear_x, target_angular_z, track_width, velocity_wheels, 4);
            break;

        default:
            /* Safety default: Stop is already 0 */
            break;
    }

    /* 4. Set targets for individual PID controllers (already in m/s) */
    /* Note: Right side motors (1 and 3) represent the physical mapping of the hardware.
       The sign mapping here ensures positive linear_x moves the robot forward. */
    encoder_motor_set_speed(motors[0], velocity_wheels[0]);
    encoder_motor_set_speed(motors[1], velocity_wheels[1]);
    encoder_motor_set_speed(motors[2], velocity_wheels[2]); 
    encoder_motor_set_speed(motors[3], velocity_wheels[3]); 

    /* Run Motors. Run PID loop (if ROBOT_STATE_DEFAULT_PID_ENABLED enabled) for each motor */
    for (int i = 0; i < 4; i++) {
        encoder_motor_control(motors[i], 0.02f);
    }
}

void MobState_Moving_OnExit(void) {
    /* Reset commands when exiting MOVING state */
    RobotState_ResetMobilityCommands();
    LOG_INFO(LOG_TAG, "Exiting MOVING State. Commands reset.\r\n");
}
