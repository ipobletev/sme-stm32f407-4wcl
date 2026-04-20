#include "app_rtos.h"
#include "bsp_imu.h"
#include "motor_hardware.h"
#include "encoder_motor.h"
#include "robot_state.h"
#include "debug_module.h"
#include "app_config.h"
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
    int samples = 100;
    float sum_ax = 0, sum_ay = 0, sum_az = 0;
    float sum_gx = 0, sum_gy = 0, sum_gz = 0;

    LOG_INFO(LOG_TAG, "Stabilizing Sensors (ZUPT Init)...\r\n");
    
    for (int i = 0; i < samples; i++) {
        if (BSP_IMU_ReadRaw(&raw) == IMU_OK) {
            sum_ax += (float)raw.ax;
            sum_ay += (float)raw.ay;
            sum_az += (float)raw.az;
            sum_gx += (float)raw.gx;
            sum_gy += (float)raw.gy;
            sum_gz += (float)raw.gz;
        }
        vTaskDelay(pdMS_TO_TICKS(5));
    }

    /* Convert raw averages to SI units for the driver bias */
    float ax_b = (sum_ax / samples) * GRAVITY_MSS / accel_sf;
    float ay_b = (sum_ay / samples) * GRAVITY_MSS / accel_sf;
    float az_b = ((sum_az / samples) - accel_sf) * GRAVITY_MSS / accel_sf;
    float gx_b = (sum_gx / samples) * M_PI / (gyro_sf * 180.0f);
    float gy_b = (sum_gy / samples) * M_PI / (gyro_sf * 180.0f);
    float gz_b = (sum_gz / samples) * M_PI / (gyro_sf * 180.0f);

    BSP_IMU_SetBias(ax_b, ay_b, az_b, gx_b, gy_b, gz_b);
    
    /* Reset local biases as they are now handled by the driver */
    ax_bias = 0; ay_bias = 0; az_bias = 0;
    gx_bias = 0; gy_bias = 0; gz_bias = 0;
    
    LOG_INFO(LOG_TAG, "Calibration Complete. Gz_Bias: %.4f rad/s\r\n", gz_b);
}

/**
 * @brief Unified Sensor Acquisition Task (100Hz)
 * Handles IMU, Encoders, and Odometry Calculation.
 */
void StartSensorsTask(void *argument) {
    IMU_Status_t status;
    IMU_RawData_t raw;
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
        /* Use dynamic period from configuration */
        uint32_t period_ms = AppConfig->imu_publish_period_ms;
        if (period_ms < 5) period_ms = 5; // Sanity floor

        osal_delay_until(&last_wake_time, period_ms);
        float period_sec = (float)period_ms / 1000.0f;

        /* --- 2. ODOMETRY / ENCODERS --- */
        for (int i = 0; i < 4; i++) {
            int64_t counts = BSP_Motor_Hardware_GetEncoderCount(i);
            encoder_update(motors[i], period_sec, counts);
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

        float r_pi_d = M_PI * AppConfig->wheel_diameter;
        v1 *= r_pi_d; v2 *= r_pi_d; v3 *= r_pi_d; v4 *= r_pi_d;

        float l_plus_w = AppConfig->wheelbase_length + AppConfig->shaft_width;
        float vx = (v1 + v2 + v3 + v4) / 4.0f;
        float az = (-v1 - v2 + v3 + v4) / (4.0f * l_plus_w);

        RobotState_SetMeasuredVelocity(vx, az);


        /* --- 3. IMU (100Hz) --- */
        if (status == IMU_OK) {
            IMU_Status_t s = BSP_IMU_ReadRaw(&raw);
            Quaternion q;
            EulerAngles ea;
            BSP_IMU_ReadOrientationFull(&q, &ea);

            if (s == IMU_OK) {
                /* Values from BSP_IMU_ReadRaw are already de-biased if QMI8658 */
                float ax = (float)raw.ax / accel_sf;
                float ay = (float)raw.ay / accel_sf;
                float az_imu = (float)raw.az / accel_sf;
                float gx = (float)raw.gx / gyro_sf;
                float gy = (float)raw.gy / gyro_sf;
                float gz = (float)raw.gz / gyro_sf;

                /* --- Dynamic ZUPT (Zero Velocity Update) --- */
                /* If robot is static (encoders), gently update gyro bias to kill thermal drift */
                static int static_count = 0;
                static float accum_gx = 0, accum_gy = 0, accum_gz = 0;
                
                if (fabsf(vx) < 0.001f && fabsf(az) < 0.001f) {
                    static_count++;
                    accum_gx += gx * TO_RAD;
                    accum_gy += gy * TO_RAD;
                    accum_gz += gz * TO_RAD;

                    if (static_count >= 100) { // Every 1 second of stability
                        /* Calculate average drift in this window */
                        float avg_gx = accum_gx / static_count;
                        float avg_gy = accum_gy / static_count;
                        float avg_gz = accum_gz / static_count;

                        /* Feed back it to the driver bias (using a small gain to avoid oscillation) */
                        /* Current bias in driver is already subtracted from readings. 
                           So avg_gx is the REMAINING error. */
                        static float cal_gx = 0, cal_gy = 0, cal_gz = 0;
                        cal_gx += avg_gx * 0.2f;
                        cal_gy += avg_gy * 0.2f;
                        cal_gz += avg_gz * 0.2f;

                        BSP_IMU_SetBias(0, 0, 0, cal_gx, cal_gy, cal_gz);

                        /* Reset accumulators */
                        static_count = 0;
                        accum_gx = 0; accum_gy = 0; accum_gz = 0;
                    }
                    /* While stationary, force readings to 0 to prevent visual jitter */
                    gx = 0; gy = 0; gz = 0;
                } else {
                    static_count = 0;
                    accum_gx = 0; accum_gy = 0; accum_gz = 0;
                }

                taskENTER_CRITICAL();
                RobotState_4wcl.Telemetry.accel_x = ax * GRAVITY_MSS;
                RobotState_4wcl.Telemetry.accel_y = ay * GRAVITY_MSS;
                RobotState_4wcl.Telemetry.accel_z = az_imu * GRAVITY_MSS;
                RobotState_4wcl.Telemetry.gyro_x = gx * TO_RAD;
                RobotState_4wcl.Telemetry.gyro_y = gy * TO_RAD;
                RobotState_4wcl.Telemetry.gyro_z = gz * TO_RAD;
                taskEXIT_CRITICAL();

                /* Update full orientation (thread-safe setter) */
                RobotState_SetIMUOrientation(q, ea);
            }
        }
    }
}
