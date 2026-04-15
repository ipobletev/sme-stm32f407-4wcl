#include "app_rtos.h"
#include "robot_state.h"
#include "bsp_battery.h"
#include "bsp_mcu_sensors.h"
#include "serial_ros.h"
#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief  System Sensors Timer Callback (Board Health Monitoring).
 * Runs periodically to sample battery voltage and MCU internal diagnostics,
 * then publishes the corresponding ROS topics via SerialRos.
 * @param  argument: Not used
 */
void SystemVariablesTimerCallback(void *argument)
{
    /* --- 1. Read from BSPs (Hardware Layer) --- */
    float batt_v = BSP_Battery_GetVoltage();
    /* float batt_i = BSP_Battery_GetCurrent(); */
    float mcu_t  = BSP_MCU_GetInternalTemp();

    /* --- 2. Write into Robot State (Data Layer) --- */
    RobotState_SetBatteryVoltage(batt_v);
    /* RobotState_SetBatteryCurrent(batt_i); */
    RobotState_SetUCTemperature(mcu_t);

    /* --- 3. PUBLISH TX TOPICS VIA SERIAL_ROS --- 
     * We don't snapshot the entire RobotState anymore to save stack space.
     * Copying specific fields into the message structs is safer.
     */

    /* Topic 0x81 – System Status (Health & Battery) */
    SystemStatusMsg_t status_msg;
    status_msg.current_state   = (uint8_t)RobotState_GetSystemState();
    status_msg.mobility_state  = (uint8_t)RobotState_GetMobilityState();
    status_msg.arm_state       = (uint8_t)RobotState_GetArmState();
    status_msg.error_flags     = RobotState_GetErrorFlags();
    status_msg.mcu_temp        = mcu_t;
    status_msg.battery_voltage = batt_v;
    /* status_msg.battery_current = batt_i; */
    SerialRos_EnqueueTx(TOPIC_ID_SYS_STATUS, &status_msg, sizeof(SystemStatusMsg_t));

    /* Topic 0x82 – IMU Data */
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

    /* Topic 0x83 – Odometry Data */
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

