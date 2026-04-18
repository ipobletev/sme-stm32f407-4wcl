#include "States/state_handlers.h"
#include "debug_module.h"

void State_Paused_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering PAUSED State\r\n");
}

void State_Paused_Run(void) {
     /* Logic */
}

void State_Paused_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting PAUSED State\r\n");
    /* Cleanup before returning to active modes */
}
