#ifndef __APP_RTOS_H
#define __APP_RTOS_H

#include "cmsis_os.h"
#include "app_state_machine.h"

/* --- RTOS INITIALIZATION --- */
void App_RTOS_Init(void);

/* --- MESSAGE QUEUES --- */
extern osMessageQueueId_t stateMsgQueueHandle;
extern osMessageQueueId_t uartEventQueueHandle;

/* --- THREADS (TASKS) --- */
extern osThreadId_t managerTaskHandle;
extern osThreadId_t controllerTaskHandle;
extern osThreadId_t defaultTaskHandle;
extern osThreadId_t uartListenerTaskHandle;

/* Thread Prototypes */
void StartManagerTask(void *argument);
void StartControllerTask(void *argument);
void StartDefaultTask(void *argument);
void StartUARTListenerTask(void *argument);

/* --- TIMERS --- */
extern osTimerId_t heartbeatTimerHandle;

/* Soft-Timer Callback Prototypes */
void HeartbeatTimerCallback(void *argument);

#endif /* __APP_RTOS_H */
