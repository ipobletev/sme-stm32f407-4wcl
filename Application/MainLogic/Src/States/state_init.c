#include "States/state_handlers.h"
#include <stdio.h>

void State_Init_OnEnter(void) {
    printf("SM: Entering STATE_INIT\r\n");
}

void State_Init_OnExit(void) {
    printf("SM: Exiting STATE_INIT\r\n");
}
