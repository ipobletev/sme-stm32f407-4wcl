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
 * @brief Bridge between BSP callback and RTOS Task
 */
static void bsp_rx_callback(uint16_t size)
{
    /* Notify the task that data is ready */
    osal_thread_flags_set(listener_task_id, RX_EVENT_FLAG);
}

#include "serial_ros.h"
#include "serial_ros_protocol.h"

/**
 * @brief Thread entry point for UART Listener Task
 */
void StartUARTListenerTask(void *argument)
{
    listener_task_id = osal_thread_get_self();
    LOG_INFO(LOG_TAG, "Console Listener Task Started (Binary Protocol Only)\r\n");

    /* Initialize BSP Rx and register our local bridge callback */
    BSP_Console_InitRx(bsp_rx_callback);

    Console_Packet_t io_packet;
    uint8_t rx_buffer[RX_BUF_SIZE];

    for(;;)
    {
        /* 1. Wait for RX event or 10ms timeout to poll TX queue */
        uint32_t flags = osal_thread_flags_wait(RX_EVENT_FLAG, 10U);
        
        if (flags & RX_EVENT_FLAG)
        {
            /* Copy data from BSP internal buffer */
            uint16_t size = BSP_Console_GetData(rx_buffer, RX_BUF_SIZE);
            if (size > 0) {
                /* Put in RX queue for other tasks if they want raw access (Legacy) */
                Console_Packet_t rx_pkg;
                rx_pkg.size = (size > sizeof(rx_pkg.data)) ? sizeof(rx_pkg.data) : size;
                memcpy(rx_pkg.data, rx_buffer, rx_pkg.size);
                osal_queue_put(consoleRxQueueHandle, &rx_pkg, 0);

                /* --- UNIFIED BINARY DISPATCHER --- */
                if (size >= 2 && rx_buffer[0] == SERIAL_ROS_SYNC1 && rx_buffer[1] == SERIAL_ROS_SYNC2) {
                    SerialRos_ProcessPacket(rx_buffer, size);
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

