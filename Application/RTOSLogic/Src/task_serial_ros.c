#include "app_rtos.h"
#include "bsp_serial_ros.h"
#include "serial_ros.h"
#include "robot_state.h"
#include "debug_module.h"
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
    uint8_t telemetry_buf[64];
    uint32_t last_telemetry_tick = osal_get_tick();
    bool was_connected = false;
    uint32_t last_conn_check_tick = osal_get_tick();

    for (;;) {
        /* 1. Wait for RX packet notification (short timeout to poll TX queue and periodic logic) */
        uint32_t flags = osal_thread_flags_wait(RX_EVENT_FLAG, 10U);

        if (flags & RX_EVENT_FLAG) {
            /* Process raw packet in the module */
            SerialRos_ProcessPacket(shared_rx_buffer, shared_rx_size);
            
            /* Optional: Put the processed packet into the RX queue for other tasks */
            SerialRos_Packet_t rx_packet;
            rx_packet.size = (shared_rx_size > 64) ? 64 : shared_rx_size;
            memcpy(rx_packet.data, shared_rx_buffer, rx_packet.size);
            osal_queue_put(rosRxQueueHandle, &rx_packet, 0); // Non-blocking
        }

        /* 2. Periodic Telemetry (approx 50Hz / 20ms) */
        uint32_t current_tick = osal_get_tick();
        if ((current_tick - last_telemetry_tick) >= 20) {
            last_telemetry_tick = current_tick;
            uint16_t t_size = SerialRos_BuildTelemetryPacket(telemetry_buf, sizeof(telemetry_buf));
            if (t_size > 0) {
                BSP_SerialRos_Transmit(telemetry_buf, t_size);
                // We add a small delay after DMA start if we expect burst from queue immediately
                // However, DMA is handled by HAL and we'll check status in the sender loop.
            }
        }

        /* 3. Send queued packets from other tasks */
        while (osal_queue_get(rosTxQueueHandle, &tx_packet, 0) == OSAL_OK) {
            BSP_SerialRos_Transmit(tx_packet.data, tx_packet.size);
            osal_delay(1); 
        }

        /* 4. Connection Monitoring (approx every 100ms) */
        if ((current_tick - last_conn_check_tick) >= 100) {
            last_conn_check_tick = current_tick;
            bool is_connected = SerialRos_IsConnected();

            if (was_connected && !is_connected) {
                /* Disconnection Pulse Detected */
                LOG_WARNING(LOG_TAG, "Jetson Disconnected! Switching to MANUAL mode for safety.");
                
                if (Supervisor_GetCurrentState() == STATE_AUTO) {
                    Supervisor_ProcessEvent(EVENT_MODE_MANUAL, SRC_INTERNAL_SUPERVISOR);
                }
            }
            
            was_connected = is_connected;
        }
    }
}
