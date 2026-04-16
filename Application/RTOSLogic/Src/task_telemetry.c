#include "app_rtos.h"
#include "config.h"
#include "robot_state.h"
#include "serial_ros.h"
#include "serial_ros_protocol.h"
#include "debug_module.h"
#include "bsp_battery.h"
#include "bsp_mcu_sensors.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>

#define LOG_TAG "TASK_TELE"

/**
 * @brief Unified Telemetry Task.
 * Manages all outgoing ROS topics at different rates to ensure
 * stable transmission and minimal RTOS overhead.
 * 
 * Rates (Base Loop 10ms):
 * - IMU: 100Hz (Every cycle)
 * - Odometry: 10Hz (Every 10 cycles)
 * - System Status: 2Hz (Every 50 cycles)
 */
void StartTelemetryTask(void *argument) {

    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t cycle_counter = 0;

    for (;;) {
        /* Maintain strict 10ms period */
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(TELEMETRY_BASE_PERIOD_MS));

        /* --- 1. IMU TOPIC (100Hz) --- */
        ImuMsg_t imu_msg;
        taskENTER_CRITICAL();
        imu_msg.roll    = RobotState_4wcl.Telemetry.roll;
        imu_msg.pitch   = RobotState_4wcl.Telemetry.pitch;
        imu_msg.yaw     = RobotState_4wcl.Telemetry.yaw;
        imu_msg.gyro_x  = RobotState_4wcl.Telemetry.gyro_x;
        imu_msg.gyro_y  = RobotState_4wcl.Telemetry.gyro_y;
        imu_msg.gyro_z  = RobotState_4wcl.Telemetry.gyro_z;
        imu_msg.accel_x = RobotState_4wcl.Telemetry.accel_x;
        imu_msg.accel_y = RobotState_4wcl.Telemetry.accel_y;
        imu_msg.accel_z = RobotState_4wcl.Telemetry.accel_z;
        taskEXIT_CRITICAL();
        SerialRos_EnqueueTx(TOPIC_ID_IMU, &imu_msg, sizeof(ImuMsg_t));

        /* --- 2. ODOMETRY TOPIC (10Hz) --- */
        if (cycle_counter % 10 == 0) {
            OdometryMsg_t odom_msg;
            taskENTER_CRITICAL();
            odom_msg.linear_x  = RobotState_4wcl.Telemetry.measured_linear_x;
            odom_msg.angular_z = RobotState_4wcl.Telemetry.measured_angular_z;
            odom_msg.enc_1     = RobotState_4wcl.Telemetry.enc_1;
            odom_msg.enc_2     = RobotState_4wcl.Telemetry.enc_2;
            odom_msg.enc_3     = RobotState_4wcl.Telemetry.enc_3;
            odom_msg.enc_4     = RobotState_4wcl.Telemetry.enc_4;
            taskEXIT_CRITICAL();
            SerialRos_EnqueueTx(TOPIC_ID_ODOMETRY, &odom_msg, sizeof(OdometryMsg_t));
        }

        /* --- 3. SYSTEM STATUS TOPIC & SENSORS (2Hz) --- */
        if (cycle_counter % 50 == 0) {
            /* Read hardware sensors directly in the task to keep them synced with reporting */
            float batt_v = BSP_Battery_GetVoltage();
            float mcu_t  = BSP_MCU_GetInternalTemp();

            /* Update Robot State */
            RobotState_SetBatteryVoltage(batt_v);
            RobotState_SetUCTemperature(mcu_t);

            /* Pack and Send */
            SystemStatusMsg_t status_msg;
            status_msg.error_flags     = RobotState_GetErrorFlags();
            status_msg.mcu_temp        = mcu_t;
            status_msg.battery_voltage = batt_v;
            status_msg.current_state   = (uint8_t)RobotState_GetSystemState();
            status_msg.mobility_state  = (uint8_t)RobotState_GetMobilityState();
            status_msg.arm_state       = (uint8_t)RobotState_GetArmState();

            SerialRos_EnqueueTx(TOPIC_ID_SYS_STATUS, &status_msg, sizeof(SystemStatusMsg_t));

            /* Report Status via DEBUG COM with Stack Diagnostics */
            uint32_t stack_uart = osal_thread_get_stack_space(uartListenerTaskHandle);
            uint32_t stack_ctrl = osal_thread_get_stack_space(controllerTaskHandle);

            uint64_t errs = RobotState_GetErrorFlags();
            char err_str[19];
            /* Manually format to 0x + 16 hex chars to avoid %llX library issues */
            snprintf(err_str, sizeof(err_str), "0x%08lX%08lX", 
                    (unsigned long)(errs >> 32), (unsigned long)(errs & 0xFFFFFFFF));


            /* Manual float formatting since library support is unreliable */
            batt_v = RobotState_GetBatteryVoltage();
            mcu_t  = RobotState_GetUCTemperature();
            char batt_str[10];
            char mcu_str[10];
            snprintf(batt_str, sizeof(batt_str), "%d.%02d", (int)batt_v, (int)(batt_v * 100) % 100);
            snprintf(mcu_str, sizeof(mcu_str), "%d.%d", (int)mcu_t, (int)(mcu_t * 10) % 10);

            /* Periodically log board health to console */
            LOG_INFO(LOG_TAG, "State: [SUP:%s MOB:%s ARM:%s] | Batt: %sV | MCU: %sC | Errors: %s | FreeStack: [CTRL:%lu UART:%lu]\n", 
                Supervisor_StateToStr(RobotState_GetSystemState()),
                Mobility_StateToStr(RobotState_GetMobilityState()),
                Arm_StateToStr(RobotState_GetArmState()),
                batt_str,
                mcu_str,
                err_str,
                (unsigned long)stack_ctrl, (unsigned long)stack_uart);
        }

        cycle_counter++;
        if (cycle_counter >= 100) cycle_counter = 0; // Reset every second to prevent overflow (though 32-bit is safe for years)
    }
}
