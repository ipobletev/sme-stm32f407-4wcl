#include "States/state_handlers.h"
#include "debug_module.h"

void State_Init_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering STATE_INIT\r\n");
}

void State_Init_Run(void) {
    /* Logic */
}

void State_Init_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting STATE_INIT\r\n");
    /* Cleanup before returning to active modes */
}
