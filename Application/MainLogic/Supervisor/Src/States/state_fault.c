#include "States/state_handlers.h"
#include "debug_module.h"
#include "io_buzzer.h"
#include "bsp_buzzer.h"
#include "mobility_fsm.h"
#include "arm_fsm.h"
#include "robot_state.h"
#include "osal.h"
#include "main.h"

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
    
    /* Start intermittent buzzer: 2kHz, 1000ms period */
    io_buzzer(2000, 1000);
}

void State_Fault_Run(void) {
    /* Buzzer is handled by io_buzzer module timer */
}

void State_Fault_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting STATE_FAULT - Error Cleared. Resetting slaves to init.\r\n");
    
    /* Clear all global error flags to allow system recovery */
    RobotState_ClearAllErrorFlags();

    FSM_Mobility_ProcessEvent(EVENT_MOB_INIT);
    FSM_Arm_ProcessEvent(EVENT_ARM_INIT);

    /* Ensure buzzer is OFF */
    io_buzzer_stop();
}
