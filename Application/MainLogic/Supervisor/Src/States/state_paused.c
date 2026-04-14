#include "States/state_handlers.h"
#include "bsp_led.h"
#include <stdio.h>

void State_Paused_OnEnter(void) {
    printf("SM: Entering PAUSED State\r\n");
    /* Output logic: Stop motors, maintain servos (implementation pending) */
    /* Visual feedback: Fast blinking or specific sequence */
    BSP_LED_SetState(BSP_LED_USER, false);
}

void State_Paused_OnExit(void) {
    printf("SM: Exiting PAUSED State\r\n");
    /* Cleanup before returning to active modes */
}
