#include "States/state_handlers.h"
#include <stdio.h>

void State_Fault_OnEnter(void) {
    printf("SM: Entering STATE_FAULT - Error Detected!\r\n");
}

void State_Fault_OnExit(void) {
    printf("SM: Exiting STATE_FAULT - Error Cleared\r\n");
}
