#include "States/state_handlers.h"
#include "robot_state.h"
#include "debug_module.h"

void State_Manual_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering MANUAL State (Operator Control)\r\n");
    RobotState_SetAutonomous(0);
}

void State_Manual_Run(void) {
    /* Logic */
}


void State_Manual_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting MANUAL State\r\n");
    /* Cleanup before returning to active modes */
}
