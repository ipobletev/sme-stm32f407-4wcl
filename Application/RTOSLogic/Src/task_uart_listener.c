#include "app_rtos.h"
#include "bsp_console.h"
#include "debug_module.h"
#include <string.h>
#include <stdio.h>

#define LOG_TAG "UART_LISTENER"
#define RX_BUF_SIZE 128

/* Thread notification flag */
#define RX_EVENT_FLAG 0x01

static osal_thread_h listener_task_id;

/**
 * @brief Helper to publish events to the central controller
 */
static void publish_event(SystemEvent_t event, EventSource_t source)
{
    StateChangeMsg_t msg;
    msg.event = event;
    msg.timestamp = osal_get_tick();
    msg.source = source;
    
    osal_status_t status = osal_queue_put(uartEventQueueHandle, &msg, 0U);
    if (status != OSAL_OK) {
        LOG_ERROR(LOG_TAG, "Failed to publish event %d", event);
    }
}

/**
 * @brief Bridge between BSP callback and RTOS Task
 */
static void bsp_rx_callback(uint16_t size)
{
    /* Notify the task that data is ready */
    osal_thread_flags_set(listener_task_id, RX_EVENT_FLAG);
}

/**
 * @brief Thread entry point for UART Listener Task
 */
void StartUARTListenerTask(void *argument)
{
    listener_task_id = osal_thread_get_self();
    LOG_INFO(LOG_TAG, "Console Dispatcher Task Started (TX/RX Queues)");

    /* Initialize BSP Rx and register our local bridge callback */
    BSP_Console_InitRx(bsp_rx_callback);

    Console_Packet_t io_packet;
    char cmd_buffer[RX_BUF_SIZE];

    for(;;)
    {
        /* 1. Wait for RX event or 10ms timeout to poll TX queue */
        uint32_t flags = osal_thread_flags_wait(RX_EVENT_FLAG, 10U);
        
        if (flags & RX_EVENT_FLAG)
        {
            /* Copy data from BSP internal buffer */
            uint16_t size = BSP_Console_GetData((uint8_t *)cmd_buffer, RX_BUF_SIZE);
            if (size > 0) {
                cmd_buffer[size < RX_BUF_SIZE ? size : RX_BUF_SIZE - 1] = '\0';

                /* Put in RX queue for other tasks if they want raw access */
                Console_Packet_t rx_pkg;
                rx_pkg.size = (size > 128) ? 128 : size;
                memcpy(rx_pkg.data, cmd_buffer, rx_pkg.size);
                osal_queue_put(consoleRxQueueHandle, &rx_pkg, 0);

                /* Parse Command (legacy logic for system events) */
                if (strncmp(cmd_buffer, "EVENT:", 6) == 0) 
                {
                    char *cmd = &cmd_buffer[6];
                    strtok(cmd, "\r\n ");

                    if (strcmp(cmd, "START") == 0)      publish_event(EVENT_START, SRC_UART1_LOCAL);
                    else if (strcmp(cmd, "STOP") == 0)  publish_event(EVENT_STOP, SRC_UART1_LOCAL);
                    else if (strcmp(cmd, "MANUAL") == 0) publish_event(EVENT_MODE_MANUAL, SRC_UART1_LOCAL);
                    else if (strcmp(cmd, "AUTO") == 0)   publish_event(EVENT_MODE_AUTO, SRC_UART1_LOCAL);
                    else if (strcmp(cmd, "PAUSE") == 0)  publish_event(EVENT_PAUSE, SRC_UART1_LOCAL);
                    else if (strcmp(cmd, "RESUME") == 0) publish_event(EVENT_RESUME, SRC_UART1_LOCAL);
                    else if (strcmp(cmd, "RESET") == 0)  publish_event(EVENT_RESET, SRC_UART1_LOCAL);
                }
            }

            /* Clear data and tell BSP we are ready for next packet */
            BSP_Console_AcceptNext();
        }

        /* 2. Process Outgoing Logs/Data (TX Queue) */
        while (osal_queue_get(consoleTxQueueHandle, &io_packet, 0) == OSAL_OK) {
            BSP_Console_Send(io_packet.data, io_packet.size);
            // Small delay to allow DMA to start and avoid slamming HAL
            // In a better implementation, we'd wait for a DMA-complete notification.
            osal_delay(1);
        }
    }
}

