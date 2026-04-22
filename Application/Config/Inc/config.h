/**
 * DEFAULT STATIC PARAMETERS
 * These values are used when PESISTENT_CONFIG is disabled
 * and as the initial values when the memory is first initialized.
 */
/* Debug & System */
#define DEFAULT_APP_DEBUG_LEVEL                 LOG_LEVEL_INFO
#define DEFAULT_TELEMETRY_BASE_PERIOD_MS        10
#define DEFAULT_SYSTEM_VARS_PERIOD_MS           500
#define DEFAULT_IMU_PUBLISH_PERIOD_MS           10
#define DEFAULT_ODOM_PUBLISH_PERIOD_MS          20
/* Motor Parameters */
#define DEFAULT_ROBOT_STATE_PID_ENABLED         0
#define DEFAULT_MOTOR_TICKS_PER_CIRCLE          654.0f
#define DEFAULT_MOTOR_SPEED_LIMIT               1.26f
#define DEFAULT_ROBOT_MAX_ANGULAR_SPEED         2.50f
#define DEFAULT_MOTOR_PULSE_DEADZONE            23000.0f
#define DEFAULT_MOTOR_PWM_MAX                   65535.0f

/* Default Motor PID Gains */
#define DEFAULT_MOTOR_KP                        47489.0f
#define DEFAULT_MOTOR_KI                        100000.0f
#define DEFAULT_MOTOR_KD                        50.0f
/* Chassis Parameters */
#define DEFAULT_ROBOT_WHEEL_DIAMETER            0.08f
#define DEFAULT_ROBOT_SHAFT_WIDTH               0.170f  // Distance between left and right wheels
#define DEFAULT_ROBOT_WHEELBASE_LENGTH          0.200f  // Distance between front and rear wheels
#define DEFAULT_ROBOT_MOBILITY_MODE             0       // 0: Direct, 1: Differential, 2: Ackermann, 3: Mecanum
/* Motor Directions */
#define DEFAULT_MOTOR1_INVERT                   1
#define DEFAULT_MOTOR2_INVERT                   1
#define DEFAULT_MOTOR3_INVERT                   1
#define DEFAULT_MOTOR4_INVERT                   1

/**
 * MEMORY CONFIGURATION TOGGLE
 * Enable PESISTENT_CONFIG in the build system or here to load parameters from Flash (Persistent Memory).
 * If disabled, the system initializes with the DEFAULT_ macros in RAM and never saves them to Flash.
 */
#define PESISTENT_CONFIG


