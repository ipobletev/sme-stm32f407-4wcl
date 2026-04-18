#include "States/state_handlers.h"
#include "robot_state.h"
#include "debug_module.h"

void State_Auto_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering AUTO State (ROS Command)\r\n");
    RobotState_SetAutonomous(1); // Set RobotState_4wcl as is_autonomous to 1
}

void State_Auto_Run(void) {
     /* Logic */
}


void State_Auto_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting AUTO State\r\n");
    /* Cleanup before returning to active modes */
}
