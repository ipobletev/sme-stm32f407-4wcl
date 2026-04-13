#ifndef __APP_TASKS_H
#define __APP_TASKS_H

#include "cmsis_os.h"
#include "app_state_machine.h"

/* Queue handle for communicating state changes */
extern osMessageQueueId_t stateMsgQueueHandle;
extern osMessageQueueId_t uartEventQueueHandle;

/* Initialization */
void App_Tasks_Init(void);

/* Task Prototypes */
void StartManagerTask(void *argument);
void StartControllerTask(void *argument);
void StartDefaultTask(void *argument);
void StartUARTListenerTask(void *argument);

#endif /* __APP_TASKS_H */
