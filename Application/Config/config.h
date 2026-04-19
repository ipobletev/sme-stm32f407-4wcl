/* 
 * DEBUG LOG CONFIG 
 */
#define APP_DEBUG_LEVEL                 LOG_LEVEL_INFO  // Set global log level (NONE, ERROR, WARN, INFO, DEBUG)

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
#define ROBOT_STATE_DEFAULT_PID_ENABLED 0
#define MOTOR_TICKS_PER_CIRCLE          3960.0f
#define MOTOR_RPS_LIMIT                 1.0f
#define MOTOR_PULSE_DEADZONE            1000.0f
// #define MOTOR_PID_KP                    63.0f
// #define MOTOR_PID_KI                    2.6f
// #define MOTOR_PID_KD                    2.4f
// #define MOTOR_RPS_LIMIT                 1.5f

/**
 * Chassis Physical Parameters (4WCL Mecanum)
 */
#define ROBOT_WHEEL_DIAMETER   0.08f    /* mm */
#define ROBOT_SHAFT_WIDTH      0.170f   /* Distance from the center of the wheel and the center of the other wheel */
#define ROBOT_WHEELBASE_LENGTH 0.150f   /* Distance between the center of the front wheels and the center of the rear wheels */

/**
 * PWM Configuration
 */
#define MOTOR_PWM_MAX           65535.0f

/**
 * Motor Direction Inversion (1 for Normal, -1 for Inverted)
 * Use this to easily flip a motor that is rotating in the wrong direction.
 */
#define MOTOR1_INVERT           1
#define MOTOR2_INVERT           1
#define MOTOR3_INVERT           1
#define MOTOR4_INVERT           1