#ifndef __APP_RTOS_H
#define __APP_RTOS_H

#include "osal.h"
#include "supervisor_fsm.h"

/* --- RTOS INITIALIZATION --- */
void App_RTOS_Init(void);

/* --- MESSAGE QUEUES --- */
extern osal_queue_h stateMsgQueueHandle;
extern osal_queue_h uartEventQueueHandle;
extern osal_queue_h rosTxQueueHandle;
extern osal_queue_h rosRxQueueHandle;
extern osal_queue_h consoleTxQueueHandle;
extern osal_queue_h consoleRxQueueHandle;

/* --- THREADS (TASKS) --- */
extern osal_thread_h managerTaskHandle;
extern osal_thread_h controllerTaskHandle;
extern osal_thread_h uartListenerTaskHandle;
extern osal_thread_h mobilityTaskHandle;
extern osal_thread_h armTaskHandle;
extern osal_thread_h serialRosTaskHandle;
extern osal_thread_h telemetryTaskHandle;

/* Thread Prototypes */
void StartManagerTask(void *argument);
void StartControllerTask(void *argument);
void StartDefaultTask(void *argument);
void StartUARTListenerTask(void *argument);
void StartMobilityTask(void *argument);
void StartArmTask(void *argument);
void StartSerialRosTask(void *argument);
void StartTelemetryTask(void *argument);

/* --- TIMERS --- */
extern osal_timer_h heartbeatTimerHandle;

/* Soft-Timer Callback Prototypes */
void HeartbeatTimerCallback(void *argument);

#endif /* __APP_RTOS_H */

