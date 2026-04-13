#include "app_rtos.h"
#include "bsp_console.h"
#include "debug_module.h"
#include <string.h>
#include <stdio.h>

#define LOG_TAG "UART_LISTENER"
#define RX_BUF_SIZE 128

/* Thread notification flag */
#define RX_EVENT_FLAG 0x01

static osThreadId_t listener_task_id;

/**
 * @brief Helper to publish events to the central controller
 */
static void publish_event(SystemEvent_t event)
{
    StateChangeMsg_t msg;
    msg.event = event;
    msg.timestamp = osKernelGetTickCount();
    
    osStatus_t status = osMessageQueuePut(uartEventQueueHandle, &msg, 0U, 0U);
    if (status != osOK) {
        LOG_ERROR(LOG_TAG, "Failed to publish event %d", event);
    }
}

/**
 * @brief Bridge between BSP callback and RTOS Task
 */
static void bsp_rx_callback(uint16_t size)
{
    /* Notify the task that data is ready */
    osThreadFlagsSet(listener_task_id, RX_EVENT_FLAG);
}

/**
 * @brief Thread entry point for UART Listener Task
 */
void StartUARTListenerTask(void *argument)
{
    listener_task_id = osThreadGetId();
    LOG_INFO(LOG_TAG, "UART Listener Task Started (Clean Architecture)");

    /* Initialize BSP Rx and register our local bridge callback */
    BSP_Console_InitRx(bsp_rx_callback);

    char cmd_buffer[RX_BUF_SIZE];

    for(;;)
    {
        /* Wait for notification from BSP callback */
        uint32_t flags = osThreadFlagsWait(RX_EVENT_FLAG, osFlagsWaitAny, osWaitForever);
        
        if (flags & RX_EVENT_FLAG)
        {
            /* Copy data from BSP internal buffer */
            BSP_Console_GetData((uint8_t *)cmd_buffer, RX_BUF_SIZE);
            cmd_buffer[RX_BUF_SIZE - 1] = '\0';

            LOG_DEBUG(LOG_TAG, "Received: %s", cmd_buffer);

            /* Parse Command */
            if (strncmp(cmd_buffer, "EVENT:", 6) == 0) 
            {
                char *cmd = &cmd_buffer[6];
                strtok(cmd, "\r\n ");

                if (strcmp(cmd, "START") == 0) {
                    LOG_INFO(LOG_TAG, "PC Command: START");
                    publish_event(EVENT_START);
                } else if (strcmp(cmd, "STOP") == 0) {
                    LOG_INFO(LOG_TAG, "PC Command: STOP");
                    publish_event(EVENT_STOP);
                } else if (strcmp(cmd, "ERROR") == 0) {
                    LOG_INFO(LOG_TAG, "PC Command: ERROR");
                    publish_event(EVENT_ERROR);
                } else if (strcmp(cmd, "RESET") == 0) {
                    LOG_INFO(LOG_TAG, "PC Command: RESET");
                    publish_event(EVENT_RESET);
                } else {
                    LOG_WARNING(LOG_TAG, "Unknown PC command: '%s'", cmd);
                }
            }

            /* Clear data and tell BSP we are ready for next packet */
            BSP_Console_AcceptNext();
        }
    }
}
