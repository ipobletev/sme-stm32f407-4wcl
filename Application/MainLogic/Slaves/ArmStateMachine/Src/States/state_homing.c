#include "States/arm_state_handlers.h"
#include "arm_fsm_internal.h"
#include "debug_module.h"

void ArmState_Homing_OnEnter(void) {
    homing_progress = 0;
    LOG_INFO(LOG_TAG, "Entering HOMING State (Seeking Zero)\r\n");
}

void ArmState_Homing_Run(void) {
    /* Simulate Homing Routine */
    homing_progress++;
    
    if (homing_progress > 10) { // Simulate completion
        FSM_Arm_ProcessEvent(EVENT_ARM_IDLE);
    }
}

void ArmState_Homing_OnExit(void) {
    LOG_INFO(LOG_TAG, "Homing Routine Finished\r\n");
}
