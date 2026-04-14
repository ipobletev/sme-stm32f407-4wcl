#include "serial_ros.h"
#include "serial_ros_protocol.h"
#include "robot_state.h"
#include "debug_module.h"
#include <string.h>
#include "app_rtos.h"

#define LOG_TAG "SERIAL_ROS_MOD"

static uint32_t last_rx_tick = 0;

void SerialRos_Init(void) {
    last_rx_tick = 0; // Initialize as disconnected
    LOG_INFO(LOG_TAG, "SerialRos Module Initialized");
}

bool SerialRos_IsConnected(void) {
    if (last_rx_tick == 0) return false;
    return (osal_get_tick() - last_rx_tick) < SERIAL_ROS_COMMS_TIMEOUT_MS;
}

void SerialRos_ProcessPacket(uint8_t *buffer, uint16_t size) {
    if (size < SERIAL_ROS_MIN_SIZE) return;

    if (buffer[0] == SERIAL_ROS_SYNC1 && buffer[1] == SERIAL_ROS_SYNC2) {
        uint8_t msg_id = buffer[2];
        uint8_t len = buffer[3];

        if (size >= (4 + len + 2)) {
            uint16_t received_crc;
            memcpy(&received_crc, &buffer[4 + len], 2);
            uint16_t computed_crc = SerialRos_CRC16(buffer, 4 + len);

            if (received_crc == computed_crc) {
                last_rx_tick = osal_get_tick();
                /* Dispatch to RobotState */
                if (msg_id == TOPIC_ID_CMD_VEL) {
                    CmdVelMsg_t *cmd = (CmdVelMsg_t *)&buffer[4];
                    RobotState_SetTargetVelocity(cmd->linear_x, cmd->angular_z);
                    LOG_DEBUG(LOG_TAG, "CmdVel Rx: x=%.2f, z=%.2f", cmd->linear_x, cmd->angular_z);
                }
                else if (msg_id == TOPIC_ID_ARM_GOAL) {
                    ArmGoalMsg_t *arm = (ArmGoalMsg_t *)&buffer[4];
                    RobotState_SetTargetArmPose(arm->j1, arm->j2, arm->j3);
                }
                else if (msg_id == TOPIC_ID_CONFIG) {
                    SysConfigMsg_t *cfg = (SysConfigMsg_t *)&buffer[4];
                    RobotState_SetTargetMobilityMode(cfg->mobility_mode);
                    RobotState_SetAutonomous(cfg->is_autonomous);
                    LOG_INFO(LOG_TAG, "Config Rx: Mode=%u, Auto=%u", cfg->mobility_mode, cfg->is_autonomous);
                }
                else if (msg_id == TOPIC_ID_SYS_EVENT) {
                    // Handle system events if needed
                }
            } else {
                LOG_ERROR(LOG_TAG, "CRC Error on ID 0x%02X", msg_id);
            }
        }
    }
}

uint16_t SerialRos_BuildTelemetryPacket(uint8_t *out_buffer, uint16_t max_size) {
    if (max_size < (4 + sizeof(SystemStatusMsg_t) + 2)) return 0;

    SystemStatusMsg_t status_msg;
    status_msg.current_state = (uint8_t)RobotState_GetSystemState();
    status_msg.error_flags = RobotState_GetErrorFlags();
    status_msg.mcu_temp = RobotState_4wcl.Telemetry.uc_temperature;
    status_msg.battery_voltage = RobotState_4wcl.Telemetry.battery_voltage;
    status_msg.battery_current = RobotState_4wcl.Telemetry.battery_current;

    out_buffer[0] = SERIAL_ROS_SYNC1;
    out_buffer[1] = SERIAL_ROS_SYNC2;
    out_buffer[2] = TOPIC_ID_SYS_STATUS;
    out_buffer[3] = sizeof(SystemStatusMsg_t);
    memcpy(&out_buffer[4], &status_msg, sizeof(SystemStatusMsg_t));
    uint16_t crc = SerialRos_CRC16(out_buffer, 4 + sizeof(SystemStatusMsg_t));
    memcpy(&out_buffer[4 + sizeof(SystemStatusMsg_t)], &crc, 2);
    return (4 + sizeof(SystemStatusMsg_t) + 2);
}

bool SerialRos_EnqueueTx(uint8_t topic_id, void* msg, uint8_t len) {
    if (len > 58) return false; // 64 - 4 (header) - 2 (crc) = 58 max payload

    SerialRos_Packet_t packet;
    packet.data[0] = SERIAL_ROS_SYNC1;
    packet.data[1] = SERIAL_ROS_SYNC2;
    packet.data[2] = topic_id;
    packet.data[3] = len;
    
    if (msg != NULL && len > 0) {
        memcpy(&packet.data[4], msg, len);
    }

    uint16_t crc = SerialRos_CRC16(packet.data, 4 + len);
    memcpy(&packet.data[4 + len], &crc, 2);
    packet.size = 4 + len + 2;

    osal_status_t status = osal_queue_put(rosTxQueueHandle, &packet, 100);
    return (status == OSAL_OK);
}

bool SerialRos_DequeueRx(SerialRos_Packet_t* out_packet, uint32_t timeout_ms) {
    osal_status_t status = osal_queue_get(rosRxQueueHandle, out_packet, timeout_ms);
    return (status == OSAL_OK);
}
