#include "States/arm_state_handlers.h"
#define __ARM_FSM_INTERNAL_H_IMPORT
#include "arm_fsm_internal.h"
#include "supervisor_fsm.h"
#include "debug_module.h"

void ArmState_Init_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering INIT State (Servos Off/Calibrating)\r\n");
    /* Code to cut servo power or initialize PWM pins would go here */
}

void ArmState_Init_Run(void) {
    /* Transition logic: Go to IDLE when system is stabilized */
    LOG_INFO(LOG_TAG, "System Ready -> Transitioning to IDLE\r\n");
    FSM_Arm_ProcessEvent(EVENT_ARM_IDLE);
}

void ArmState_Init_OnExit(void) {
    LOG_INFO(LOG_TAG, "Initialization/Safety Phase Finished\r\n");
}
