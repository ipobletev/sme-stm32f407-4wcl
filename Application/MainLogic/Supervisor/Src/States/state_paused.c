#include "States/state_handlers.h"
#include "bsp_led.h"
#include "debug_module.h"
#include <stdio.h>

#define LOG_TAG "SUPERVISOR"

void State_Paused_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering PAUSED State\r\n");
    /* Output logic: Stop motors, maintain servos (implementation pending) */
    /* Visual feedback: Fast blinking or specific sequence */
    BSP_LED_SetState(BSP_LED_USER, false);
}

void State_Paused_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting PAUSED State\r\n");
    /* Cleanup before returning to active modes */
}
