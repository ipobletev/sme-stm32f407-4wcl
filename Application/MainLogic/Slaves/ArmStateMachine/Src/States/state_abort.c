#include "States/arm_state_handlers.h"
#include "arm_fsm_internal.h"
#include "debug_module.h"

void ArmState_Abort_OnEnter(void) {
    LOG_WARNING(LOG_TAG, "!!! ABORT STATE !!! (External Safety Stop)\r\n");
}

void ArmState_Abort_Run(void) {
    /* Safe state: hold current position, but do not process new trajectory targets */
}

void ArmState_Abort_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting Abort State\r\n");
}
