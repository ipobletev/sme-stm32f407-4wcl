#include "app_rtos.h"
#include "arm_fsm.h"
#include "debug_module.h"
#include "robot_state.h"

#define LOG_TAG "ARM_TASK"

void StartArmTask(void *argument)
{
    LOG_INFO(LOG_TAG, "Robotic Arm Logic Task Started.");

    Arm_Init();

    for(;;)
    {
        /* Increment Watchdog for Supervisor */
        RobotState_FeedWatchdogArm();

        /* Process Arm Logic at 50Hz (20ms) */
        Arm_ProcessLogic();

        /* Update Shared Robot State telemetry */
        RobotState_SetArmState(Arm_GetCurrentState());

        /* In a real scenario, check queues for joint_trajectory commands */

        osal_delay(20);
    }
}
