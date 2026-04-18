#include "States/arm_state_handlers.h"
#include "arm_fsm_internal.h"
#include "debug_module.h"

void ArmState_Fault_OnEnter(void) {
    LOG_ERROR(LOG_TAG, "!!! FAULT DETECTED !!!\r\n");
}

void ArmState_Fault_Run(void) {
    /* Handle error recovery or stay in safe mode */
}

void ArmState_Fault_OnExit(void) {
    LOG_INFO(LOG_TAG, "Clearing Fault State\r\n");
}
