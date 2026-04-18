#include "app_rtos.h"
#include "robot_state.h"
#include "supervisor_fsm.h"
#include <stdio.h>

void StartManagerTask(void *argument)
{
    StateChangeMsg_t msg;

    /* Initialize the internal supervisor logic */
    Supervisor_Init();

    printf("Manager Task Started. Ready to process events.\r\n");

    for(;;)
    {
        /* 1. Event Processing (~50Hz check) */
        /* Wait for a message from other tasks, block for 20ms */
        if (osal_queue_get(stateMsgQueueHandle, &msg, 20U) == OSAL_OK)
        {
            printf("Manager: Processing Event %d collected at tick %lu\r\n", 
                   msg.event, (unsigned long)msg.timestamp);

            /* Delegate the transition logic to the Supervisor module */
            Supervisor_ProcessEvent(msg.event, msg.source);
        }

        /* 2. Periodic State Machine Logic (~50Hz) */
        /* This replaces the legacy hardcoded watchdog logic that was here */
        Supervisor_ProcessLogic();
    }
}
