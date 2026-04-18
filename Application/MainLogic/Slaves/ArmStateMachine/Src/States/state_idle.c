#include "States/arm_state_handlers.h"
#include "arm_fsm_internal.h"
#include "debug_module.h"

void ArmState_Idle_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering IDLE State (Holding Position)\r\n");
}

void ArmState_Idle_Run(void) {
    // TODO: Implement idle logic
}

void ArmState_Idle_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting IDLE State\r\n");
}
