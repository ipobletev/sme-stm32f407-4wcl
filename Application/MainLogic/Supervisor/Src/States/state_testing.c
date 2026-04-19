#include "States/state_handlers.h"
#include "mobility_fsm.h"
#include "robot_state.h"
#include "debug_module.h"

void State_Testing_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering TESTING State (Diagnostic Mode)\r\n");
    
    /* Command Mobility subsystem to enter TESTING mode */
    FSM_Mobility_ProcessEvent(EVENT_MOB_TESTING);
    FSM_Arm_ProcessEvent(EVENT_ARM_TESTING);
}

void State_Testing_Run(void) {
    /* Standard safety checks (watchdogs, global errors) */
}

void State_Testing_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting TESTING State\r\n");
    FSM_Mobility_ProcessEvent(EVENT_MOB_IDLE);
    FSM_Arm_ProcessEvent(EVENT_ARM_IDLE);
}
