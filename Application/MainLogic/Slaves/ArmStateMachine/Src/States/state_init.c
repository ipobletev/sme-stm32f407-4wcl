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
    SystemState_t master_state = Supervisor_GetCurrentState();

    /* Transition logic: If supervisor is in MANUAL or AUTO, start homing */
    if (master_state == STATE_SUPERVISOR_MANUAL || master_state == STATE_SUPERVISOR_AUTO) {
        LOG_INFO(LOG_TAG, "System Ready -> Transitioning to HOMING\r\n");
        Arm_ProcessEvent(EVENT_ARM_HOMING);
    }
}

void ArmState_Init_OnExit(void) {
    LOG_INFO(LOG_TAG, "Initialization/Safety Phase Finished\r\n");
}
