#include "app_rtos.h"
#include "supervisor_fsm.h"
#include "app_config.h"
#include "robot_state.h"
#include "serial_ros.h"
#include "serial_ros_protocol.h"
#include "debug_module.h"
#include "bsp_battery.h"
#include "bsp_mcu_sensors.h"
#include "usb_joystick.h"
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
    uint32_t last_joy_tick = 0;
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
            odom_msg.target_speed[0]   = RobotState_4wcl.Telemetry.target_speed_1;
            odom_msg.target_speed[1]   = RobotState_4wcl.Telemetry.target_speed_2;
            odom_msg.target_speed[2]   = RobotState_4wcl.Telemetry.target_speed_3;
            odom_msg.target_speed[3]   = RobotState_4wcl.Telemetry.target_speed_4;
            
            odom_msg.measured_speed[0] = RobotState_4wcl.Telemetry.measured_speed_1;
            odom_msg.measured_speed[1] = RobotState_4wcl.Telemetry.measured_speed_2;
            odom_msg.measured_speed[2] = RobotState_4wcl.Telemetry.measured_speed_3;
            odom_msg.measured_speed[3] = RobotState_4wcl.Telemetry.measured_speed_4;
            
            odom_msg.pwm_output[0]   = RobotState_4wcl.Telemetry.pwm_output_1;
            odom_msg.pwm_output[1]   = RobotState_4wcl.Telemetry.pwm_output_2;
            odom_msg.pwm_output[2]   = RobotState_4wcl.Telemetry.pwm_output_3;
            odom_msg.pwm_output[3]   = RobotState_4wcl.Telemetry.pwm_output_4;
            taskEXIT_CRITICAL();

            SerialRos_EnqueueTx(TOPIC_ID_ODOMETRY, &odom_msg, sizeof(OdometryMsg_t));
            
            LOG_DEBUG(LOG_TAG, "M1 TRG: %.2f, PWM: %d, SPD: %.2f M2 TRG: %.2f, PWM: %d, SPD: %.2f M3 TRG: %.2f, PWM: %d, SPD: %.2f M4 TRG: %.2f, PWM: %d, SPD: %.2f\r\n", 
                odom_msg.target_speed[0], (int)odom_msg.pwm_output[0], odom_msg.measured_speed[0],
                odom_msg.target_speed[1], (int)odom_msg.pwm_output[1], odom_msg.measured_speed[1],
                odom_msg.target_speed[2], (int)odom_msg.pwm_output[2], odom_msg.measured_speed[2],
                odom_msg.target_speed[3], (int)odom_msg.pwm_output[3], odom_msg.measured_speed[3]);
            
            /* Commented out by default to reduce console noise. Update format if enabling. */
            // LOG_DEBUG(LOG_TAG, "M2 TRGT: %.2f, PWM: %d, SPD: %.2f\r\n", odom_msg.target_speed[1], (int)odom_msg.pwm_output[1], odom_msg.measured_speed[1]);
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
            status_msg.enable_autonomous = RobotState_GetEnableAutonomous();
            status_msg.emergency_active = RobotState_GetEmergencyActive();

            SerialRos_EnqueueTx(TOPIC_ID_SYS_STATUS, &status_msg, sizeof(SystemStatusMsg_t));
            // LOG_INFO(LOG_TAG, "MCU Heartbeat (Status) Sent. Size: %d\r\n", (int)sizeof(SystemStatusMsg_t));

            uint64_t errs = RobotState_GetErrorFlags();
            char err_str[19];
            /* Manually format to 0x + 16 hex chars to avoid %llX library issues */
            snprintf(err_str, sizeof(err_str), "0x%08lX%08lX", 
                    (unsigned long)(errs >> 32), (unsigned long)(errs & 0xFFFFFFFF));


            float cmd_lx, cmd_az;
            RobotState_GetTargetVelocity(&cmd_lx, &cmd_az);

            /* Report basic status to INFO level (Native float support) */
            LOG_INFO(LOG_TAG, "HW: [EMGY:%d AUT:%d] | State: [SUP:%s MOB:%s ARM:%s] | CmdVel: [%.2f, %.2f, %s] | BATT/TEMP: [%.2fV %.1fC] | Errors: %s\r\n", 
                RobotState_GetEmergencyActive(),
                RobotState_GetEnableAutonomous(),
                Supervisor_StateToStr(RobotState_GetSystemState()),
                FSM_Mobility_StateToStr(RobotState_GetMobilityState()),
                FSM_Arm_StateToStr(RobotState_GetArmState()),
                cmd_lx, cmd_az, FSM_Mobility_ModeToStr(RobotState_GetTargetMobilityMode()),
                batt_v,
                mcu_t,
                err_str);

            /* Report detailed diagnostics only to DEBUG level */
            if (AppConfig->debug_level >= LOG_LEVEL_DEBUG) {
            uint32_t s_mng  = osal_thread_get_stack_space(managerTaskHandle);
            uint32_t s_ctl  = osal_thread_get_stack_space(hwInputTaskHandle);
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

            LOG_DEBUG(LOG_TAG, "Cfg-Mot: [M1:%d | M2:%d | M3:%d | M4:%d] | Ticks:%.1f | Spd:%.2f | PWM:%d\r\n", 
                (int)AppConfig->motor1_invert, (int)AppConfig->motor2_invert, (int)AppConfig->motor3_invert, (int)AppConfig->motor4_invert, 
                AppConfig->motor_ticks_per_circle,
                AppConfig->motor_speed_limit,
                (int)AppConfig->motor_pwm_max);

            LOG_DEBUG(LOG_TAG, "Cfg-Phys: [D:%.3fm | W:%.3fm | L:%.3fm]\r\n", 
                AppConfig->wheel_diameter,
                AppConfig->shaft_width,
                AppConfig->wheelbase_length);
            
            LOG_DEBUG(LOG_TAG, "Cfg-PID: [M1:%.2f,%.2f,%.2f,%.1f | M2:%.2f,%.2f,%.2f,%.1f | M3:%.2f,%.2f,%.2f,%.1f | M4:%.2f,%.2f,%.2f,%.1f]\r\n", 
                AppConfig->motor1_kp, AppConfig->motor1_ki, AppConfig->motor1_kd, AppConfig->motor1_deadzone,
                AppConfig->motor2_kp, AppConfig->motor2_ki, AppConfig->motor2_kd, AppConfig->motor2_deadzone,
                AppConfig->motor3_kp, AppConfig->motor3_ki, AppConfig->motor3_kd, AppConfig->motor3_deadzone,
                AppConfig->motor4_kp, AppConfig->motor4_ki, AppConfig->motor4_kd, AppConfig->motor4_deadzone);
        }
        
        /* --- 4. JOYSTICK STATE (10Hz) --- */
        if (current_tick - last_joy_tick >= 100) {
            last_joy_tick = current_tick;
            
            if (USB_Joystick_IsConnected()) {
                USB_Joystick_State_t *js = USB_Joystick_GetState();
                JoystickStateMsg_t joy_msg;
                
                joy_msg.lx = js->lx;
                joy_msg.ly = js->ly;
                joy_msg.rx = js->rx;
                joy_msg.ry = js->ry;
                joy_msg.l2 = js->l2;
                joy_msg.r2 = js->r2;
                joy_msg.buttons = js->buttons;
                joy_msg.connected = js->connected;
                
                SerialRos_EnqueueTx(TOPIC_ID_JOYSTICK_DATA, &joy_msg, sizeof(JoystickStateMsg_t));
            }
        }

        cycle_counter++;
        if (cycle_counter >= 100) cycle_counter = 0;
    }
}
