#include "app_tasks.h"
#include <stdio.h>

/* Handles for RTOS objects */
osMessageQueueId_t stateMsgQueueHandle;
osMessageQueueId_t uartEventQueueHandle;
osThreadId_t managerTaskHandle;
osThreadId_t controllerTaskHandle;
osThreadId_t defaultTaskHandle;
osThreadId_t uartListenerTaskHandle;

/* Task Attributes */
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
void App_Tasks_Init(void) {
    /* Create Queues */
    stateMsgQueueHandle = osMessageQueueNew(10, sizeof(StateChangeMsg_t), NULL);
    uartEventQueueHandle = osMessageQueueNew(10, sizeof(StateChangeMsg_t), NULL);
    
    /* Create Threads */
    defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);
    managerTaskHandle = osThreadNew(StartManagerTask, NULL, &managerTask_attributes);
    controllerTaskHandle = osThreadNew(StartControllerTask, NULL, &controllerTask_attributes);
    uartListenerTaskHandle = osThreadNew(StartUARTListenerTask, NULL, &uartListenerTask_attributes);
}

/**
 * @brief  Function implementing the defaultTask thread (Heartbeat).
 * @param  argument: Not used
 */
void StartDefaultTask(void *argument)
{
  printf("Application Main Task Started (Tasks Layer)\r\n");
  for(;;)
  {
    printf("Alive (Tasks Layer)\r\n");
    osDelay(1000);
  }
}
