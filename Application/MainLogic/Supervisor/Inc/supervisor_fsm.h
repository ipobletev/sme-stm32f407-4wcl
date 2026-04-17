#ifndef __SUPERVISOR_FSM_H
#define __SUPERVISOR_FSM_H

#include <stdint.h>

/* State Machine definitions */
typedef enum {
    STATE_INIT = 0,
    STATE_IDLE,         /* Waiting for operator to start (EVENT_START) */
    STATE_MANUAL,       /* Manual control */
    STATE_AUTO,         /* Autonomous control */
    STATE_PAUSED,       /* Paused */
    STATE_FAULT         /* Fault */
} SystemState_t;

typedef enum {
    EVENT_NONE = 0,
    EVENT_START,        /* Enable system. transition to STATE_MANUAL */
    EVENT_STOP,         /* Disable system. transition to STATE_IDLE */
    EVENT_ERROR,        /* Set system to STATE_FAULT - STOP EMERGENCY */
    EVENT_RESET,        /* Set system to STATE_INIT (if in STATE_FAULT) - CLEAR EMERGENCY */
    EVENT_MODE_MANUAL,  /* Set system to STATE_MANUAL - MANUAL MODE */
    EVENT_MODE_AUTO,    /* Set system to STATE_AUTO - AUTO MODE */
    EVENT_PAUSE,        /* Set system to STATE_PAUSED - PAUSE */
    EVENT_RESUME        /* Set system to STATE_MANUAL or STATE_AUTO - RESUME */
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
SystemState_t Supervisor_GetCurrentState(void);

#endif /* __SUPERVISOR_FSM_H */

