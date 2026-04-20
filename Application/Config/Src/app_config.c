#include "app_config.h"
#include "config.h"
#include "bsp_internal_flash.h"
#include <string.h>
#include <stdio.h>
#include "debug_module.h"
#include "mobility_fsm.h"

#define CONFIG_MAGIC_NUMBER     0xABCD1235
#define LOG_TAG                 "APP_CONFIG"
#define MAX_CONFIG_CALLBACKS    8

static AppConfig_t g_current_config;
static void (*g_config_callbacks[MAX_CONFIG_CALLBACKS])(void);
static uint8_t g_callback_count = 0;

/* Global config accessor */
const AppConfig_t * const AppConfig = &g_current_config;

/**
 * @brief Simple checksum calculation
 */
static uint32_t AppConfig_CalculateChecksum(AppConfig_t* config) {
    uint32_t checksum = 0;
    uint32_t* ptr = (uint32_t*)config;
    /* Sum all fields except the last one (the crc itself) */
    for (size_t i = 0; i < (sizeof(AppConfig_t) / 4) - 1; i++) {
        checksum += ptr[i];
    }
    return checksum;
}

/**
 * @brief Check if flash data is valid
 */
static bool AppConfig_IsValid(AppConfig_t* config) {
    if (config->magic != CONFIG_MAGIC_NUMBER) return false;
    if (config->crc != AppConfig_CalculateChecksum(config)) return false;
    return true;
}

AppConfig_t* AppConfig_Get(void) {
    return &g_current_config;
}

void AppConfig_ResetToDefaults(void) {
    memset(&g_current_config, 0, sizeof(AppConfig_t));
    
    g_current_config.magic = CONFIG_MAGIC_NUMBER;
    
    /* System */
    g_current_config.debug_level           = DEFAULT_APP_DEBUG_LEVEL;
    g_current_config.telemetry_period_ms   = DEFAULT_TELEMETRY_BASE_PERIOD_MS;
    g_current_config.sys_vars_period_ms     = DEFAULT_SYSTEM_VARS_PERIOD_MS;
    g_current_config.imu_publish_period_ms = DEFAULT_IMU_PUBLISH_PERIOD_MS;
    g_current_config.odom_publish_period_ms = DEFAULT_ODOM_PUBLISH_PERIOD_MS;
    
    /* Motor Control */
    g_current_config.pid_enabled            = DEFAULT_ROBOT_STATE_PID_ENABLED;
    g_current_config.motor_ticks_per_circle = DEFAULT_MOTOR_TICKS_PER_CIRCLE;
    g_current_config.motor_rps_limit        = DEFAULT_MOTOR_RPS_LIMIT;
    g_current_config.motor_pwm_max          = DEFAULT_MOTOR_PWM_MAX;
    
    /* Physics */
    g_current_config.wheel_diameter   = DEFAULT_ROBOT_WHEEL_DIAMETER;
    g_current_config.shaft_width      = DEFAULT_ROBOT_SHAFT_WIDTH;
    g_current_config.wheelbase_length = DEFAULT_ROBOT_WHEELBASE_LENGTH;
    g_current_config.mobility_mode     = DEFAULT_ROBOT_MOBILITY_MODE;
    
    /* Inversion */
    g_current_config.motor1_invert = DEFAULT_MOTOR1_INVERT;
    g_current_config.motor2_invert = DEFAULT_MOTOR2_INVERT;
    g_current_config.motor3_invert = DEFAULT_MOTOR3_INVERT;
    g_current_config.motor4_invert = DEFAULT_MOTOR4_INVERT;

    /* Per-Motor Calibration Defaults */
    g_current_config.motor1_kp = DEFAULT_MOTOR_KP; g_current_config.motor1_ki = DEFAULT_MOTOR_KI; g_current_config.motor1_kd = DEFAULT_MOTOR_KD; g_current_config.motor1_deadzone = DEFAULT_MOTOR_PULSE_DEADZONE;
    g_current_config.motor2_kp = DEFAULT_MOTOR_KP; g_current_config.motor2_ki = DEFAULT_MOTOR_KI; g_current_config.motor2_kd = DEFAULT_MOTOR_KD; g_current_config.motor2_deadzone = DEFAULT_MOTOR_PULSE_DEADZONE;
    g_current_config.motor3_kp = DEFAULT_MOTOR_KP; g_current_config.motor3_ki = DEFAULT_MOTOR_KI; g_current_config.motor3_kd = DEFAULT_MOTOR_KD; g_current_config.motor3_deadzone = DEFAULT_MOTOR_PULSE_DEADZONE;
    g_current_config.motor4_kp = DEFAULT_MOTOR_KP; g_current_config.motor4_ki = DEFAULT_MOTOR_KI; g_current_config.motor4_kd = DEFAULT_MOTOR_KD; g_current_config.motor4_deadzone = DEFAULT_MOTOR_PULSE_DEADZONE;
    
    g_current_config.crc = AppConfig_CalculateChecksum(&g_current_config);
    
    LOG_INFO(LOG_TAG, "Reseting to defaults from config.h\r\n");
    AppConfig_Save();
    AppConfig_NotifyChange();
}

