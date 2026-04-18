#include "States/mob_state_handlers.h"
#include "mobility_fsm_internal.h"
#include "supervisor_fsm.h"
#include "debug_module.h"

void MobState_Init_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering INIT State. Initializing...\r\n");
    
    /* Initialize Motor Objects with default types */
    for(int i=0; i<4; i++) {
        encoder_motor_object_init(motors[i]);
        Motor_Hardware_SetType(motors[i], MOTOR_TYPE_JGB520);
        encoder_motor_brake(motors[i]); /* Ensure safety during init */
    }

    /* Initialize Hardware Peripherals */
    Motor_Hardware_Init(motors);
}

void MobState_Init_Run(void) {
    /* Transition to IDLE if Master Supervisor is active (NOT in FAULT or INIT) */
    SystemState_t master_state = Supervisor_GetCurrentState();
    if (master_state != STATE_SUPERVISOR_FAULT && master_state != STATE_SUPERVISOR_INIT) {
        LOG_INFO(LOG_TAG, "System Ready -> Transitioning to IDLE\r\n");
        FSM_Mobility_ProcessEvent(EVENT_IDLE);
    }
}

void MobState_Init_OnExit(void) {
    LOG_INFO(LOG_TAG, "Hardware Initialization Complete.\r\n");
}
