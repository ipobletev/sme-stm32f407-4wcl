/* 
 * TIME CONFIG 
 */
#define TELEMETRY_BASE_PERIOD_MS  10        // 100Hz Base Loop
#define HEARTBEAT_PERIOD_MS       1000      // Used in HeartbeatTimerCallback()
#define SYSTEM_VARS_PERIOD_MS     500       // Frequency to sample battery and sensors
#define IMU_PUBLISH_PERIOD_MS     10        // 100Hz
#define ODOM_PUBLISH_PERIOD_MS    100       // 10Hz

/**
 * Common Motor Parameters (JGB520)
 */
#define MOTOR_TICKS_PER_CIRCLE 3960.0f
#define MOTOR_PID_KP           63.0f
#define MOTOR_PID_KI           2.6f
#define MOTOR_PID_KD           2.4f
#define MOTOR_RPS_LIMIT        1.5f

/**
 * Chassis Physical Parameters (4WCL Mecanum)
 */
#define ROBOT_WHEEL_DIAMETER   0.08f    /* 80mm */
#define ROBOT_SHAFT_WIDTH      0.170f   /* Distance from the center of the wheel and the center of the other wheel */
#define ROBOT_WHEELBASE_LENGTH 0.150f   /* Distance between the center of the front wheels and the center of the rear wheels */

/**
 * PWM Configuration
 */
#define MOTOR_PWM_MAX           65535.0f


// #define TIMEOUT_LAST_CMD_MS 250     // TIMEOUT CONFIG FOR CMD VEL. If no new command is received for this time, the motors will stop
// #define TIME_WATCHDOG_MS 1000       // 1s refresh rate

// // ROS PUBLISH TIME CONFIG
// #define TIME_ENCODER_PUBLISH_MS         100              //10Hz    
// #define TIME_IMU_PUBLISH_MS             50               //20Hz
// #define TIME_MACHINE_INFO_PUBLISH_MS    250              //4Hz
// #define TIME_PID_DEBUG_PUBLISH_MS       50               //20Hz
// // STORAGE CONFIG
// #define FLASH_STORAGE_ADDR              0x0803F000       // STM32F103RCTx has 256KB Flash. Page size is 2KB. Last page starts at 0x0803F800. Reserved 2 pages from 0x0803F000
// #define VIRTUAL_FLASH_PAGE_SIZE         2048

// //IMU CONFIG
// #define IMU_USE_DEBUG true
// #define TIME_IMU_UPDATE_MS 20                   //50Hz (Internal update rate)
// #define IMU_INIT_RETRIES 5

// // MOTOR CONFIG
// #define MOTOR_ID_1 0
// #define MOTOR_ID_2 1
// #define MOTOR_ID_3 2
// #define MOTOR_ID_4 3

// // MOTOR PID CONFIG
// #define MOTOR1_USE_PID true
// #define MOTOR1_PID_KP 1.5f
// #define MOTOR1_PID_KI 0.5f
// #define MOTOR1_PID_KD 0.0f
// #define MOTOR1_PID_MAX_OUTPUT 3600.0f
// #define MOTOR1_PID_MIN_OUTPUT -3600.0f

// #define MOTOR2_USE_PID true
// #define MOTOR2_PID_KP 1.5f
// #define MOTOR2_PID_KI 0.5f
// #define MOTOR2_PID_KD 0.0f
// #define MOTOR2_PID_MAX_OUTPUT 3600.0f
// #define MOTOR2_PID_MIN_OUTPUT -3600.0f

// #define MOTOR3_USE_PID true
// #define MOTOR3_PID_KP 1.5f
// #define MOTOR3_PID_KI 0.5f
// #define MOTOR3_PID_KD 0.0f
// #define MOTOR3_PID_MAX_OUTPUT 3600.0f
// #define MOTOR3_PID_MIN_OUTPUT -3600.0f

// #define MOTOR4_USE_PID true
// #define MOTOR4_PID_KP 1.5f
// #define MOTOR4_PID_KI 0.5f
// #define MOTOR4_PID_KD 0.0f
// #define MOTOR4_PID_MAX_OUTPUT 3600.0f
// #define MOTOR4_PID_MIN_OUTPUT -3600.0f

// // SERIAL ROS CONFIG
// #define SERIAL_ROS_HEADER_1 0xAA
// #define SERIAL_ROS_HEADER_2 0x55
// #define SERIAL_ROS_MAX_PAYLOAD 128

// // SERIAL ROS TOPIC ID
// // Published topics
// #define TOPIC_MACHINE_INFO      0x01
// #define TOPIC_IMU               0x02
// #define TOPIC_ENCODER           0x03
// #define TOPIC_PID_DEBUG         0x09
// // Subscribed topics
// #define TOPIC_CMD_VEL           0x04
// #define TOPIC_OPERATION_MODE    0x05
// #define TOPIC_OPERATION_RUN     0x06
// #define TOPIC_RESET_STOP_CMD    0x07
// #define TOPIC_ESTOP_CMD         0x08
// #define TOPIC_CONFIRM_CONN      0x0A
// #define TOPIC_CONFIG            0x0B

// // CONFIG ITEM IDS
// #define CONFIG_ITEM_PID_M1      0x00
// #define CONFIG_ITEM_PID_M2      0x01
// #define CONFIG_ITEM_PID_M3      0x02
// #define CONFIG_ITEM_PID_M4      0x03
// #define CONFIG_ITEM_WHEEL_DIAM  0x04

// // CONNECTION CONFIG
// #define CONN_TIMEOUT_MS             3000

// // MOTION CONFIG
// #define MOTION_FREQ_MS 50
// #define ENCODER_THRESHOLD 1
// #define GYRO_THRESHOLD 0.05f
// #define ACCEL_THRESHOLD 0.1f

// // ROBOT PHYSICAL PARAMS
// #define WHEEL_DIAMETER 0.080f   // 80mm
// #define ENCODER_PPR 1320.0f      // 11 ticks * 30 gear ratio * 4 quadrature ?? TODO: Verify

// // ERROR CONFIG
// #define APP_USE_HARDWARE_ERROR // If is enabled, the system will stop (EMERGENCY STOP) if a hardware error is detected (global_system_error != 0)

// // DEBUG CONFIG
// #define APP_DEBUG_LEVEL                 DEBUG_LEVEL_DEBUG  // Set global log level (NONE, ERROR, WARN, INFO, DEBUG)
// #define APP_DEBUG_USE_COLORS            // Enable ANSI color codes
// // LOG DEBUG FOR PUBLISH
// #define TIME_LOG_DEBUG_ENCODER_MS 1000
// #define TIME_LOG_DEBUG_IMU_MS 1000
// #define TIME_LOG_DEBUG_STATE_MS 1000

// // #define APP_DISABLE_SERIAL_ROS_PUBLISHERS // DISABLE SERIAL ROS PUBLISHERS ONLY FOR DEVELOPMENT!!!!!!!