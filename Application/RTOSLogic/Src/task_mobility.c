#include "app_rtos.h"
#include "mobility_fsm.h"
#include "debug_module.h"
#include "robot_state.h"

#define LOG_TAG "MOBILITY_TASK"

void StartMobilityTask(void *argument)
{
    LOG_INFO(LOG_TAG, "Mobility Logic Task Started.");

    Mobility_Init();

    for(;;)
    {
        /* Increment Watchdog for Supervisor */
        RobotState_FeedWatchdogMobility();

        /* Process Mobility Logic at 50Hz (20ms loop) */
        Mobility_ProcessLogic();
        
        /* Update Shared Robot State telemetry */
        RobotState_SetMobilityState(Mobility_GetCurrentState());

        /* In a real scenario, here we check queues for cmd_vel if needed, 
           or global targets updated by UART listener. */

        osal_delay(20);
    }
}
