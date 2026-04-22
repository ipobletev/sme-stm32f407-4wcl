#include "States/mob_state_handlers.h"
#include "mobility_fsm_internal.h"
#include "debug_module.h"
#include "robot_state.h"

void MobState_Testing_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering TESTING State (Raw Pulse Control).\r\n");
}

void MobState_Testing_Run(void) {
    float val;
    uint8_t use_vel;

    for (int i = 0; i < 4; i++) {
        RobotState_GetMotorTestCommand(i, &val, &use_vel);

        if (use_vel) {
            /* Velocity Mode: Use PID control */
            encoder_motor_set_speed(motors[i], val);
            encoder_motor_control(motors[i], 0.020f);
        } else {
            /* PWM Mode: Use unified output for telemetry sync */
            encoder_motor_apply_pulse(motors[i], val);
            
            /* Sync motor object state to prevent jumps when returning to PID */
            motors[i]->target_speed = 0;
            motors[i]->pid_controller.set_point = 0;
            motors[i]->current_pulse = val;
        }
    }
}

void MobState_Testing_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting TESTING State (Stopping motors).\r\n");
    
    /* Reset all test commands for safety */
    RobotState_ResetTestCommands();

    /* Stop all motors */
    for (int i = 0; i < 4; i++) {
        encoder_motor_brake(motors[i]);
    }
}
