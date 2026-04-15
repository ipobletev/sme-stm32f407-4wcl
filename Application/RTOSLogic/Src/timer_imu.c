#include "app_rtos.h"
#include "robot_state.h"
#include "serial_ros.h"
#include "serial_ros_protocol.h"
#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief Soft-timer callback to publish IMU telemetry at a higher frequency (20Hz).
 */
void ImuTimerCallback(void *argument)
{
    ImuMsg_t imu_msg;

    /* Snapshot current IMU telemetry in a thread-safe way */
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

    /* Publish to SerialRos TX Queue */
    SerialRos_EnqueueTx(TOPIC_ID_IMU, &imu_msg, sizeof(ImuMsg_t));
}
