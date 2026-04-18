#include "States/mob_state_handlers.h"
#include "mobility_fsm_internal.h"
#include "debug_module.h"

void MobState_Break_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering BREAK State (Active Braking).\r\n");
    for(int i=0; i<4; i++) {
        encoder_motor_brake(motors[i]);
    }
}

void MobState_Break_Run(void) {
    /* If targets appear, transition to MOVING */
    if (target_linear_x != 0.0f || target_angular_z != 0.0f) {
        FSM_Mobility_ProcessEvent(EVENT_MOB_MOVING);
    }
}

void MobState_Break_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting BREAK State.\r\n");
}
