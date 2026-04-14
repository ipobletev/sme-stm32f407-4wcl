#ifndef __SERIAL_ROS_PROTOCOL_H
#define __SERIAL_ROS_PROTOCOL_H

#include <stdint.h>

/* --- Protocol Constants --- */
#define SERIAL_ROS_SYNC1        0xAA
#define SERIAL_ROS_SYNC2        0x55
#define SERIAL_ROS_MIN_SIZE     6  /* SYNC1 + SYNC2 + ID + LEN + CRC_H + CRC_L */

/* Rx (Virtual Subscribed) Topics */
#define TOPIC_ID_AUTONOMOUS     0x01    /* Rx: System Autonomous/Manual Mode */
#define TOPIC_ID_MOBILITY_MODE  0x02    /* Rx: Mobility Mode */
#define TOPIC_ID_CMD_VEL        0x03    /* Rx: Movement setpoints */
#define TOPIC_ID_ARM_GOAL       0x04    /* Rx: Robotic arm joint targets */
#define TOPIC_ID_SYS_EVENT      0x05    /* Rx: Logic events (START, RESET, STOP, etc) */

/* Tx (Virtual Published) Topics */
#define TOPIC_ID_SYS_STATUS     0x81    /* Tx: System state, health, and battery */
#define TOPIC_ID_IMU            0x82    /* Tx: IMU data */
#define TOPIC_ID_ODOMETRY       0x83    /* Tx: Odometry data */

/* --- Message Structures (Packed) --- */
#pragma pack(push, 1)

/**
 * @brief Message: cmd_vel [Topic 0x01]
 */
typedef struct {
    float linear_x;
    float angular_z;
} CmdVelMsg_t;

/**
 * @brief Message: arm_goal [Topic 0x02]
 */
typedef struct {
    float j1;
    float j2;
    float j3;
} ArmGoalMsg_t;

/**
 * @brief Message: sys_event [Topic 0x05]
 */
typedef struct {
    uint8_t event_id;   /* Maps to SystemEvent_t */
} SysEventMsg_t;

/**
 * @brief Message: sys_config [Topic 0x06]
 */
typedef struct {
    uint8_t mobility_mode;  /* 0: Direct, 1: DiffDrive, 2: Ackermann, 3: Mecanum */
    uint8_t is_autonomous;  /* 0: Manual, 1: Auto */
} SysConfigMsg_t;

/**
 * @brief Message: system_status [Topic 0x81]
 * Consolidated telemetry message.
 */
typedef struct {
    uint8_t current_state;
    uint64_t error_flags;
    float mcu_temp;
    float battery_voltage;
    float battery_current;
} SystemStatusMsg_t;

#pragma pack(pop)

/* --- Common Utility: CRC16 --- */
static inline uint16_t SerialRos_CRC16(const uint8_t *data, uint16_t len) {
    uint16_t crc = 0xFFFF;
    for (uint16_t i = 0; i < len; i++) {
        crc ^= (uint16_t)data[i];
        for (int j = 8; j != 0; j--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

/**
 * @brief Container for SerialRos packets in RTOS queues
 */
typedef struct {
    uint8_t data[64];
    uint8_t size;
} SerialRos_Packet_t;

#endif /* __SERIAL_ROS_PROTOCOL_H */
