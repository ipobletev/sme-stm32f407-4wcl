/**
 * @file    serial_ros.c
 * @brief   SerialRos Module — Binary protocol parser and telemetry builder.
 *
 * Rx topic dispatch strategy:
 *   - Simple state writes  → RobotState_* API (direct, thread-safe)
 *   - Supervisor events    → Supervisor_SendEvent()
 *   - Complex control      → Dedicated dummy handlers (TODO stubs)
 */

#include "serial_ros.h"
#include "serial_ros_protocol.h"
#include "robot_state.h"
#include "supervisor_fsm.h"
#include "debug_module.h"
#include <string.h>
#include "app_rtos.h"
#include "app_config.h"

#define LOG_TAG "SERIAL_ROS_MOD"

/* =========================================================================
 * Private State
 * ========================================================================= */

static uint32_t last_rx_tick = 0;

/* =========================================================================
 * Private: Rx Topic Handlers (dummy stubs for complex control)
 * ========================================================================= */

#include "command_dispatcher.h"

/* =========================================================================
 * Private: Rx Topic Handlers
 * ========================================================================= */

static void handle_autonomous(const AutonomousMsg_t *msg) {
    CmdDispatcher_SetMobilityConfig(RobotState_GetTargetMobilityMode(), msg->is_autonomous, SRC_EXT_CLIENT);
}

static void handle_mobility_mode(const SysConfigMsg_t *msg) {
    CmdDispatcher_SetMobilityConfig(msg->mobility_mode, msg->is_autonomous, SRC_EXT_CLIENT);
}

static void handle_cmd_vel(const CmdVelMsg_t *msg) {
    CmdDispatcher_SetVelocity(msg->linear_x, msg->angular_z, SRC_EXT_CLIENT);
}

static void handle_arm_goal(const ArmGoalMsg_t *msg) {
    CmdDispatcher_SetArmGoal(msg->j1, msg->j2, msg->j3, SRC_EXT_CLIENT);
}

static void handle_sys_event(const SysEventMsg_t *msg) {
    SystemEvent_t event = EVENT_SUPERVISOR_NONE;
    switch (msg->event_id) {
        case SYS_EVENT_START:   event = EVENT_SUPERVISOR_START;       break;
        case SYS_EVENT_STOP:    event = EVENT_SUPERVISOR_STOP;        break;
        case SYS_EVENT_PAUSE:   event = EVENT_SUPERVISOR_PAUSE;       break;
        case SYS_EVENT_RESUME:  event = EVENT_SUPERVISOR_RESUME;      break;
        case SYS_EVENT_RESET:   event = EVENT_SUPERVISOR_RESET;       break;
        case SYS_EVENT_FAULT:   event = EVENT_SUPERVISOR_ERROR;       break;
        case SYS_EVENT_TEST:    event = EVENT_SUPERVISOR_TESTING;     break;
        default: break;
    }
    if (event != EVENT_SUPERVISOR_NONE) {
        CmdDispatcher_TriggerEvent(event, SRC_EXT_CLIENT);
    }
}

static void handle_set_config(const SetConfigMsg_t *msg) {
    CmdDispatcher_UpdateConfig(msg->id, msg->value, SRC_EXT_CLIENT);
    /* Broadcast back the full current config to keep all clients in sync */
    AppConfig_t* config = AppConfig_Get();
    SerialRos_EnqueueTx(TOPIC_ID_APP_CONFIG_DATA, config, sizeof(AppConfig_t));
}

static void handle_get_config(void) {
#ifdef PESISTENT_CONFIG
    AppConfig_ReloadFromFlash();
#endif
    AppConfig_t* config = AppConfig_Get();
    SerialRos_EnqueueTx(TOPIC_ID_APP_CONFIG_DATA, config, sizeof(AppConfig_t));
}

static void handle_actuator_test(const ActuatorTestMsg_t *msg) {
    CmdDispatcher_ActuatorTest(msg->actuator_id, msg->pulse, false, SRC_EXT_CLIENT);
}

static void handle_actuator_velocity(const ActuatorTestMsg_t *msg) {
    CmdDispatcher_ActuatorTest(msg->actuator_id, msg->pulse, true, SRC_EXT_CLIENT);
}


/* =========================================================================
 * Public API
 * ========================================================================= */

void SerialRos_Init(void) {
    last_rx_tick = 0;
    LOG_INFO(LOG_TAG, "SerialRos Module Initialized\r\n");
}

bool SerialRos_IsConnected(void) {
    if (last_rx_tick == 0) return false;
    return (osal_get_tick() - last_rx_tick) < SERIAL_ROS_COMMS_TIMEOUT_MS;
}

