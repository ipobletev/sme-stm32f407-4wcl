#include "States/state_handlers.h"
#include <stdio.h>

void State_Idle_OnEnter(void) {
    printf("SM: Entering STATE_IDLE - System Ready\r\n");
}

void State_Idle_OnExit(void) {
    printf("SM: Exiting STATE_IDLE\r\n");
}
