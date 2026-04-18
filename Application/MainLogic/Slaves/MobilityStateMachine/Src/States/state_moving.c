#include "States/mob_state_handlers.h"
#include "mobility_fsm_internal.h"
#include "debug_module.h"
#include "motor_hardware.h"
#include <math.h>
#include <stdio.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

void MobState_Moving_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering MOVING State.\r\n");
}

void MobState_Moving_Run(void) {
    /* If targets return to zero, transition back to IDLE */
    if (target_linear_x == 0.0f && target_angular_z == 0.0f) {
        FSM_Mobility_ProcessEvent(EVENT_IDLE);
        return;
    }

    /* --- MECANUM KINEMATICS (Logica Bruta) --- */
    float vx = target_linear_x;
    float vy = 0; /* No strafing for now as only linear_x and angular_z are in RobotState */
    float az = target_angular_z;
    float l_plus_w = JETAUTO_WHEELBASE + JETAUTO_SHAFT_LENGTH;
    
    /* Calculate motor target velocities */
    float v1 = vx - vy - l_plus_w * az;
    float v2 = vx + vy - l_plus_w * az;
    float v3 = vx + vy + l_plus_w * az;
    float v4 = vx - vy + l_plus_w * az;

    /* Convert linear velocity (m/s) to RPS: rps = v / (PI * D) */
    float rps_conv = 1.0f / (M_PI * JETAUTO_WHEEL_DIAMETER);
    
    /* Set targets for PID controllers */
    encoder_motor_set_speed(motors[0], v1 * rps_conv);
    encoder_motor_set_speed(motors[1], v2 * rps_conv);
    encoder_motor_set_speed(motors[2], -v3 * rps_conv); 
    encoder_motor_set_speed(motors[3], -v4 * rps_conv); 
}

void MobState_Moving_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting MOVING State.\r\n");
}
