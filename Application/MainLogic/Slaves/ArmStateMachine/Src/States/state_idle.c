#include "States/arm_state_handlers.h"
#include "arm_fsm_internal.h"
#include "debug_module.h"

void ArmState_Idle_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering IDLE State (Holding Position)\r\n");
}

void ArmState_Idle_Run(void) {
    if (target_j1 != 0.0f || target_j2 != 0.0f || target_j3 != 0.0f) {
        Arm_ProcessEvent(EVENT_ARM_MOVING);
    }
}

void ArmState_Idle_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting IDLE State\r\n");
}
