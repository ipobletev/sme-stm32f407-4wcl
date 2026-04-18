#include "States/state_handlers.h"
#include "debug_module.h"
#include "mobility_fsm.h"
#include "arm_fsm.h"
#include "robot_state.h"

void State_Fault_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering STATE_FAULT (EMERGENCY STOP)\r\n");

    MobilityState_t mobility_state = RobotState_GetMobilityState();
    if (mobility_state != STATE_MOB_FAULT && mobility_state != STATE_MOB_ABORT) {
        FSM_Mobility_ProcessEvent(EVENT_MOB_ABORT);
    }

    ArmState_t arm_state = RobotState_GetArmState();
    if (arm_state != STATE_ARM_FAULT && arm_state != STATE_ARM_ABORT) {
        FSM_Arm_ProcessEvent(EVENT_ARM_ABORT);
    }
}

void State_Fault_Run(void) {
    /* Logic */
}

void State_Fault_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting STATE_FAULT - Error Cleared. Resetting slaves to init.\r\n");
    
    /* Clear all global error flags to allow system recovery */
    RobotState_ClearAllErrorFlags();

    FSM_Mobility_ProcessEvent(EVENT_MOB_INIT);
    FSM_Arm_ProcessEvent(EVENT_ARM_INIT);
}
