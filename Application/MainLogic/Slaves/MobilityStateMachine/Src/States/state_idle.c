#include "States/mob_state_handlers.h"
#include "mobility_fsm_internal.h"
#include "debug_module.h"

void MobState_Idle_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering IDLE State. Waiting for commands.\r\n");
    target_linear_x = 0.0f;
    target_angular_z = 0.0f;
    for(int i=0; i<4; i++) {
        encoder_motor_brake(motors[i]);
    }
}

void MobState_Idle_Run(void) {
    /* Transition to MOVING if targets are non-zero */
    if (target_linear_x != 0.0f || target_angular_z != 0.0f) {
        Mobility_ProcessEvent(EVENT_MOVING);
    }
}

void MobState_Idle_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting IDLE State.\r\n");
}
