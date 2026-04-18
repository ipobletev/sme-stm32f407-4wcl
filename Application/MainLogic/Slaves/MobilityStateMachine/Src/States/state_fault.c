#include "States/mob_state_handlers.h"
#include "mobility_fsm_internal.h"
#include "debug_module.h"

void MobState_Fault_OnEnter(void) {
    LOG_ERROR(LOG_TAG, "Entering FAULT State! Disabling Motors.\r\n");
    for(int i=0; i<4; i++) {
        encoder_motor_brake(motors[i]);
    }
}

void MobState_Fault_Run(void) {
    /* Fault requires manual reset or Master FSM reset logic */
}

void MobState_Fault_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting FAULT State. System reset.\r\n");
}