bool AppConfig_Save(void) {
    /* Update Checksum before saving */
    g_current_config.crc = AppConfig_CalculateChecksum(&g_current_config);
    
#ifdef PESISTENT_CONFIG
    /* 1. Erase Sector */
    if (!BSP_InternalFlash_EraseSector(FLASH_SECTOR_CONFIG)) {
        LOG_ERROR(LOG_TAG, "Erasing Flash Sector failed!\r\n");
        return false;
    }
    
    /* 2. Program Flash */
    if (!BSP_InternalFlash_Write(FLASH_ADDRESS_CONFIG, (uint32_t*)&g_current_config, sizeof(AppConfig_t))) {
        LOG_ERROR(LOG_TAG, "Writing to Flash failed!\r\n");
        return false;
    }
    
    LOG_INFO(LOG_TAG, "Configuration saved to Flash Sector %d\r\n", FLASH_SECTOR_CONFIG);
#else
    LOG_INFO(LOG_TAG, "Static mode: Save to Flash skipped (not enabled in config.h)\r\n");
#endif

    AppConfig_NotifyChange();
    return true;
}

void AppConfig_RegisterCallback(void (*callback)(void)) {
    if (g_callback_count < MAX_CONFIG_CALLBACKS) {
        g_config_callbacks[g_callback_count++] = callback;
    }
}

void AppConfig_NotifyChange(void) {
    LOG_INFO(LOG_TAG, "Notifying %d listeners of config change...\r\n", g_callback_count);
    for (uint8_t i = 0; i < g_callback_count; i++) {
        if (g_config_callbacks[i]) {
            g_config_callbacks[i]();
        }
    }
}

