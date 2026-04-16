#include "app_rtos.h"
#include "bsp_imu.h"
#include "robot_state.h"
#include "debug_module.h"
#include "FreeRTOS.h"
#include "task.h"
#include <math.h>

#define LOG_TAG "TASK_IMU"

/* Constants */
#define GRAVITY_MSS     9.80665f
#define TO_RAD          0.01745329251f

/* Scale factors updated by driver */
static float accel_sf = 8192.0f; 
static float gyro_sf = 16.0f;    

/* Simple Calibration Offsets (Mainly for MPU fallback) */
static float ax_bias = 0.0f, ay_bias = 0.0f, az_bias = 0.0f;
static float gx_bias = 0.0f, gy_bias = 0.0f, gz_bias = 0.0f;

/**
 * @brief Startup Calibration Routine
 */
static void PerformCalibration(void) {
    IMU_RawData_t raw;
    int samples = 50;
    float sum_ax = 0, sum_ay = 0, sum_az = 0;
    float sum_gx = 0, sum_gy = 0, sum_gz = 0;

    LOG_INFO(LOG_TAG, "Stabilizing IMU...\r\n");
    
    for (int i = 0; i < samples; i++) {
        if (BSP_IMU_ReadRaw(&raw) == IMU_OK) {
            sum_ax += (float)raw.ax;
            sum_ay += (float)raw.ay;
            sum_az += (float)raw.az;
            sum_gx += (float)raw.gx;
            sum_gy += (float)raw.gy;
            sum_gz += (float)raw.gz;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    ax_bias = sum_ax / samples;
    ay_bias = sum_ay / samples;
    az_bias = (sum_az / samples) - accel_sf; 
    gx_bias = sum_gx / samples;
    gy_bias = sum_gy / samples;
    gz_bias = sum_gz / samples;
}

/**
 * @brief IMU Acquisition Task (100Hz)
 */
void StartImuTask(void *argument) {
    IMU_Status_t status;
    IMU_RawData_t raw;
    float roll, pitch, yaw;
    uint32_t last_wake_time;
    
    /* 1. Initialize IMU Hardware (Scans for MPU or QMI) */
    int retries = 5;
    while (retries > 0) {
        status = BSP_IMU_Init();
        if (status == IMU_OK) break;
        
        LOG_WARNING(LOG_TAG, "IMU Init failed. Retrying...");
        vTaskDelay(pdMS_TO_TICKS(500));
        retries--;
    }

    if (status != IMU_OK) {
        LOG_ERROR(LOG_TAG, "IMU Hardware not found. Task suspended.");
        RobotState_SetErrorFlag(ERR_HAL_I2C);
        vTaskDelete(NULL);
    }

    /* 2. Configure Ranges */
    BSP_IMU_SetAccelFSR(1, &accel_sf);
    BSP_IMU_SetGyroFSR(7, &gyro_sf);
    
    LOG_INFO(LOG_TAG, "IMU Official Driver Running (%s)\r\n", (BSP_IMU_GetDetectedType() == IMU_TYPE_QMI8658) ? "QMI8658" : "MPU6050");

    /* 3. Initial Calibration (Stabilization) */
    PerformCalibration();

    last_wake_time = osal_get_tick();

    while (1) {
        /* Period: 10ms (100Hz) */
        osal_delay_until(&last_wake_time, 10);

        /* Read raw data for acceleration/gyroscope stream */
        status = BSP_IMU_ReadRaw(&raw);
        
        /* Read orientation using high-precision official filter */
        BSP_IMU_ReadOrientation(&pitch, &roll, &yaw);

        if (status == IMU_OK) {
            float ax = ((float)raw.ax - ax_bias) / accel_sf;
            float ay = ((float)raw.ay - ay_bias) / accel_sf;
            float az = ((float)raw.az - az_bias) / accel_sf;
            float gx = ((float)raw.gx - gx_bias) / gyro_sf;
            float gy = ((float)raw.gy - gy_bias) / gyro_sf;
            float gz = ((float)raw.gz - gz_bias) / gyro_sf;

            /* Update Global State (Thread Safe) */
            taskENTER_CRITICAL();
            RobotState_4wcl.Telemetry.accel_x = ax * GRAVITY_MSS;
            RobotState_4wcl.Telemetry.accel_y = ay * GRAVITY_MSS;
            RobotState_4wcl.Telemetry.accel_z = az * GRAVITY_MSS;
            RobotState_4wcl.Telemetry.gyro_x = gx * TO_RAD;
            RobotState_4wcl.Telemetry.gyro_y = gy * TO_RAD;
            RobotState_4wcl.Telemetry.gyro_z = gz * TO_RAD;
            RobotState_4wcl.Telemetry.roll = roll;
            RobotState_4wcl.Telemetry.pitch = pitch;
            RobotState_4wcl.Telemetry.yaw = yaw;
            taskEXIT_CRITICAL();

        } else {
            LOG_WARNING(LOG_TAG, "IMU Read Error");
        }
    }
}
