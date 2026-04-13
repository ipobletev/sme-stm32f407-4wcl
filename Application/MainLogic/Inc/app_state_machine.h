#ifndef __APP_STATE_MACHINE_H
#define __APP_STATE_MACHINE_H

#include <stdint.h>

/* State Machine definitions */
typedef enum {
    STATE_INIT = 0,
    STATE_IDLE,
    STATE_ACTIVE,
    STATE_FAULT
} SystemState_t;

typedef enum {
    EVENT_NONE = 0,
    EVENT_START,
    EVENT_STOP,
    EVENT_ERROR,
    EVENT_RESET
} SystemEvent_t;

/* Message structure for the queue */
typedef struct {
    SystemEvent_t event;
    uint32_t timestamp; /* timestamp of event creation */
} StateChangeMsg_t;

/* State Machine Interface */
void SM_Init(void);
void SM_ProcessEvent(SystemEvent_t event);
SystemState_t SM_GetCurrentState(void);

#endif /* __APP_STATE_MACHINE_H */
