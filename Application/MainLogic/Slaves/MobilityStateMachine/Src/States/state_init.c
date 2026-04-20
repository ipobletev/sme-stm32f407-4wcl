#include "States/mob_state_handlers.h"
#include "mobility_fsm_internal.h"
#include "robot_state.h"
#include "supervisor_fsm.h"
#include "debug_module.h"
#include "error_codes.h"

static bool init_success = true;

void MobState_Init_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering INIT State. Initializing...\r\n");
}

void MobState_Init_Run(void) {

    init_success = true;

    /* 1. Initialize Hardware Peripherals first so pointers are set */
    if (!encoder_motor_init_hw_system(motors)) {
        init_success = false;
    }

    /* 2. Configure Motor Objects with specific profile */
    for(int i=0; i<4; i++) {
        if (!encoder_motor_configure(motors[i], (uint8_t)(i + 1), MOTOR_TYPE_JGB520)) {
            init_success = false;
        }
    }

    if (!init_success) {
        LOG_ERROR(LOG_TAG, "Hardware Initialization FAILED!\r\n");
    }

    /* 1. If initialization failed, trigger local fault */
    if (!init_success) {
        RobotState_SetErrorFlag(ERR_MOB_DRIVE);
        LOG_ERROR(LOG_TAG, "Hardware Motor Initialization FAILED! -> Transitioning to FAULT\r\n");
        FSM_Mobility_ProcessEvent(EVENT_MOB_FAULT);
        return;
    }
    /* 2. Transition to IDLE if initialization succeed */
    LOG_INFO(LOG_TAG, "Hardware Initialization Complete.\r\n");
    LOG_INFO(LOG_TAG, "System Ready -> Transitioning to IDLE\r\n");
    FSM_Mobility_ProcessEvent(EVENT_MOB_IDLE);
}

void MobState_Init_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting INIT State.\r\n");
}
