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
    float batt_i = BSP_Battery_GetCurrent();
    float mcu_t  = BSP_MCU_GetInternalTemp();

    /* --- 2. Write into Robot State (Data Layer) --- */
    RobotState_SetBatteryVoltage(batt_v);
    RobotState_SetBatteryCurrent(batt_i);
    RobotState_SetUCTemperature(mcu_t);

    /* --- 3. Atomic snapshot of the full Telemetry block ---
     * A single critical section guarantees data consistency across
     * all three topic messages without repeated unprotected accesses
     * to the shared global struct.
     */
    RobotState_t snap_state;
    taskENTER_CRITICAL();
    snap_state.Telemetry = RobotState_4wcl.Telemetry;
    taskEXIT_CRITICAL();

    /* --- 4. PUBLISH TX TOPICS VIA SERIAL_ROS --- */

    /* Topic 0x81 – System Status (Health & Battery) */
    SystemStatusMsg_t status_msg;
    status_msg.current_state   = (uint8_t)snap_state.Telemetry.current_state;
    status_msg.error_flags     = snap_state.Telemetry.error_flags;
    status_msg.mcu_temp        = snap_state.Telemetry.uc_temperature;
    status_msg.battery_voltage = snap_state.Telemetry.battery_voltage;
    status_msg.battery_current = snap_state.Telemetry.battery_current;
    SerialRos_EnqueueTx(TOPIC_ID_SYS_STATUS, &status_msg, sizeof(SystemStatusMsg_t));

    /* Topic 0x82 – IMU Data */
    ImuMsg_t imu_msg;
    imu_msg.roll    = snap_state.Telemetry.roll;
    imu_msg.pitch   = snap_state.Telemetry.pitch;
    imu_msg.yaw     = snap_state.Telemetry.yaw;
    imu_msg.gyro_x  = snap_state.Telemetry.gyro_x;
    imu_msg.gyro_y  = snap_state.Telemetry.gyro_y;
    imu_msg.gyro_z  = snap_state.Telemetry.gyro_z;
    imu_msg.accel_x = snap_state.Telemetry.accel_x;
    imu_msg.accel_y = snap_state.Telemetry.accel_y;
    imu_msg.accel_z = snap_state.Telemetry.accel_z;
    SerialRos_EnqueueTx(TOPIC_ID_IMU, &imu_msg, sizeof(ImuMsg_t));

    /* Topic 0x83 – Odometry Data */
    OdometryMsg_t odom_msg;
    odom_msg.linear_x  = snap_state.Telemetry.measured_linear_x;
    odom_msg.angular_z = snap_state.Telemetry.measured_angular_z;
    odom_msg.enc_1     = snap_state.Telemetry.enc_1;
    odom_msg.enc_2     = snap_state.Telemetry.enc_2;
    odom_msg.enc_3     = snap_state.Telemetry.enc_3;
    odom_msg.enc_4     = snap_state.Telemetry.enc_4;
    SerialRos_EnqueueTx(TOPIC_ID_ODOMETRY, &odom_msg, sizeof(OdometryMsg_t));

}
