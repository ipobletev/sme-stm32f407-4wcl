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
#include "app_config.h"

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
    uint8_t current_auto = RobotState_IsAutonomous();

    if (msg->is_autonomous != current_auto) 
    {
        RobotState_SetAutonomous(msg->is_autonomous);

        if (msg->is_autonomous) {
            Supervisor_ProcessEvent(EVENT_SUPERVISOR_MODE_AUTO, SRC_EXT_CLIENT);
            LOG_INFO(LOG_TAG, "Autonomous ON (Direct Handover)\r\n");
        } else {
            Supervisor_ProcessEvent(EVENT_SUPERVISOR_MODE_MANUAL, SRC_EXT_CLIENT);
            LOG_INFO(LOG_TAG, "Autonomous OFF -> Manual Takeover\r\n");
        }
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
    MobilityMode_t current_mode = RobotState_GetTargetMobilityMode();
    uint8_t current_auto = RobotState_IsAutonomous();

    if ((MobilityMode_t)msg->mobility_mode != current_mode || msg->is_autonomous != current_auto)
    {
        RobotState_SetTargetMobilityMode((MobilityMode_t)msg->mobility_mode);
        RobotState_SetAutonomous(msg->is_autonomous);
        LOG_INFO(LOG_TAG, "Config Change: Mode=%u, Auto=%u\r\n", msg->mobility_mode, msg->is_autonomous);
    }
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
    SystemState_t current_sup = Supervisor_GetCurrentState();
    if (current_sup == STATE_SUPERVISOR_AUTO || current_sup == STATE_SUPERVISOR_MANUAL) {
        RobotState_SetTargetVelocity(msg->linear_x, msg->angular_z);
        LOG_INFO(LOG_TAG, "CmdVel ACCEPTED: x=%.3f z=%.3f\r\n", msg->linear_x, msg->angular_z);
    } else {
        /* Ignore commands if not in Auto or Manual mode */
        LOG_WARNING(LOG_TAG, "CmdVel REJECTED: SUP is %d (Need AUTO or MANUAL)\r\n", current_sup);
    }
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
    SystemState_t current_sup = Supervisor_GetCurrentState();
    if (current_sup == STATE_SUPERVISOR_AUTO || current_sup == STATE_SUPERVISOR_MANUAL) {
        RobotState_SetTargetArmPose(msg->j1, msg->j2, msg->j3);
        LOG_INFO(LOG_TAG, "ArmGoal ACCEPTED: j1=%.2f j2=%.2f j3=%.2f\r\n", msg->j1, msg->j2, msg->j3);
    } else {
        /* Ignore commands if not in Auto or Manual mode */
        LOG_WARNING(LOG_TAG, "ArmGoal REJECTED: SUP is %d (Need AUTO or MANUAL)\r\n", current_sup);
    }
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
    bool event_to_process = true;

    switch (msg->event_id) {
        case SYS_EVENT_START:   event = EVENT_SUPERVISOR_START;       break;
        case SYS_EVENT_STOP:    event = EVENT_SUPERVISOR_STOP;        break;
        case SYS_EVENT_PAUSE:   event = EVENT_SUPERVISOR_PAUSE;       break;
        case SYS_EVENT_RESUME:  event = EVENT_SUPERVISOR_RESUME;      break;
        case SYS_EVENT_RESET:   event = EVENT_SUPERVISOR_RESET;       break;
        case SYS_EVENT_FAULT:   event = EVENT_SUPERVISOR_ERROR;       break;
        case SYS_EVENT_TEST:    event = EVENT_SUPERVISOR_TESTING;     break;
        default:
            LOG_WARNING(LOG_TAG, "Unknown sys_event id=0x%02X\r\n", msg->event_id);
            event_to_process=false;
            break;
    }

    if (event_to_process) {
        Supervisor_ProcessEvent(event, SRC_EXT_CLIENT);
    }
}

/**
 * @brief [TOPIC 0x08] Handle remote configuration set.
 *
 * @param msg Pointer to the parsed SetConfigMsg_t payload.
 */
static void handle_set_config(const SetConfigMsg_t *msg)
{
    AppConfig_UpdateParam(msg->id, msg->value);
    
    /* Broadcast back the full current config to keep all clients in sync (RAM state) */
    AppConfig_t* config = AppConfig_Get();
    SerialRos_EnqueueTx(TOPIC_ID_APP_CONFIG_DATA, config, sizeof(AppConfig_t));
}

/**
 * @brief [TOPIC 0x09] Handle remote configuration get request.
 *
 * Triggers a full dump of the current AppConfig_t to the client.
 */
static void handle_get_config(void)
{
    /* REFRESH logic: Load from Flash to RAM before sending so the client gets 
     * the persisted values (Discarding unsaved RAM changes). */
#ifdef PESISTENT_CONFIG
    AppConfig_ReloadFromFlash();
#endif
    AppConfig_t* config = AppConfig_Get();
    /* We send the raw struct since it is packed and contains all active fields after magic */
    if (SerialRos_EnqueueTx(TOPIC_ID_APP_CONFIG_DATA, config, sizeof(AppConfig_t))) {
        LOG_INFO(LOG_TAG, "Config Dump Sent to Client (%d bytes)\r\n", (int)sizeof(AppConfig_t));
    } else {
        LOG_ERROR(LOG_TAG, "Failed to Enqueue Config Dump (%d bytes). Buffer too small?\r\n", (int)sizeof(AppConfig_t));
    }
}

/**
 * @brief [TOPIC 0x06] Handle raw actuator pulse for debugging.
 *
 * @param msg Pointer to the parsed ActuatorTestMsg_t payload.
 */
static void handle_actuator_test(const ActuatorTestMsg_t *msg)
{
    if (msg->actuator_id == 0xFF) {
        /* Broadcast to all 4 motors */
        for (uint8_t i = 0; i < 4; i++) {
            RobotState_SetMotorTestCommand(i, msg->pulse, 0);
        }
    } else if (msg->actuator_id < 4) {
        /* Single motor control */
        RobotState_SetMotorTestCommand(msg->actuator_id, msg->pulse, 0); /* 0 = PWM */
    }
}

/**
 * @brief [TOPIC 0x07] Handle raw actuator velocity for debugging.
 *
 * @param msg Pointer to the parsed ActuatorTestMsg_t payload.
 */
static void handle_actuator_velocity(const ActuatorTestMsg_t *msg)
{
    if (msg->actuator_id == 0xFF) {
        /* Broadcast to all 4 motors */
        for (uint8_t i = 0; i < 4; i++) {
            RobotState_SetMotorTestCommand(i, msg->pulse, 1);
        }
    } else if (msg->actuator_id < 4) {
        /* Single motor control */
        RobotState_SetMotorTestCommand(msg->actuator_id, msg->pulse, 1); /* 1 = Velocity */
    }
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
