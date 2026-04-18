#include "States/state_handlers.h"
#include "debug_module.h"
#include "mobility_fsm.h"
#include "arm_fsm.h"

void State_Fault_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering STATE_FAULT (EMERGENCY STOP)\r\n");
}

void State_Fault_Run(void) {
    /* Logic */
}

void State_Fault_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting STATE_FAULT - Error Cleared\r\n");
    /* Cleanup before returning to active modes */
}
