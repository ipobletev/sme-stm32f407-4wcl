#include "States/state_handlers.h"
#include "bsp_led.h"
#include "robot_state.h"
#include "debug_module.h"
#include <stdio.h>

#define LOG_TAG "SUPERVISOR"

void State_Auto_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering AUTO State (ROS Command)\r\n");
    RobotState_SetAutonomous(1);
    /* Auto Mode: Signal active heartbeat or specific feedback */
    BSP_LED_SetState(BSP_LED_USER, true);
}


void State_Auto_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting AUTO State\r\n");
    BSP_LED_SetState(BSP_LED_USER, false);
}
