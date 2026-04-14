#ifndef __SUPERVISOR_FSM_H
#define __SUPERVISOR_FSM_H

#include <stdint.h>

/* State Machine definitions */
typedef enum {
    STATE_INIT = 0,
    STATE_IDLE,
    STATE_MANUAL,
    STATE_AUTO,
    STATE_PAUSED,
    STATE_FAULT
} SystemState_t;

typedef enum {
    EVENT_NONE = 0,
    EVENT_START,        /* Defaults to MANUAL start */
    EVENT_STOP,
    EVENT_ERROR,
    EVENT_RESET,
    EVENT_MODE_MANUAL,
    EVENT_MODE_AUTO,
    EVENT_PAUSE,
    EVENT_RESUME
} SystemEvent_t;

/* Event Source Authority Levels (Higher = More Priority) */
typedef enum {
    SRC_UNKNOWN = 0,
    SRC_UART3_ROS = 1,             /* Level 1: Remote Autonomous Control */
    SRC_UART1_LOCAL = 2,           /* Level 2: Local Operator Console */
    SRC_PHYSICAL = 3,              /* Level 3: Physical On-Board Buttons */
    SRC_INTERNAL_SUPERVISOR = 4    /* Level 4: Internal Hardware/RTOS Monitor */
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

