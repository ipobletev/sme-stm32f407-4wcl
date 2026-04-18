#include "app_rtos.h"
#include "debug_module.h"
#include "robot_state.h"

#define LOG_TAG "MOBILITY_TASK"

void StartMobilityTask(void *argument)
{
    LOG_INFO(LOG_TAG, "Mobility Logic Task Started.\r\n");

    /* Initialize Mobility Subsystem */
    FSM_Mobility_Init();

    for(;;)
    {
        /* 1. Process Mobility Subsystem Logic at 50Hz (20ms loop) */
        FSM_Mobility_ProcessLogic();
        
        /* 2. Update Shared Robot State telemetry */
        RobotState_SetMobilityState(FSM_Mobility_GetCurrentState());

        /* 3. Heartbeat: Tell the Supervisor we are alived */
        RobotState_UpdateMobilityHeartbeat();

    }
}
