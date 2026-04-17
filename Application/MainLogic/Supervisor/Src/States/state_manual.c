#include "States/state_handlers.h"
#include "bsp_led.h"
#include "robot_state.h"
#include "debug_module.h"
#include <stdio.h>

#define LOG_TAG "SUPERVISOR"

void State_Manual_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering MANUAL State (Operator Control)\r\n");
    RobotState_SetAutonomous(0);
    /* Manual Mode: Solid LED or steady feedback */
    BSP_LED_SetState(BSP_LED_USER, true);
}


void State_Manual_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting MANUAL State\r\n");
    BSP_LED_SetState(BSP_LED_USER, false);
}
