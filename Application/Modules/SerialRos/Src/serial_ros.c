/**
 * @file    serial_ros.c
 * @brief   SerialRos Module — Binary protocol parser and telemetry builder.
 *
 * Rx topic dispatch strategy:
 *   - Simple state writes  → RobotState_* API (direct, thread-safe)
 *   - Supervisor events    → Supervisor_ProcessEvent()
 *   - Complex control      → Dedicated dummy handlers (TODO stubs)
 */

#include "serial_ros.h"
#include "serial_ros_protocol.h"
#include "robot_state.h"
#include "supervisor_fsm.h"
#include "debug_module.h"
#include <string.h>
#include "app_rtos.h"

#define LOG_TAG "SERIAL_ROS_MOD"

/* =========================================================================
 * Private State
 * ========================================================================= */

static uint32_t last_rx_tick = 0;

/* =========================================================================
 * Private: Rx Topic Handlers (dummy stubs for complex control)
 * ========================================================================= */

/**
 * @brief [TOPIC 0x01] Handle autonomous mode toggle.
 *
 * Translates the binary flag into a Supervisor FSM event so the state machine
 * controls the actual mode transition (instead of writing state directly).
 *
 * @param msg Pointer to the parsed AutonomousMsg payload.
 */
static void handle_autonomous(const AutonomousMsg_t *msg)
{
    RobotState_SetAutonomous(msg->is_autonomous);

    if (msg->is_autonomous) {
        Supervisor_ProcessEvent(EVENT_MODE_AUTO, SRC_UART3_ROS);
        LOG_INFO(LOG_TAG, "Autonomous ON");
    } else {
        Supervisor_ProcessEvent(EVENT_MODE_MANUAL, SRC_UART3_ROS);
        LOG_INFO(LOG_TAG, "Autonomous OFF -> Manual");
    }
}

/**
 * @brief [TOPIC 0x02] Handle mobility mode + autonomous flag config packet.
 *
 * Mobility mode selection is forwarded to RobotState. The is_autonomous flag
 * mirrors the 0x01 topic logic for convenience when both are sent together.
 *
 * @param msg Pointer to the parsed SysConfigMsg_t payload.
 */
static void handle_mobility_mode(const SysConfigMsg_t *msg)
{
    RobotState_SetTargetMobilityMode(msg->mobility_mode);
    RobotState_SetAutonomous(msg->is_autonomous);
    LOG_INFO(LOG_TAG, "Config: Mode=%u, Auto=%u", msg->mobility_mode, msg->is_autonomous);
}

/**
 * @brief [TOPIC 0x03] Handle cmd_vel setpoints.
 *
 * Writes the velocity command into the shared Commands block.
 * The MobilityTask reads this on its control cycle.
 *
 * @param msg Pointer to the parsed CmdVelMsg_t payload.
 */
static void handle_cmd_vel(const CmdVelMsg_t *msg)
{
    RobotState_SetTargetVelocity(msg->linear_x, msg->angular_z);
    LOG_DEBUG(LOG_TAG, "CmdVel: x=%.3f z=%.3f", msg->linear_x, msg->angular_z);
}

/**
 * @brief [TOPIC 0x04] Handle arm joint goal.
 *
 * Writes the target joint positions into the shared Commands block.
 * The ArmTask reads this on its control cycle.
 *
 * @param msg Pointer to the parsed ArmGoalMsg_t payload.
 *
 * @todo Validate joint limits before writing.
 * @todo Implement trajectory interpolation in ArmTask.
 */
static void handle_arm_goal(const ArmGoalMsg_t *msg)
{
    RobotState_SetTargetArmPose(msg->j1, msg->j2, msg->j3);
    LOG_DEBUG(LOG_TAG, "ArmGoal: j1=%.2f j2=%.2f j3=%.2f", msg->j1, msg->j2, msg->j3);
}

/**
 * @brief [TOPIC 0x05] Handle system events from ROS.
 *
 * Maps the protocol event_id to the Supervisor FSM event enum and
 * dispatches it with SRC_ROS as the originating source.
 *
 * @param msg Pointer to the parsed SysEventMsg_t payload.
 */
static void handle_sys_event(const SysEventMsg_t *msg)
{
    SystemEvent_t event;
    bool valid = true;

    switch (msg->event_id) {
        case SYS_EVENT_START:   event = EVENT_START;       break;
        case SYS_EVENT_STOP:    event = EVENT_STOP;        break;
        case SYS_EVENT_PAUSE:   event = EVENT_PAUSE;       break;
        case SYS_EVENT_RESUME:  event = EVENT_RESUME;      break;
        case SYS_EVENT_RESET:   event = EVENT_RESET;       break;
        default:
            LOG_WARNING(LOG_TAG, "Unknown sys_event id=0x%02X", msg->event_id);
            valid = false;
            break;
    }

    if (valid) {
        LOG_INFO(LOG_TAG, "SysEvent 0x%02X -> FSM event %d", msg->event_id, event);
        Supervisor_ProcessEvent(event, SRC_UART3_ROS);
    }
}

/* =========================================================================
 * Public API
 * ========================================================================= */

void SerialRos_Init(void) {
    last_rx_tick = 0;
    LOG_INFO(LOG_TAG, "SerialRos Module Initialized");
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
    /* 64 byte packet: 4 header + 58 max payload + 2 CRC */
    if (len > 58) return false;

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

bool SerialRos_DequeueRx(SerialRos_Packet_t *out_packet, uint32_t timeout_ms)
{
    osal_status_t status = osal_queue_get(rosRxQueueHandle, out_packet, timeout_ms);
    return (status == OSAL_OK);
}