bool AppConfig_UpdateParam(uint8_t id, float value) {
    LOG_INFO(LOG_TAG, "Update Request: ID 0x%02X -> %.3f\r\n", id, value);
    
    switch (id) {
        /* System */
        case CONF_DEBUG_LEVEL:      g_current_config.debug_level = (uint32_t)value; break;
        case CONF_TELEMETRY_PERIOD:  
            if (value < 1) value = 1;
            if (value > 1000) value = 1000;
            g_current_config.telemetry_period_ms = (uint32_t)value; 
            break;
        case CONF_SYS_VARS_PERIOD:   
            if (value < 50) value = 50;   // Don't flood UART with status
            if (value > 5000) value = 5000; // Max 5 seconds
            g_current_config.sys_vars_period_ms = (uint32_t)value; 
            break;
        case CONF_IMU_PERIOD:       
            if (value < 5) value = 5;     // 200Hz max
            if (value > 500) value = 500;
            g_current_config.imu_publish_period_ms = (uint32_t)value; 
            break;
        case CONF_ODOM_PERIOD:      
            if (value < 5) value = 5;   // 200Hz max
            if (value > 1000) value = 1000;
            g_current_config.odom_publish_period_ms = (uint32_t)value; 
            break;

        /* Motor */
        case CONF_PID_ENABLED:      g_current_config.pid_enabled = (value > 0.5f) ? 1 : 0; break;
        case CONF_MOTOR_TICKS:      
            if (value < 1.0f) value = 1.0f;
            g_current_config.motor_ticks_per_circle = value; 
            break;
        case CONF_MOTOR_RPS_LIMIT:  
            if (value < 0.1f) value = 0.1f;
            g_current_config.motor_rps_limit = value; 
            break;
        case CONF_MOTOR_PWM_MAX:    
            if (value < 1000.0f) value = 1000.0f;
            if (value > 65535.0f) value = 65535.0f;
            g_current_config.motor_pwm_max = value; 
            break;

        /* Chassis */
        case CONF_WHEEL_DIAMETER:   
            if (value < 0.01f) value = 0.01f;
            g_current_config.wheel_diameter = value; 
            break;
        case CONF_SHAFT_WIDTH:      
            if (value < 0.01f) value = 0.05f;
            g_current_config.shaft_width = value; 
            break;
        case CONF_WHEELBASE:        
            if (value < 0.01f) value = 0.05f;
            g_current_config.wheelbase_length = value; 
            break;
        case CONF_MOBILITY_MODE:
            g_current_config.mobility_mode = (uint32_t)value;
            break;

        /* Directions (Normalize to 1 or -1) */
        case CONF_MOTOR1_INV:       g_current_config.motor1_invert = (value >= 0) ? 1 : -1; break;
        case CONF_MOTOR2_INV:       g_current_config.motor2_invert = (value >= 0) ? 1 : -1; break;
        case CONF_MOTOR3_INV:       g_current_config.motor3_invert = (value >= 0) ? 1 : -1; break;
        case CONF_MOTOR4_INV:       g_current_config.motor4_invert = (value >= 0) ? 1 : -1; break;

        /* Motor 1 Calibration */
        case CONF_MOTOR1_KP:        g_current_config.motor1_kp = value; break;
        case CONF_MOTOR1_KI:        g_current_config.motor1_ki = value; break;
        case CONF_MOTOR1_KD:        g_current_config.motor1_kd = value; break;
        case CONF_MOTOR1_DEADZONE:  g_current_config.motor1_deadzone = value; break;

        /* Motor 2 Calibration */
        case CONF_MOTOR2_KP:        g_current_config.motor2_kp = value; break;
        case CONF_MOTOR2_KI:        g_current_config.motor2_ki = value; break;
        case CONF_MOTOR2_KD:        g_current_config.motor2_kd = value; break;
        case CONF_MOTOR2_DEADZONE:  g_current_config.motor2_deadzone = value; break;

        /* Motor 3 Calibration */
        case CONF_MOTOR3_KP:        g_current_config.motor3_kp = value; break;
        case CONF_MOTOR3_KI:        g_current_config.motor3_ki = value; break;
        case CONF_MOTOR3_KD:        g_current_config.motor3_kd = value; break;
        case CONF_MOTOR3_DEADZONE:  g_current_config.motor3_deadzone = value; break;

        /* Motor 4 Calibration */
        case CONF_MOTOR4_KP:        g_current_config.motor4_kp = value; break;
        case CONF_MOTOR4_KI:        g_current_config.motor4_ki = value; break;
        case CONF_MOTOR4_KD:        g_current_config.motor4_kd = value; break;
        case CONF_MOTOR4_DEADZONE:  g_current_config.motor4_deadzone = value; break;

        default:
            LOG_ERROR(LOG_TAG, "Unknown Param ID 0x%02X\r\n", id);
            return false;
    }

    /* Notify listeners so RAM values are applied (e.g. motors read new ticks) */
    AppConfig_NotifyChange();
    return true;
}

void AppConfig_Init(void) {
    BSP_InternalFlash_Init();

#ifdef PESISTENT_CONFIG
    /* Internal flash is memory mapped, so we can access it directly for validation */
    AppConfig_t* flash_ptr = (AppConfig_t*)FLASH_ADDRESS_CONFIG;
    
    if (AppConfig_IsValid(flash_ptr)) {
        BSP_InternalFlash_Read(FLASH_ADDRESS_CONFIG, &g_current_config, sizeof(AppConfig_t));
        LOG_INFO(LOG_TAG, "Loaded from Flash (Valid Checksum)\r\n");
    } else {
        LOG_WARNING(LOG_TAG, "Flash empty or invalid! Initializing defaults...\r\n");
        AppConfig_ResetToDefaults();
    }
#else
    LOG_INFO(LOG_TAG, "Mode: STATIC (config.h used)\r\n");
    AppConfig_ResetToDefaults();
#endif
}

bool AppConfig_ReloadFromFlash(void) {
    AppConfig_t* flash_ptr = (AppConfig_t*)FLASH_ADDRESS_CONFIG;
    
    if (AppConfig_IsValid(flash_ptr)) {
        BSP_InternalFlash_Read(FLASH_ADDRESS_CONFIG, &g_current_config, sizeof(AppConfig_t));
        LOG_INFO(LOG_TAG, "Configuration reloaded from Flash (RAM sync)\r\n");
        AppConfig_NotifyChange();
        return true;
    } else {
        LOG_WARNING(LOG_TAG, "Reload failed: Flash empty or invalid checksum\r\n");
        return false;
    }
}
