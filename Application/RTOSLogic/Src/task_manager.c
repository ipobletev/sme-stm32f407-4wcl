#include "app_rtos.h"
#include <stdio.h>

void StartManagerTask(void *argument)
{
    StateChangeMsg_t msg;

    /* Initialize the internal state machine logic */
    SM_Init();

    printf("Manager Task Started. Ready to process events.\r\n");

    for(;;)
    {
        /* Wait for a message from the Controller task, block indefinitely */
        if (osMessageQueueGet(stateMsgQueueHandle, &msg, 0U, osWaitForever) == osOK)
        {
            printf("Manager: Processing Event %d collected at tick %lu\r\n", 
                   msg.event, (unsigned long)msg.timestamp);

            /* Delegate the transition logic to the State Machine module */
            SM_ProcessEvent(msg.event);
        }
    }
}
