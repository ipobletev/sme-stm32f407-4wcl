#include "app_rtos.h"
#include "bsp_serial_ros.h"
#include "serial_ros.h"
#include "robot_state.h"
#include "debug_module.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

#define LOG_TAG "TASK_S_ROS"
#define RX_EVENT_FLAG 0x01

static osal_thread_h serial_ros_task_id;
static uint8_t shared_rx_buffer[256];
static uint16_t shared_rx_size = 0;

/**
 * @brief Bridge callback from BSP (runs in IRQ context)
 */
static void bsp_rx_callback(uint8_t *data, uint16_t size) {
    if (size > 256) size = 256;
    memcpy(shared_rx_buffer, data, size);
    shared_rx_size = size;
    osal_thread_flags_set(serial_ros_task_id, RX_EVENT_FLAG);
}

/**
 * @brief Main SerialRos Task
 */
void StartSerialRosTask(void *argument) {
    serial_ros_task_id = osal_thread_get_self();
    LOG_INFO(LOG_TAG, "SerialRos Task Multi-tasking dispatcher started");

    /* Init Module and Hardware */
    SerialRos_Init();
    BSP_SerialRos_Init(bsp_rx_callback);

    SerialRos_Packet_t tx_packet;
    uint32_t last_conn_check_tick = osal_get_tick();
    bool was_connected = false;

    uint8_t local_rx_buffer[256];
    uint16_t local_rx_size;

    for (;;) {
        /* 1. Wait for TX data from other tasks (IMU, Odom, etc.) 
         * This is now the primary blocking point for the task.
         */
        if (osal_queue_get(rosTxQueueHandle, &tx_packet, 10U) == OSAL_OK) {
            /* Transmit the first packet */
            BSP_SerialRos_Transmit(tx_packet.data, tx_packet.size);
            
            /* Drain all other pending transmission packets immediately */
            while (osal_queue_get(rosTxQueueHandle, &tx_packet, 0) == OSAL_OK) {
                BSP_SerialRos_Transmit(tx_packet.data, tx_packet.size);
            }
        }

        /* 2. Check for RX packet notifications (non-blocking) */
        uint32_t flags = osal_thread_flags_wait(RX_EVENT_FLAG, 0U);

        if (flags & RX_EVENT_FLAG) {
            /* Snapshot the shared RX buffer to minimize race conditions with next ISR */
            taskENTER_CRITICAL();
            local_rx_size = shared_rx_size;
            memcpy(local_rx_buffer, shared_rx_buffer, local_rx_size);
            taskEXIT_CRITICAL();

            /* Process raw packet in the module */
            SerialRos_ProcessPacket(local_rx_buffer, local_rx_size);
            
            /* Put the processed packet into the RX queue for other tasks */
            SerialRos_Packet_t rx_packet;
            rx_packet.size = (local_rx_size > 64) ? 64 : local_rx_size;
            memcpy(rx_packet.data, local_rx_buffer, rx_packet.size);
            osal_queue_put(rosRxQueueHandle, &rx_packet, 0); 
        }

        /* 3. Connection Monitoring (approx every 100ms) */
        uint32_t current_tick = osal_get_tick();
        if ((current_tick - last_conn_check_tick) >= 100) {
            last_conn_check_tick = current_tick;
            bool is_connected = SerialRos_IsConnected();

            if (was_connected && !is_connected) {
                /* Disconnection Pulse Detected */
                LOG_WARNING(LOG_TAG, "Jetson Disconnected! Safety fallback to MANUAL.");
                if (RobotState_GetSystemState() == STATE_AUTO) {
                    Supervisor_ProcessEvent(EVENT_MODE_MANUAL, SRC_INTERNAL_SUPERVISOR);
                }
            }
            was_connected = is_connected;
        }
    }
}
