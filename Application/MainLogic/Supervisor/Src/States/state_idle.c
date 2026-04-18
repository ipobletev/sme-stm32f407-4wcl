#include "States/state_handlers.h"
#include "debug_module.h"

void State_Idle_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering STATE_IDLE\r\n");
}

void State_Idle_Run(void) {
    /* Logic */
}

void State_Idle_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting STATE_IDLE\r\n");
    /* Cleanup before returning to active modes */
}
