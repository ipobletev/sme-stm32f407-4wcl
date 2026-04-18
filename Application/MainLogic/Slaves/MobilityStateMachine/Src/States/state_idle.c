#include "States/mob_state_handlers.h"
#include "mobility_fsm_internal.h"
#include "robot_state.h"
#include "debug_module.h"

void MobState_Idle_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering IDLE State. Resetting motor targets and braking all motors. Waiting for commands.\r\n");

    /* Reset the target velocity values*/
    target_linear_x = 0.0f;
    target_angular_z = 0.0f;
    
    /* Brake all motors */
    for(int i=0; i<4; i++) {
        encoder_motor_brake(motors[i]);
    }
}

void MobState_Idle_Run(void) {
    /* Check the Mobility command values from shared RobotState */
    RobotState_GetTargetVelocity(&target_linear_x, &target_angular_z);

    /* Transition to MOVING if targets are non-zero */
    if (target_linear_x != 0.0f || target_angular_z != 0.0f) {
        LOG_INFO(LOG_TAG, "Mobility command values are non-zero -> Transitioning to MOVING\r\n");
        FSM_Mobility_ProcessEvent(EVENT_MOB_MOVING);
    }
}

void MobState_Idle_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting IDLE State.\r\n");
}
