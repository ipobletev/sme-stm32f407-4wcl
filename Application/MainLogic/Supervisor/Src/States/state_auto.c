#include "States/state_handlers.h"
#include "bsp_led.h"
#include "robot_state.h"
#include <stdio.h>

void State_Auto_OnEnter(void) {
    printf("Supervisor: Entering AUTO State (ROS Command)\r\n");
    RobotState_SetAutonomous(1);
    /* Auto Mode: Signal active heartbeat or specific feedback */
    BSP_LED_SetState(BSP_LED_USER, true);
}


void State_Auto_OnExit(void) {
    printf("SM: Exiting AUTO State\r\n");
    BSP_LED_SetState(BSP_LED_USER, false);
}
