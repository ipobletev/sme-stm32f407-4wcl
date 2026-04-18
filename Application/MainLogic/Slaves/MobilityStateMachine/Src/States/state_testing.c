#include "States/mob_state_handlers.h"
#include "mobility_fsm_internal.h"
#include "debug_module.h"

void MobState_Testing_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering TESTING State (Raw Pulse Control).\r\n");
}

void MobState_Testing_Run(void) {
    /* If a velocity command arrives, exit testing mode and go to MOVING */
    // if (target_linear_x != 0.0f || target_angular_z != 0.0f) {
    //     FSM_Mobility_ProcessEvent(EVENT_MOB_MOVING);
    // }
}

void MobState_Testing_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting TESTING State.\r\n");
}
