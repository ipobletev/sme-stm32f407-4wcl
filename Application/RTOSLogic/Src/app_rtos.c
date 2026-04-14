#include "app_rtos.h"
#include "config.h"

/* --- RTOS OBJECT HANDLES (Generic) --- */

/* Message Queues */
osal_queue_h stateMsgQueueHandle;
osal_queue_h uartEventQueueHandle;

/* Thread Handles */
osal_thread_h managerTaskHandle;
osal_thread_h controllerTaskHandle;
osal_thread_h defaultTaskHandle;
osal_thread_h uartListenerTaskHandle;

/* Timer Handles */
osal_timer_h heartbeatTimerHandle;

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

/**
 * @brief Initialize all application RTOS resources using OSAL.
 * This is the bridge between the Application logic and the actual RTOS.
 */
void App_RTOS_Init(void) {
    /* 1. Create Queues */
    stateMsgQueueHandle = osal_queue_create(10, sizeof(StateChangeMsg_t));
    uartEventQueueHandle = osal_queue_create(10, sizeof(StateChangeMsg_t));
    
    /* 2. Create Threads (Tasks) */
    defaultTaskHandle      = osal_thread_create(StartDefaultTask,      NULL, &defaultTask_attributes);
    managerTaskHandle      = osal_thread_create(StartManagerTask,      NULL, &managerTask_attributes);
    controllerTaskHandle   = osal_thread_create(StartControllerTask,   NULL, &controllerTask_attributes);
    uartListenerTaskHandle = osal_thread_create(StartUARTListenerTask, NULL, &uartListenerTask_attributes);

    /* 3. Create and Start Timers */
    heartbeatTimerHandle = osal_timer_create(HeartbeatTimerCallback, OSAL_TIMER_PERIODIC, NULL);
    if (heartbeatTimerHandle != NULL) {
        osal_timer_start(heartbeatTimerHandle, HEARTBEAT_PERIOD_MS);
    }
}

