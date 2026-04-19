#include "app_rtos.h"
#include "bsp_imu.h"
#include "motor_hardware.h"
#include "encoder_motor.h"
#include "robot_state.h"
#include "debug_module.h"
#include "FreeRTOS.h"
#include "task.h"
#include "config.h"
#include <math.h>

#define LOG_TAG "TASK_SENSORS"

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

/* Constants */
#define GRAVITY_MSS     9.80665f
#define TO_RAD          0.01745329251f

/* Scale factors updated by driver */
static float accel_sf = 8192.0f; 
static float gyro_sf = 16.0f;    

/* Simple Calibration Offsets */
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

    LOG_INFO(LOG_TAG, "Stabilizing Sensors...\r\n");
    
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
 * @brief Unified Sensor Acquisition Task (100Hz)
 * Handles IMU, Encoders, and Odometry Calculation.
 */
void StartSensorsTask(void *argument) {
    IMU_Status_t status;
    IMU_RawData_t raw;
    float roll, pitch, yaw;
    uint32_t last_wake_time;
    
    /* --- 1. Init IMU --- */
    int retries = 5;
    while (retries > 0) {
        status = BSP_IMU_Init();
        if (status == IMU_OK) break;
        vTaskDelay(pdMS_TO_TICKS(500));
        retries--;
    }

    if (status == IMU_OK) {
        BSP_IMU_SetAccelFSR(1, &accel_sf);
        BSP_IMU_SetGyroFSR(7, &gyro_sf);
        LOG_INFO(LOG_TAG, "IMU Initialized (%s)\r\n", (BSP_IMU_GetDetectedType() == IMU_TYPE_QMI8658) ? "QMI8658" : "MPU6050");
        PerformCalibration();
    } else {
        LOG_ERROR(LOG_TAG, "IMU Hardware Error.");
        RobotState_SetErrorFlag(ERR_HAL_I2C);
    }

    last_wake_time = osal_get_tick();

    while (1) {
        /* Period: 10ms (100Hz) */
        osal_delay_until(&last_wake_time, 10);
        float period = 0.010f;

        /* --- 2. ODOMETRY / ENCODERS (100Hz) --- */
        for (int i = 0; i < 4; i++) {
            int64_t counts = BSP_Motor_Hardware_GetEncoderCount(i);
            encoder_update(motors[i], period, counts);
        }

        /* Update Robot State Feedback */
        RobotState_SetEncoderCounts(
            (int32_t)motors[0]->counter, (int32_t)motors[1]->counter,
            (int32_t)motors[2]->counter, (int32_t)motors[3]->counter
        );

        RobotState_SetMeasuredRPS(
            motors[0]->rps, motors[1]->rps,
            motors[2]->rps, motors[3]->rps
        );

        /* Calculate Forward Kinematics (Mecanum assumption) */
        /* Note: Right side motors (2 and 3) are usually controlled with inverted polarity elsewhere, 
           but we use their raw RPS here. We must flip signs based on mapping if needed. */
        float v1 = motors[0]->rps;
        float v2 = motors[1]->rps;
        float v3 = -motors[2]->rps; /* Restore sign mapping for kinematics */
        float v4 = -motors[3]->rps; /* Restore sign mapping for kinematics */

        float r_pi_d = M_PI * ROBOT_WHEEL_DIAMETER;
        v1 *= r_pi_d; v2 *= r_pi_d; v3 *= r_pi_d; v4 *= r_pi_d;

        float l_plus_w = ROBOT_WHEELBASE_LENGTH + ROBOT_SHAFT_WIDTH;
        float vx = (v1 + v2 + v3 + v4) / 4.0f;
        float az = (-v1 - v2 + v3 + v4) / (4.0f * l_plus_w);

        RobotState_SetMeasuredVelocity(vx, az);


        /* --- 3. IMU (100Hz) --- */
        if (status == IMU_OK) {
            IMU_Status_t s = BSP_IMU_ReadRaw(&raw);
            BSP_IMU_ReadOrientation(&pitch, &roll, &yaw);

            if (s == IMU_OK) {
                float ax = ((float)raw.ax - ax_bias) / accel_sf;
                float ay = ((float)raw.ay - ay_bias) / accel_sf;
                float az_imu = ((float)raw.az - az_bias) / accel_sf;
                float gx = ((float)raw.gx - gx_bias) / gyro_sf;
                float gy = ((float)raw.gy - gy_bias) / gyro_sf;
                float gz = ((float)raw.gz - gz_bias) / gyro_sf;

                taskENTER_CRITICAL();
                RobotState_4wcl.Telemetry.accel_x = ax * GRAVITY_MSS;
                RobotState_4wcl.Telemetry.accel_y = ay * GRAVITY_MSS;
                RobotState_4wcl.Telemetry.accel_z = az_imu * GRAVITY_MSS;
                RobotState_4wcl.Telemetry.gyro_x = gx * TO_RAD;
                RobotState_4wcl.Telemetry.gyro_y = gy * TO_RAD;
                RobotState_4wcl.Telemetry.gyro_z = gz * TO_RAD;
                RobotState_4wcl.Telemetry.roll = roll;
                RobotState_4wcl.Telemetry.pitch = pitch;
                RobotState_4wcl.Telemetry.yaw = yaw;
                taskEXIT_CRITICAL();
            }
        }
    }
}
