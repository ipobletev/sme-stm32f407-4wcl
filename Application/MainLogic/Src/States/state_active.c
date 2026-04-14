#include "States/state_handlers.h"
#include "bsp_led.h"
#include <stdio.h>

void State_Active_OnEnter(void) {
    printf("SM: Entering STATE_ACTIVE - Operation Started\r\n");
    BSP_LED_SetState(BSP_LED_USER, true);
}

void State_Active_OnExit(void) {
    printf("SM: Exiting STATE_ACTIVE - Operation Stopped\r\n");
    BSP_LED_SetState(BSP_LED_USER, false);
}
