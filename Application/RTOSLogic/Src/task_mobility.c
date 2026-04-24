#include "app_rtos.h"
#include "debug_module.h"
#include "robot_state.h"

#define LOG_TAG "MOBILITY_TASK"

#include "app_config.h"

void StartMobilityTask(void *argument)
{
    LOG_INFO(LOG_TAG, "Mobility Logic Task Started.\r\n");

    /* Initialize Mobility Subsystem */
    FSM_Mobility_Init();

    for(;;)
    {
        /* 1. Process Mobility Subsystem Logic at 50Hz (20ms loop) 
         * The FSM handles inputs (Joystick) internally within its state handlers. */
        FSM_Mobility_ProcessLogic();
        
        /* 2. Heartbeat: Tell the Supervisor we are alive */
        RobotState_UpdateMobilityHeartbeat();

        osal_delay(20);
    }
}
