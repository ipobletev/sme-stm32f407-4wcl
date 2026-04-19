#ifndef __SERIAL_ROS_PROTOCOL_H
#define __SERIAL_ROS_PROTOCOL_H

#include <stdint.h>

/* --- Protocol Constants --- */
#define SERIAL_ROS_SYNC1        0xAA
#define SERIAL_ROS_SYNC2        0x55
#define SERIAL_ROS_MIN_SIZE     6  /* SYNC1 + SYNC2 + ID + LEN + CRC_H + CRC_L */

/* Rx (Virtual Subscribed) Topics */
#define TOPIC_ID_HEARTBEAT      0x00    /* Rx: Heartbeat from Client */
#define TOPIC_ID_AUTONOMOUS     0x01    /* Rx: System Autonomous/Manual Mode */
#define TOPIC_ID_MOBILITY_MODE  0x02    /* Rx: Mobility Mode */
#define TOPIC_ID_CMD_VEL        0x03    /* Rx: Movement setpoints */
#define TOPIC_ID_ARM_GOAL       0x04    /* Rx: Robotic arm joint targets */
#define TOPIC_ID_SYS_EVENT      0x05    /* Rx: Logic events (START, RESET, STOP, etc) */
#define TOPIC_ID_ACTUATOR_PWM   0x06    /* Rx: Raw actuator PWM control (ID + Pulse) */
#define TOPIC_ID_ACTUATOR_VEL   0x07    /* Rx: Raw actuator Velocity control (ID + RPS) */

/* Tx (Virtual Published) Topics */
#define TOPIC_ID_SYS_STATUS     0x81    /* Tx: System state, health, and battery */
#define TOPIC_ID_IMU            0x82    /* Tx: IMU data */
#define TOPIC_ID_ODOMETRY       0x83    /* Tx: Odometry data */

/* --- sys_event payload IDs (maps event_id field to Supervisor FSM events) --- */
typedef enum {
    SYS_EVENT_START  = 0x01,
    SYS_EVENT_STOP   = 0x02,
    SYS_EVENT_PAUSE  = 0x03,
    SYS_EVENT_RESUME = 0x04,
    SYS_EVENT_RESET  = 0x05,
    SYS_EVENT_FAULT  = 0x06,
    SYS_EVENT_TEST   = 0x07,
} SysEventId_t;

/* --- Message Structures (Packed) --- */
#pragma pack(push, 1)

/**
 * @brief Message: autonomous [Topic 0x01]
 * Requests a switch between Manual and Autonomous operation mode.
 */
typedef struct {
    uint8_t is_autonomous;  /* 0: Manual, 1: Autonomous */
} AutonomousMsg_t;

/**
 * @brief Message: cmd_vel [Topic 0x03]
 */
typedef struct {
    float linear_x;
    float angular_z;
} CmdVelMsg_t;

/**
 * @brief Message: arm_goal [Topic 0x04]
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
 * @brief Message: actuator_pwm [Topic 0x06]
 * Used for testing/debugging individual motors.
 */
typedef struct {
    uint8_t  actuator_id; /* 0-3 for motors */
    float    pulse;       /* Raw PWM value (-65535 to 65535) */
} ActuatorTestMsg_t;

/**
 * @brief Message: mobility_mode [Topic 0x02]
 * Sets the kinematic model and optionally the autonomous flag.
 */
typedef struct {
    uint8_t mobility_mode;  /* 0: Direct, 1: DiffDrive, 2: Ackermann, 3: Mecanum */
    uint8_t is_autonomous;  /* 0: Manual, 1: Auto */
} SysConfigMsg_t;

/**
 * @brief Message: ImuMsg [Topic 0x82]
 */
typedef struct {
    float roll, pitch, yaw;
    float gyro_x, gyro_y, gyro_z;
    float accel_x, accel_y, accel_z;
} ImuMsg_t;

/**
 * @brief Message: OdometryMsg [Topic 0x83]
 */
typedef struct {
    float linear_x;
    float angular_z;
    int32_t enc_1;
    int32_t enc_2;
    int32_t enc_3;
    int32_t enc_4;
} OdometryMsg_t;

/**
 * @brief Message: system_status [Topic 0x81]
 * Consolidated telemetry message.
 */
typedef struct {
    uint64_t error_flags;       /* 8-byte errors (at offset 0) */
    float mcu_temp;             /* 4-byte float (at offset 8) */
    float battery_voltage;      /* 4-byte float (at offset 12) */
    uint8_t current_state;      /* 1-byte state (at offset 16) */
    uint8_t mobility_state;     /* 1-byte mobility state (at offset 17) */
    uint8_t arm_state;          /* 1-byte arm state (at offset 18) */
    uint8_t mobility_mode;      /* 1-byte mobility mode (at offset 19) */
    /* float battery_current; */ /* Unsupported by hardware */
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
