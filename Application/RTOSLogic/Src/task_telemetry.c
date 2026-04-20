#include "app_rtos.h"
#include "supervisor_fsm.h"
#include "app_config.h"
#include "robot_state.h"
#include "serial_ros.h"
#include "serial_ros_protocol.h"
#include "debug_module.h"
#include "bsp_battery.h"
#include "bsp_mcu_sensors.h"
#include "FreeRTOS.h"
#include "task.h"
#include <stdio.h>
#include <stdlib.h>

#define LOG_TAG "TASK_TELE"

/**
 * @brief Unified Telemetry Task.
 * Manages all outgoing ROS topics at different rates to ensure
 * stable transmission and minimal RTOS overhead.
 * 
 * Rates (Base Loop controlled by AppConfig):
 * - IMU: Configurable (AppConfig->imu_publish_period_ms)
 * - Odometry: Configurable (AppConfig->odom_publish_period_ms)
 * - System Status: Configurable (AppConfig->sys_vars_period_ms)
 */
void StartTelemetryTask(void *argument) {

    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint32_t last_imu_tick = 0;
    uint32_t last_odom_tick = 0;
    uint32_t last_sys_tick = 0;
    uint32_t cycle_counter = 0;

    for (;;) {
        /* Maintain strict period based on configuration */
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(AppConfig->telemetry_period_ms));
        
        uint32_t current_tick = osal_get_tick();
 
        /* --- 1. IMU TOPIC --- */
        if (current_tick - last_imu_tick >= AppConfig->imu_publish_period_ms) {
            last_imu_tick = current_tick;
            ImuMsg_t imu_msg;
            taskENTER_CRITICAL();
            imu_msg.qx      = RobotState_4wcl.Telemetry.qx;
            imu_msg.qy      = RobotState_4wcl.Telemetry.qy;
            imu_msg.qz      = RobotState_4wcl.Telemetry.qz;
            imu_msg.qw      = RobotState_4wcl.Telemetry.qw;
            imu_msg.gyro_x  = RobotState_4wcl.Telemetry.gyro_x;
            imu_msg.gyro_y  = RobotState_4wcl.Telemetry.gyro_y;
            imu_msg.gyro_z  = RobotState_4wcl.Telemetry.gyro_z;
            imu_msg.accel_x = RobotState_4wcl.Telemetry.accel_x;
            imu_msg.accel_y = RobotState_4wcl.Telemetry.accel_y;
            imu_msg.accel_z = RobotState_4wcl.Telemetry.accel_z;
            imu_msg.roll    = RobotState_4wcl.Telemetry.roll;
            imu_msg.pitch   = RobotState_4wcl.Telemetry.pitch;
            imu_msg.yaw     = RobotState_4wcl.Telemetry.yaw;
            taskEXIT_CRITICAL();
            SerialRos_EnqueueTx(TOPIC_ID_IMU, &imu_msg, sizeof(ImuMsg_t));
        }


        /* --- 2. ODOMETRY & WHEEL STATE (Configurable Rate) --- */
        if (current_tick - last_odom_tick >= AppConfig->odom_publish_period_ms) {
            last_odom_tick = current_tick;
            OdometryMsg_t odom_msg;
            
            taskENTER_CRITICAL();
            /* Kinematics */
            odom_msg.linear_x  = RobotState_4wcl.Telemetry.measured_linear_x;
            odom_msg.angular_z = RobotState_4wcl.Telemetry.measured_angular_z;
            
            /* Accumulators (Position) */
            odom_msg.enc_1     = RobotState_4wcl.Telemetry.enc_1;
            odom_msg.enc_2     = RobotState_4wcl.Telemetry.enc_2;
            odom_msg.enc_3     = RobotState_4wcl.Telemetry.enc_3;
            odom_msg.enc_4     = RobotState_4wcl.Telemetry.enc_4;

            /* PID Feedback (Velocity & Effort) */
            odom_msg.target_rps[0]   = RobotState_4wcl.Telemetry.target_rps_1;
            odom_msg.target_rps[1]   = RobotState_4wcl.Telemetry.target_rps_2;
            odom_msg.target_rps[2]   = RobotState_4wcl.Telemetry.target_rps_3;
            odom_msg.target_rps[3]   = RobotState_4wcl.Telemetry.target_rps_4;
            
            odom_msg.measured_rps[0] = RobotState_4wcl.Telemetry.measured_rps_1;
            odom_msg.measured_rps[1] = RobotState_4wcl.Telemetry.measured_rps_2;
            odom_msg.measured_rps[2] = RobotState_4wcl.Telemetry.measured_rps_3;
            odom_msg.measured_rps[3] = RobotState_4wcl.Telemetry.measured_rps_4;
            
            odom_msg.pwm_output[0]   = RobotState_4wcl.Telemetry.pwm_output_1;
            odom_msg.pwm_output[1]   = RobotState_4wcl.Telemetry.pwm_output_2;
            odom_msg.pwm_output[2]   = RobotState_4wcl.Telemetry.pwm_output_3;
            odom_msg.pwm_output[3]   = RobotState_4wcl.Telemetry.pwm_output_4;
            taskEXIT_CRITICAL();

            SerialRos_EnqueueTx(TOPIC_ID_ODOMETRY, &odom_msg, sizeof(OdometryMsg_t));
        }

        /* --- 3. SYSTEM CONFIG/APP CONFIG (Low Priority, Async) --- */
        if (current_tick - last_sys_tick >= AppConfig->sys_vars_period_ms) {
            last_sys_tick = current_tick;
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
            status_msg.mobility_mode    = (uint8_t)RobotState_GetTargetMobilityMode();

            SerialRos_EnqueueTx(TOPIC_ID_SYS_STATUS, &status_msg, sizeof(SystemStatusMsg_t));

            uint64_t errs = RobotState_GetErrorFlags();
            char err_str[19];
            /* Manually format to 0x + 16 hex chars to avoid %llX library issues */
            snprintf(err_str, sizeof(err_str), "0x%08lX%08lX", 
                    (unsigned long)(errs >> 32), (unsigned long)(errs & 0xFFFFFFFF));


            /* Manual float formatting since library support is unreliable */
            float batt_v_val = RobotState_GetBatteryVoltage();
            float mcu_t_val  = RobotState_GetUCTemperature();

            float cmd_lx, cmd_az;
            RobotState_GetTargetVelocity(&cmd_lx, &cmd_az);

            char batt_str[10];
            char mcu_str[10];
            char cmd_lx_str[10];
            char cmd_az_str[10];

            snprintf(batt_str, sizeof(batt_str), "%d.%02d", (int)batt_v_val, (int)(batt_v_val * 100) % 100);
            snprintf(mcu_str, sizeof(mcu_str), "%d.%d", (int)mcu_t_val, abs((int)(mcu_t_val * 10) % 10));

            int lx_int = (int)cmd_lx;
            int lx_frac = abs((int)(cmd_lx * 100) % 100);
            int az_int = (int)cmd_az;
            int az_frac = abs((int)(cmd_az * 100) % 100);
            snprintf(cmd_lx_str, sizeof(cmd_lx_str), "%s%d.%02d", (cmd_lx < 0 && lx_int == 0) ? "-" : "", lx_int, lx_frac);
            snprintf(cmd_az_str, sizeof(cmd_az_str), "%s%d.%02d", (cmd_az < 0 && az_int == 0) ? "-" : "", az_int, az_frac);

            /* Periodically log board health to console */
            /* Report basic status to INFO level */
            LOG_INFO(LOG_TAG, "State: [SUP:%s MOB:%s ARM:%s] | CmdVel: [%s, %s, %s] | Batt: %sV | MCU: %sC | Errors: %s\r\n", 
                Supervisor_StateToStr(RobotState_GetSystemState()),
                FSM_Mobility_StateToStr(RobotState_GetMobilityState()),
                FSM_Arm_StateToStr(RobotState_GetArmState()),
                cmd_lx_str, cmd_az_str, FSM_Mobility_ModeToStr(RobotState_GetTargetMobilityMode()),
                batt_str,
                mcu_str,
                err_str);

            /* Report detailed diagnostics only to DEBUG level */
            if (AppConfig->debug_level >= LOG_LEVEL_DEBUG) {
                uint32_t s_mng  = osal_thread_get_stack_space(managerTaskHandle);
                uint32_t s_ctl  = osal_thread_get_stack_space(controllerTaskHandle);
                uint32_t s_urt  = osal_thread_get_stack_space(uartListenerTaskHandle);
                uint32_t s_mob  = osal_thread_get_stack_space(mobilityTaskHandle);
                uint32_t s_arm  = osal_thread_get_stack_space(armTaskHandle);
                uint32_t s_ros  = osal_thread_get_stack_space(serialRosTaskHandle);
                uint32_t s_tel  = osal_thread_get_stack_space(telemetryTaskHandle);
                uint32_t s_imu  = osal_thread_get_stack_space(sensorsTaskHandle);

                LOG_DEBUG(LOG_TAG, "FreeStack: [MNG:%lu CTL:%lu URT:%lu MOB:%lu ARM:%lu ROS:%lu TEL:%lu IMU:%lu]\r\n", 
                    (unsigned long)s_mng, (unsigned long)s_ctl, (unsigned long)s_urt, (unsigned long)s_mob,
                    (unsigned long)s_arm, (unsigned long)s_ros, (unsigned long)s_tel, (unsigned long)s_imu);
            }

            // Debug configuration values (Split to avoid buffer overflow)
            LOG_DEBUG(LOG_TAG, "Cfg-Sys: [Dbg:%d | Tel.ms:%d | SysV.ms:%d | IMU.ms:%d | Odom.ms:%d | PIDenable:%d]\r\n", 
                AppConfig->debug_level, AppConfig->telemetry_period_ms, AppConfig->sys_vars_period_ms, 
                AppConfig->imu_publish_period_ms, AppConfig->odom_publish_period_ms, AppConfig->pid_enabled);

            LOG_DEBUG(LOG_TAG, "Cfg-Mot: [M1:%d | M2:%d | M3:%d | M4:%d] | Ticks:%d | RPS:%d | PWM:%d\r\n", 
                AppConfig->motor1_invert, AppConfig->motor2_invert, AppConfig->motor3_invert, AppConfig->motor4_invert,
                (int)AppConfig->motor_ticks_per_circle, (int)AppConfig->motor_rps_limit, (int)AppConfig->motor_pwm_max);

            LOG_DEBUG(LOG_TAG, "Cfg-Phys: [D:%dmm | W:%dmm | L:%dmm]\r\n", 
                (int)(AppConfig->wheel_diameter * 1000.0f), (int)(AppConfig->shaft_width * 1000.0f), (int)(AppConfig->wheelbase_length * 1000.0f));

            // Prepare PID strings (Avoid %f as it may not be supported in some printf versions)
            char m1_pid[40], m2_pid[40], m3_pid[40], m4_pid[40];
            
            // Convert floats to strings manually as some printf implementations don't support %f
            snprintf(m1_pid, sizeof(m1_pid), "%d.%02d,%d.%02d,%d.%02d,%d", 
                (int)AppConfig->motor1_kp, abs((int)(AppConfig->motor1_kp*100)%100),
                (int)AppConfig->motor1_ki, abs((int)(AppConfig->motor1_ki*100)%100),
                (int)AppConfig->motor1_kd, abs((int)(AppConfig->motor1_kd*100)%100),
                (int)AppConfig->motor1_deadzone);
            snprintf(m2_pid, sizeof(m2_pid), "%d.%02d,%d.%02d,%d.%02d,%d", 
                (int)AppConfig->motor2_kp, abs((int)(AppConfig->motor2_kp*100)%100),
                (int)AppConfig->motor2_ki, abs((int)(AppConfig->motor2_ki*100)%100),
                (int)AppConfig->motor2_kd, abs((int)(AppConfig->motor2_kd*100)%100),
                (int)AppConfig->motor2_deadzone);
            snprintf(m3_pid, sizeof(m3_pid), "%d.%02d,%d.%02d,%d.%02d,%d", 
                (int)AppConfig->motor3_kp, abs((int)(AppConfig->motor3_kp*100)%100),
                (int)AppConfig->motor3_ki, abs((int)(AppConfig->motor3_ki*100)%100),
                (int)AppConfig->motor3_kd, abs((int)(AppConfig->motor3_kd*100)%100),
                (int)AppConfig->motor3_deadzone);
            snprintf(m4_pid, sizeof(m4_pid), "%d.%02d,%d.%02d,%d.%02d,%d", 
                (int)AppConfig->motor4_kp, abs((int)(AppConfig->motor4_kp*100)%100),
                (int)AppConfig->motor4_ki, abs((int)(AppConfig->motor4_ki*100)%100),
                (int)AppConfig->motor4_kd, abs((int)(AppConfig->motor4_kd*100)%100),
                (int)AppConfig->motor4_deadzone);

            LOG_DEBUG(LOG_TAG, "Cfg-PID: [M1:%s | M2:%s | M3:%s | M4:%s]\r\n", m1_pid, m2_pid, m3_pid, m4_pid);
        }

        cycle_counter++;
        if (cycle_counter >= 100) cycle_counter = 0;
    }
}
