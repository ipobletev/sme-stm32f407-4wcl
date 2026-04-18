#include "States/mob_state_handlers.h"
#include "mobility_fsm_internal.h"
#include "debug_module.h"

void MobState_Abort_OnEnter(void) {
    LOG_WARNING(LOG_TAG, "Entering ABORT State (External Stop). Stopping Motors.\r\n");
    for(int i=0; i<4; i++) {
        encoder_motor_brake(motors[i]);
    }
}

void MobState_Abort_Run(void) {
    /* Abort state just holds motors stopped until reset */
}

void MobState_Abort_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting ABORT State.\r\n");
}
