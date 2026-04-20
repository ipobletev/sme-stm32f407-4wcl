#ifndef APP_CONFIG_H
#define APP_CONFIG_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Application Configuration Structure
 * This struct is stored in Flash memory and can be modified at runtime.
 * Ensure it is 4-byte aligned for Flash programming.
 */
typedef struct {
    uint32_t magic;                 /* Magic number to verify flash data (0xABCD1234) */
    
    /* Debug & System */
    uint32_t debug_level;
    uint32_t telemetry_period_ms;
    uint32_t sys_vars_period_ms;
    uint32_t imu_publish_period_ms;
    uint32_t odom_publish_period_ms;
    
    /* Motor Parameters */
    uint32_t pid_enabled;
    float    motor_ticks_per_circle;
    float    motor_rps_limit;
    float    motor_pulse_deadzone;
    float    motor_pwm_max;
    
    /* Chassis Parameters */
    float    wheel_diameter;
    float    shaft_width;
    float    wheelbase_length;
    
    /* Motor Directions */
    int32_t  motor1_invert;
    int32_t  motor2_invert;
    int32_t  motor3_invert;
    int32_t  motor4_invert;
    
    uint32_t crc;                   /* CRC32 or simple checksum for integrity */
} AppConfig_t;

/**
 * @brief Global pointer to the current live configuration.
 * Read-only access for modules.
 */
extern const AppConfig_t * const AppConfig;

typedef enum {
    /* System (0x01-0x0F) */
    CONF_DEBUG_LEVEL            = 0x01,
    CONF_TELEMETRY_PERIOD       = 0x02,
    CONF_SYS_VARS_PERIOD        = 0x04,
    CONF_IMU_PERIOD             = 0x05,
    CONF_ODOM_PERIOD            = 0x06,

    /* Motor (0x10-0x1F) */
    CONF_PID_ENABLED            = 0x10,
    CONF_MOTOR_TICKS            = 0x11,
    CONF_MOTOR_RPS_LIMIT        = 0x12,
    CONF_MOTOR_DEADZONE         = 0x13,
    CONF_MOTOR_PWM_MAX          = 0x14,

    /* Chassis (0x20-0x2F) */
    CONF_WHEEL_DIAMETER         = 0x20,
    CONF_SHAFT_WIDTH            = 0x21,
    CONF_WHEELBASE              = 0x22,

    /* Directions (0x30-0x3F) */
    CONF_MOTOR1_INV             = 0x31,
    CONF_MOTOR2_INV             = 0x32,
    CONF_MOTOR3_INV             = 0x33,
    CONF_MOTOR4_INV             = 0x34,
} AppConfigParamId_t;

/**
 * @brief Update a specific parameter in RAM by ID.
 * @return true if ID was valid and value updated.
 */
bool AppConfig_UpdateParam(uint8_t id, float value);

/**
 * @brief Initialize the configuration system.
 * Loads from flash if PESISTENT_CONFIG is set.
 */
void AppConfig_Init(void);

/**
 * @brief Get pointer to the current configuration
 */
AppConfig_t* AppConfig_Get(void);

/**
 * @brief Save the current configuration structure to Flash
 */
bool AppConfig_Save(void);

/**
 * @brief Register a callback to be notified when the configuration changes.
 */
void AppConfig_RegisterCallback(void (*callback)(void));

/**
 * @brief Manually trigger a notification to all registered listeners.
 */
void AppConfig_NotifyChange(void);

/**
 * @brief Reload configuration from Flash into RAM.
 * Useful for reverting unsaved changes.
 * @return true if reload was successful.
 */
bool AppConfig_ReloadFromFlash(void);

#endif /* APP_CONFIG_H */
