#include "app_rtos.h"
#include "config.h"

/* --- RTOS OBJECT HANDLES --- */

/* Message Queues */
osMessageQueueId_t stateMsgQueueHandle;
osMessageQueueId_t uartEventQueueHandle;

/* Thread Handles */
osThreadId_t managerTaskHandle;
osThreadId_t controllerTaskHandle;
osThreadId_t defaultTaskHandle;
osThreadId_t uartListenerTaskHandle;

/* Timer Handles */
osTimerId_t heartbeatTimerHandle;

/* --- TASK ATTRIBUTES --- */

const osThreadAttr_t managerTask_attributes = {
  .name = "ManagerTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t controllerTask_attributes = {
  .name = "ControllerTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 3000 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

const osThreadAttr_t uartListenerTask_attributes = {
  .name = "UARTListenerTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};

/**
 * @brief Initialize all application RTOS resources.
 * This is called from Core/Src/freertos.c
 */
void App_RTOS_Init(void) {
    /* 1. Create Queues */
    stateMsgQueueHandle = osMessageQueueNew(10, sizeof(StateChangeMsg_t), NULL);
    uartEventQueueHandle = osMessageQueueNew(10, sizeof(StateChangeMsg_t), NULL);
    
    /* 2. Create Threads (Tasks) */
    defaultTaskHandle      = osThreadNew(StartDefaultTask,      NULL, &defaultTask_attributes);
    managerTaskHandle      = osThreadNew(StartManagerTask,      NULL, &managerTask_attributes);
    controllerTaskHandle   = osThreadNew(StartControllerTask,   NULL, &controllerTask_attributes);
    uartListenerTaskHandle = osThreadNew(StartUARTListenerTask, NULL, &uartListenerTask_attributes);

    /* 3. Create and Start Timers */
    heartbeatTimerHandle = osTimerNew(HeartbeatTimerCallback, osTimerPeriodic, NULL, NULL);
    if (heartbeatTimerHandle != NULL) {
        osTimerStart(heartbeatTimerHandle, HEARTBEAT_PERIOD_MS);
    }
}
