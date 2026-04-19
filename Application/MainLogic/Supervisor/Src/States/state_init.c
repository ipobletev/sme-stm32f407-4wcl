#include "States/state_handlers.h"
#include "debug_module.h"
#include "robot_state.h"
#include "supervisor_fsm.h"
#include "osal.h"

static uint32_t entry_tick = 0;

void State_Init_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering STATE_INIT\r\n");
    entry_tick = osal_get_tick();

    /* Reset all commands and error flags upon entering INIT (Global Reset) */
    RobotState_ResetMobilityCommands();
    RobotState_ClearAllErrorFlags();
    LOG_INFO(LOG_TAG, "Global Reset: Commands and Error Flags cleared.\r\n");
}

void State_Init_Run(void) {
    /* Wait 1 second to ensure subsystems are up and running */
    if (osal_get_tick() - entry_tick >= ENTRY_WAIT_TIME_MS) {
        LOG_INFO(LOG_TAG, "Initialization complete. Signaling READY.\r\n");
        Supervisor_ProcessEvent(EVENT_SUPERVISOR_READY, SRC_INTERNAL_SUPERVISOR);
    }
}

void State_Init_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting STATE_INIT\r\n");
    /* Cleanup before returning to active modes */
}
