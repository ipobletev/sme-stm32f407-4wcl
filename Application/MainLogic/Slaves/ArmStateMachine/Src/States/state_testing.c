#include "States/arm_state_handlers.h"
#define __ARM_FSM_INTERNAL_H_IMPORT
#include "arm_fsm_internal.h"
#include "debug_module.h"

void ArmState_Testing_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering TESTING State (Raw Servo Control).\r\n");
}

void ArmState_Testing_Run(void) {
    /* 
       If a high-level goal arrives (non-zero target), 
       exit testing mode and go back to MOVING.
    */
    if (target_j1 != 0.0f || target_j2 != 0.0f || target_j3 != 0.0f) {
        LOG_INFO(LOG_TAG, "Control Takeover -> Exiting TESTING\r\n");
        Arm_ProcessEvent(EVENT_ARM_MOVING);
    }
}

void ArmState_Testing_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting TESTING State.\r\n");
}
