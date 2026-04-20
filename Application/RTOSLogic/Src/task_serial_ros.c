#include "app_rtos.h"
#include "supervisor_fsm.h"
#include "app_config.h"
#include "bsp_serial_ros.h"
#include "serial_ros.h"
#include "robot_state.h"
#include "debug_module.h"
#include "FreeRTOS.h"
#include "task.h"
#include <string.h>

#define LOG_TAG "TASK_S_ROS"
#define RX_EVENT_FLAG 0x01
#define TX_EVENT_FLAG 0x02

static osal_thread_h serial_ros_task_id;
static uint8_t shared_rx_buffer[512];
static uint16_t shared_rx_size = 0;

/**
 * @brief Bridge callback from BSP (runs in IRQ context)
 */
static void bsp_rx_callback(uint8_t *data, uint16_t size) {
    /* Critical section for ISR context to protect shared buffer during append */
    UBaseType_t uxSavedInterruptStatus = taskENTER_CRITICAL_FROM_ISR();
    
    uint16_t remaining = sizeof(shared_rx_buffer) - shared_rx_size;
    uint16_t to_copy = (size > remaining) ? remaining : size;
    
    if (to_copy > 0) {
        memcpy(&shared_rx_buffer[shared_rx_size], data, to_copy);
        shared_rx_size += to_copy;
    }
    
    taskEXIT_CRITICAL_FROM_ISR(uxSavedInterruptStatus);
    osal_thread_flags_set(serial_ros_task_id, RX_EVENT_FLAG);
}

/**
 * @brief TX complete callback from HAL
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART3) {
        if (serial_ros_task_id != NULL) {
            osal_thread_flags_set(serial_ros_task_id, TX_EVENT_FLAG);
        }
    }
}

/**
 * @brief Main SerialRos Task
 */
void StartSerialRosTask(void *argument) {
    serial_ros_task_id = osal_thread_get_self();
    LOG_INFO(LOG_TAG, "SerialRos Task Multi-tasking dispatcher started\r\n");

    /* Init Module and Hardware */
    SerialRos_Init();
    BSP_SerialRos_Init(bsp_rx_callback);

    SerialRos_Packet_t tx_packet;
    bool has_pending_tx = false;

    uint32_t last_conn_check_tick = osal_get_tick();
    bool was_connected = false;

    uint8_t local_rx_buffer[512];
    uint16_t local_rx_size;

    for (;;) {
        /* 1. Retrieve a TX packet if we don't currently have one */
        if (!has_pending_tx) {
            /* Wait up to 2ms to ensure RX events are also processed promptly */
            if (osal_queue_get(rosTxQueueHandle, &tx_packet, 2U) == OSAL_OK) {
                has_pending_tx = true;
            }
        }

        /* 2. Try to transmit if UART is Free */
        if (has_pending_tx && BSP_SerialRos_IsTxReady()) {
            BSP_SerialRos_Transmit(tx_packet.data, tx_packet.size);
            has_pending_tx = false;
        }

        /* 3. Check for RX packet notifications (non-blocking) */
        uint32_t flags = osal_thread_flags_wait(RX_EVENT_FLAG | TX_EVENT_FLAG, 0U);

        /* Flags check: Ensure no CMSIS error code (bit 31) */
        if (!(flags & 0x80000000U)) {
            if (flags & RX_EVENT_FLAG) {
                /* Snapshot the shared RX buffer to minimize race conditions with next ISR */
                taskENTER_CRITICAL();
                local_rx_size = shared_rx_size;
                memcpy(local_rx_buffer, shared_rx_buffer, local_rx_size);
                shared_rx_size = 0; /* Reset size */
                taskEXIT_CRITICAL();

                /* Process raw packet in the module */
                SerialRos_ProcessPacket(local_rx_buffer, local_rx_size);
                
                /* Put the processed packet into the RX queue for other tasks */
                SerialRos_Packet_t rx_packet;
                rx_packet.size = (local_rx_size > sizeof(rx_packet.data)) ? sizeof(rx_packet.data) : local_rx_size;
                memcpy(rx_packet.data, local_rx_buffer, rx_packet.size);
                osal_queue_put(rosRxQueueHandle, &rx_packet, 0); 
            }
        }

        /* 4. If we STILL have pending TX (UART was busy), wait for TX finish or timeout */
        if (has_pending_tx) {
            osal_thread_flags_wait(RX_EVENT_FLAG | TX_EVENT_FLAG, 2U);
        }

        /* 5. Connection Monitoring (approx every 500ms) */
        uint32_t current_tick = osal_get_tick();
        if ((current_tick - last_conn_check_tick) >= 500) {
            last_conn_check_tick = current_tick;
            bool is_connected = SerialRos_IsConnected();

            if (was_connected && !is_connected) {
                /* Disconnection Pulse Detected */
                LOG_WARNING(LOG_TAG, "Client Disconnected.\r\n");
                if (RobotState_GetSystemState() == STATE_SUPERVISOR_AUTO) {
                    Supervisor_ProcessEvent(EVENT_SUPERVISOR_MODE_MANUAL, SRC_INTERNAL_SUPERVISOR);
                }
            } else if (!was_connected && is_connected) {
                LOG_INFO(LOG_TAG, "Client Connected.\r\n");
            }
            was_connected = is_connected;
        }
    }
}
