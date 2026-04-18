#ifndef __SUPERVISOR_FSM_H
#define __SUPERVISOR_FSM_H

#include <stdint.h>

#define TIMEOUT_SUPERVISOR_ERROR_MS 500

/* State Machine definitions */
typedef enum {
    STATE_SUPERVISOR_INIT = 0,
    STATE_SUPERVISOR_IDLE,         /* Waiting for operator to start (EVENT_START) */
    STATE_SUPERVISOR_MANUAL,       /* Manual control */
    STATE_SUPERVISOR_AUTO,         /* Autonomous control */
    STATE_SUPERVISOR_PAUSED,       /* Paused */
    STATE_SUPERVISOR_FAULT         /* Fault */
} SystemState_t;

typedef enum {
    EVENT_SUPERVISOR_NONE = 0,
    EVENT_SUPERVISOR_READY,        /* Ready to Idle */
    EVENT_SUPERVISOR_START,        /* Enable system. transition to STATE_MANUAL */
    EVENT_SUPERVISOR_STOP,         /* Disable system. transition to STATE_IDLE */
    EVENT_SUPERVISOR_ERROR,        /* Set system to STATE_FAULT - STOP EMERGENCY */
    EVENT_SUPERVISOR_RESET,        /* Set system to STATE_INIT (if in STATE_FAULT) - CLEAR EMERGENCY */
    EVENT_SUPERVISOR_MODE_MANUAL,  /* Set system to STATE_MANUAL - MANUAL MODE */
    EVENT_SUPERVISOR_MODE_AUTO,    /* Set system to STATE_AUTO - AUTO MODE */
    EVENT_SUPERVISOR_PAUSE,        /* Set system to STATE_PAUSED - PAUSE */
    EVENT_SUPERVISOR_RESUME        /* Set system to STATE_MANUAL or STATE_AUTO - RESUME */
} SystemEvent_t;

/* Event Source Types */
typedef enum {
    SRC_UNKNOWN = 0,
    SRC_UART3_ROS = 1,             /* Remote (Only STATE_AUTO) */
    SRC_UART1_LOCAL = 2,           /* Local Operator Console (Only STATE_MANUAL) */
    SRC_PHYSICAL = 3,              /* Physical On-Board System (Only STATE_MANUAL) */
    SRC_PHYSICAL_CRITICAL = 4,     /* Physical Critical (STATE_MANUAL, STATE_AUTO) */
    SRC_INTERNAL_SUPERVISOR = 5    /* Internal Hardware/RTOS Monitor (STATE_MANUAL, STATE_AUTO) */
} EventSource_t;

/* Message structure for the queue */
typedef struct {
    SystemEvent_t event;
    uint32_t timestamp; /* timestamp of event creation */
    uint8_t source;     /* EventSource_t who generated it */
} StateChangeMsg_t;

/* Supervisor FSM Interface */
const char* Supervisor_StateToStr(SystemState_t state);
void Supervisor_Init(void);
void Supervisor_ProcessEvent(SystemEvent_t event, uint8_t source);
void Supervisor_ProcessLogic(void); /* Called periodically from Task Manager */
SystemState_t Supervisor_GetCurrentState(void);

#endif /* __SUPERVISOR_FSM_H */