void SerialRos_ProcessPacket(uint8_t *buffer, uint16_t size)
{
    if (size < SERIAL_ROS_MIN_SIZE) return;

    if (buffer[0] != SERIAL_ROS_SYNC1 || buffer[1] != SERIAL_ROS_SYNC2) return;

    uint8_t  msg_id = buffer[2];
    uint8_t  len    = buffer[3];

    if (size < (uint16_t)(4 + len + 2)) return;

    /* CRC validation covers header + payload */
    uint16_t received_crc;
    memcpy(&received_crc, &buffer[4 + len], 2);
    uint16_t computed_crc = SerialRos_CRC16(buffer, 4 + len);

    if (received_crc != computed_crc) {
        LOG_ERROR(LOG_TAG, "CRC error on ID 0x%02X (got 0x%04X, exp 0x%04X)",
                  msg_id, received_crc, computed_crc);
        return;
    }

    /* Valid packet — refresh connection timestamp */
    last_rx_tick = osal_get_tick();

    const uint8_t *payload = &buffer[4];

    /* Dispatch to per-topic handlers */
    switch (msg_id) {

        case TOPIC_ID_HEARTBEAT:
            /* Heartbeat received — Explicitly refresh connection tick as requested */
            last_rx_tick = osal_get_tick();
            //LOG_INFO(LOG_TAG, "Heartbeat Handshake OK\r\n");
            break;

        case TOPIC_ID_AUTONOMOUS:
            if (len >= sizeof(AutonomousMsg_t)) {
                handle_autonomous((const AutonomousMsg_t *)payload);
            }
            break;

        case TOPIC_ID_MOBILITY_MODE:
            if (len >= sizeof(SysConfigMsg_t)) {
                handle_mobility_mode((const SysConfigMsg_t *)payload);
            }
            break;

        case TOPIC_ID_CMD_VEL:
            if (len >= sizeof(CmdVelMsg_t)) {
                handle_cmd_vel((const CmdVelMsg_t *)payload);
            }
            break;

        case TOPIC_ID_ARM_GOAL:
            if (len >= sizeof(ArmGoalMsg_t)) {
                handle_arm_goal((const ArmGoalMsg_t *)payload);
            }
            break;

        case TOPIC_ID_SYS_EVENT:
            if (len >= sizeof(SysEventMsg_t)) {
                handle_sys_event((const SysEventMsg_t *)payload);
            }
            break;

        case TOPIC_ID_ACTUATOR_PWM:
            if (len >= sizeof(ActuatorTestMsg_t)) {
                handle_actuator_test((const ActuatorTestMsg_t *)payload);
            }
            break;

        case TOPIC_ID_ACTUATOR_VEL:
            if (len >= sizeof(ActuatorTestMsg_t)) {
                handle_actuator_velocity((const ActuatorTestMsg_t *)payload);
            }
            break;

        case TOPIC_ID_SET_CONFIG:
            if (len >= sizeof(SetConfigMsg_t)) {
                handle_set_config((const SetConfigMsg_t *)payload);
            }
            break;

        case TOPIC_ID_GET_CONFIG:
            handle_get_config();
            break;
            
        case TOPIC_ID_SAVE_CONFIG:
            LOG_INFO(LOG_TAG, "Command: SAVE CONFIG to Flash\r\n");
            AppConfig_Save();
            break;

        default:
            LOG_WARNING(LOG_TAG, "Unknown topic ID 0x%02X", msg_id);
            break;
    }
}

uint16_t SerialRos_BuildTelemetryPacket(uint8_t *out_buffer, uint16_t max_size)
{
    if (max_size < (4 + sizeof(SystemStatusMsg_t) + 2)) return 0;

    SystemStatusMsg_t status_msg;
    status_msg.error_flags     = RobotState_GetErrorFlags();
    status_msg.mcu_temp        = RobotState_4wcl.Telemetry.uc_temperature;
    status_msg.battery_voltage = RobotState_4wcl.Telemetry.battery_voltage;
    status_msg.current_state   = (uint8_t)RobotState_GetSystemState();
    status_msg.mobility_state  = (uint8_t)RobotState_GetMobilityState();
    status_msg.arm_state       = (uint8_t)RobotState_GetArmState();
    /* status_msg.battery_current = ... (Unsupported) */

    out_buffer[0] = SERIAL_ROS_SYNC1;
    out_buffer[1] = SERIAL_ROS_SYNC2;
    out_buffer[2] = TOPIC_ID_SYS_STATUS;
    out_buffer[3] = sizeof(SystemStatusMsg_t);
    memcpy(&out_buffer[4], &status_msg, sizeof(SystemStatusMsg_t));
    uint16_t crc = SerialRos_CRC16(out_buffer, 4 + sizeof(SystemStatusMsg_t));
    memcpy(&out_buffer[4 + sizeof(SystemStatusMsg_t)], &crc, 2);
    return (4 + sizeof(SystemStatusMsg_t) + 2);
}

bool SerialRos_EnqueueTx(uint8_t topic_id, void *msg, uint8_t len)
{
    SerialRos_Packet_t packet;
    /* Max payload = Total buffer - (4 byte header + 2 byte CRC) */
    if (len > (sizeof(packet.data) - 6)) return false;

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

bool SerialRos_DequeueRx(SerialRos_Packet_t *out_packet, uint32_t timeout_ms)
{
    osal_status_t status = osal_queue_get(rosRxQueueHandle, out_packet, timeout_ms);
    return (status == OSAL_OK);
}
