#include "app_rtos.h"
#include "config.h"
#include "robot_state.h"
#include "serial_ros_protocol.h"
#include "bsp_console.h"

/* --- RTOS OBJECT HANDLES (Generic) --- */

/* Message Queues */
osal_queue_h stateMsgQueueHandle;
osal_queue_h uartEventQueueHandle;
osal_queue_h rosTxQueueHandle;
osal_queue_h rosRxQueueHandle;
osal_queue_h consoleTxQueueHandle;
osal_queue_h consoleRxQueueHandle;

/* Thread Handles */
osal_thread_h managerTaskHandle;
osal_thread_h controllerTaskHandle;
osal_thread_h uartListenerTaskHandle;
osal_thread_h mobilityTaskHandle;
osal_thread_h armTaskHandle;
osal_thread_h serialRosTaskHandle;

/* Timer Handles */
osal_timer_h heartbeatTimerHandle;
osal_timer_h systemSensorsTimerHandle;

/* --- TASK ATTRIBUTES (Generic) --- */

const osal_thread_attr_t managerTask_attributes = {
  .name = "ManagerTask",
  .stack_size = 256 * 4,
  .priority = OSAL_PRIO_NORMAL,
};

const osal_thread_attr_t controllerTask_attributes = {
  .name = "ControllerTask",
  .stack_size = 384 * 4,
  .priority = OSAL_PRIO_NORMAL,
};

const osal_thread_attr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 512 * 4,
  .priority = OSAL_PRIO_NORMAL,
};

const osal_thread_attr_t uartListenerTask_attributes = {
  .name = "UARTListenerTask",
  .stack_size = 384 * 4,
  .priority = OSAL_PRIO_HIGH,
};

const osal_thread_attr_t mobilityTask_attributes = {
  .name = "MobilityTask",
  .stack_size = 512 * 4,
  .priority = OSAL_PRIO_NORMAL,
};

const osal_thread_attr_t armTask_attributes = {
  .name = "ArmTask",
  .stack_size = 512 * 4,
  .priority = OSAL_PRIO_NORMAL,
};

const osal_thread_attr_t serialRosTask_attributes = {
  .name = "SerialRosTask",
  .stack_size = 384 * 4,
  .priority = OSAL_PRIO_NORMAL,
};


/**
 * @brief Initialize all application RTOS resources using OSAL.
 * This is the bridge between the Application logic and the actual RTOS.
 */
void App_RTOS_Init(void) {
    /* 1. Create Queues */
    stateMsgQueueHandle = osal_queue_create(10, sizeof(StateChangeMsg_t));
    if (stateMsgQueueHandle == NULL) RobotState_SetErrorFlag(ERR_RTOS_QUEUE);

    uartEventQueueHandle = osal_queue_create(10, sizeof(StateChangeMsg_t));
    if (uartEventQueueHandle == NULL) RobotState_SetErrorFlag(ERR_RTOS_QUEUE);
    
    rosTxQueueHandle = osal_queue_create(10, sizeof(SerialRos_Packet_t));
    if (rosTxQueueHandle == NULL) RobotState_SetErrorFlag(ERR_RTOS_QUEUE);

    rosRxQueueHandle = osal_queue_create(10, sizeof(SerialRos_Packet_t));
    if (rosRxQueueHandle == NULL) RobotState_SetErrorFlag(ERR_RTOS_QUEUE);

    consoleTxQueueHandle = osal_queue_create(20, sizeof(Console_Packet_t));
    if (consoleTxQueueHandle == NULL) RobotState_SetErrorFlag(ERR_RTOS_QUEUE);

    consoleRxQueueHandle = osal_queue_create(10, sizeof(Console_Packet_t));
    if (consoleRxQueueHandle == NULL) RobotState_SetErrorFlag(ERR_RTOS_QUEUE);
    
    /* 2. Create Threads (Tasks) */
    managerTaskHandle      = osal_thread_create(StartManagerTask,      NULL, &managerTask_attributes);
    if (managerTaskHandle == NULL) RobotState_SetErrorFlag(ERR_RTOS_TASK);

    controllerTaskHandle   = osal_thread_create(StartControllerTask,   NULL, &controllerTask_attributes);
    if (controllerTaskHandle == NULL) RobotState_SetErrorFlag(ERR_RTOS_TASK);

    uartListenerTaskHandle = osal_thread_create(StartUARTListenerTask, NULL, &uartListenerTask_attributes);
    if (uartListenerTaskHandle == NULL) RobotState_SetErrorFlag(ERR_RTOS_TASK);

    mobilityTaskHandle = osal_thread_create(StartMobilityTask, NULL, &mobilityTask_attributes);
    if (mobilityTaskHandle == NULL) RobotState_SetErrorFlag(ERR_RTOS_TASK);

    armTaskHandle = osal_thread_create(StartArmTask, NULL, &armTask_attributes);
    if (armTaskHandle == NULL) RobotState_SetErrorFlag(ERR_RTOS_TASK);

    serialRosTaskHandle = osal_thread_create(StartSerialRosTask, NULL, &serialRosTask_attributes);
    if (serialRosTaskHandle == NULL) RobotState_SetErrorFlag(ERR_RTOS_TASK);

    /* 3. Create and Start Timers */
    heartbeatTimerHandle = osal_timer_create(HeartbeatTimerCallback, OSAL_TIMER_PERIODIC, NULL);
    if (heartbeatTimerHandle != NULL) {
        osal_status_t status = osal_timer_start(heartbeatTimerHandle, HEARTBEAT_PERIOD_MS);
        if (status != OSAL_OK) RobotState_SetErrorFlag(ERR_RTOS_TIMER);
    } else {
        RobotState_SetErrorFlag(ERR_RTOS_TIMER);
    }

    systemSensorsTimerHandle = osal_timer_create(SystemVariablesTimerCallback, OSAL_TIMER_PERIODIC, NULL);
    if (systemSensorsTimerHandle != NULL) {
        osal_status_t status = osal_timer_start(systemSensorsTimerHandle, SYSTEM_SENSORS_PERIOD_MS);
        if (status != OSAL_OK) RobotState_SetErrorFlag(ERR_RTOS_TIMER);
    } else {
        RobotState_SetErrorFlag(ERR_RTOS_TIMER);
    }
}


