#include "States/state_handlers.h"
#include "debug_module.h"
#include <stdio.h>

#define LOG_TAG "SUPERVISOR"

void State_Fault_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering STATE_FAULT - Error Detected!\r\n");
}

void State_Fault_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting STATE_FAULT - Error Cleared\r\n");
}
