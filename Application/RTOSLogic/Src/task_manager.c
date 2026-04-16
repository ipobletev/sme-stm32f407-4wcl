#include "app_rtos.h"
#include "robot_state.h"
#include "supervisor_fsm.h"
#include <stdio.h>

void StartManagerTask(void *argument)
{
    StateChangeMsg_t msg;
    uint8_t prev_mob_wdg = 0;
    uint8_t prev_arm_wdg = 0;
    uint32_t stall_timer_mob = 0;
    uint32_t stall_timer_arm = 0;

    /* Initialize the internal supervisor logic */
    Supervisor_Init();

    printf("Manager Task Started. Ready to process events.\r\n");

    for(;;)
    {
        /* Wait for a message from the Controller task, block for 20ms */
        if (osal_queue_get(stateMsgQueueHandle, &msg, 20U) == OSAL_OK)
        {
            printf("Manager: Processing Event %d collected at tick %lu\r\n", 
                   msg.event, (unsigned long)msg.timestamp);

            /* Delegate the transition logic to the Supervisor module */
            Supervisor_ProcessEvent(msg.event, msg.source);
        }

        /* ---------------------------------------------------- */
        /* INTERNAL SUPERVISOR ROUTINE (~50Hz)                  */
        /* ---------------------------------------------------- */
        if (Supervisor_GetCurrentState() != STATE_FAULT)
        {
            /* 1. Mobility Subsystem Check */
            uint8_t current_mob_wdg = RobotState_GetWatchdogMobility();
            if (current_mob_wdg == prev_mob_wdg) {
                stall_timer_mob += 20;
                if (stall_timer_mob >= 500) { /* 500ms timeout */
                    RobotState_SetErrorFlag(ERR_MOB_STALL);
                }
            } else {
                stall_timer_mob = 0;
                prev_mob_wdg = current_mob_wdg;
            }

            /* 2. Arm Subsystem Check */
            uint8_t current_arm_wdg = RobotState_GetWatchdogArm();
            if (current_arm_wdg == prev_arm_wdg) {
                stall_timer_arm += 20;
                if (stall_timer_arm >= 500) { /* 500ms timeout */
                    RobotState_SetErrorFlag(ERR_ARM_STALL);
                }
            } else {
                stall_timer_arm = 0;
                prev_arm_wdg = current_arm_wdg;
            }

            /* 3. Global Error Registry Evaluation */
            uint64_t current_errors = RobotState_GetErrorFlags();
            if (ERR_IS_ANY(current_errors))
            {
                printf("Manager: CRITICAL HARDWARE FAULT DETECTED (Flags: 0x%08lX%08lX). Triggering System Supervisor FAULT.\r\n",
                       (unsigned long)(current_errors >> 32), (unsigned long)(current_errors & 0xFFFFFFFF));
                Supervisor_ProcessEvent(EVENT_ERROR, SRC_INTERNAL_SUPERVISOR);
            }
        }
    }
}

