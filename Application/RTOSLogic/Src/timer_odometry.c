#include "app_rtos.h"
#include "robot_state.h"
#include "serial_ros.h"
#include "serial_ros_protocol.h"
#include "FreeRTOS.h"
#include "task.h"

/**
 * @brief Soft-timer callback to publish Odometry telemetry at 10Hz.
 */
void OdometryTimerCallback(void *argument)
{
    OdometryMsg_t odom_msg;

    /* Snapshot current Odometry telemetry in a thread-safe way */
    taskENTER_CRITICAL();
    odom_msg.linear_x  = RobotState_4wcl.Telemetry.measured_linear_x;
    odom_msg.angular_z = RobotState_4wcl.Telemetry.measured_angular_z;
    odom_msg.enc_1     = RobotState_4wcl.Telemetry.enc_1;
    odom_msg.enc_2     = RobotState_4wcl.Telemetry.enc_2;
    odom_msg.enc_3     = RobotState_4wcl.Telemetry.enc_3;
    odom_msg.enc_4     = RobotState_4wcl.Telemetry.enc_4;
    taskEXIT_CRITICAL();

    /* Publish to SerialRos TX Queue */
    SerialRos_EnqueueTx(TOPIC_ID_ODOMETRY, &odom_msg, sizeof(OdometryMsg_t));
}
