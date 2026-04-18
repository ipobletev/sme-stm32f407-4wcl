#include "app_rtos.h"
#include "arm_fsm.h"
#include "debug_module.h"
#include "robot_state.h"

#define LOG_TAG "ARM_TASK"

void StartArmTask(void *argument)
{
    LOG_INFO(LOG_TAG, "Robotic Arm Logic Task Started.\r\n");

    FSM_Arm_Init();

    for(;;)
    {
        /* 1. Process Arm Logic at 50Hz (20ms) */
        FSM_Arm_ProcessLogic();

        /* 3. Update Shared Robot State telemetry */
        RobotState_SetArmState(FSM_Arm_GetCurrentState());

        /* 4. Heartbeat: Tell the Supervisor we finished our logic successfully */
        RobotState_UpdateArmHeartbeat();

        osal_delay(20);
    }
}
