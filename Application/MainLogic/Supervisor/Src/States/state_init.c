#include "States/state_handlers.h"
#include "debug_module.h"
#include <stdio.h>

#define LOG_TAG "SUPERVISOR"

void State_Init_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering STATE_INIT\r\n");
}

void State_Init_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting STATE_INIT\r\n");
}
