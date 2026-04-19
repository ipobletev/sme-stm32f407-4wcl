#ifndef __MOBILITY_FSM_H
#define __MOBILITY_FSM_H

#include <stdint.h>
#include <stdbool.h>

/* Mobility Subsystem States */
typedef enum {
    STATE_MOB_INIT = 0,         /* Initializing motors */
    STATE_MOB_IDLE,         /* Waiting for commands (Motors enabled) */
    STATE_MOB_BREAK,        /* Active stop / Holding position */
    STATE_MOB_MOVING,       /* Actively executing twist commands */
    STATE_MOB_TESTING,      /* Independent motor testing */
    STATE_MOB_FAULT,        /* Hardware error in motor drives */
    STATE_MOB_ABORT         /* System-wide stop / External fault */
} MobilityState_t;

typedef enum {
    EVENT_MOB_INIT = 0,
    EVENT_MOB_IDLE,
    EVENT_MOB_BREAK,
    EVENT_MOB_MOVING,
    EVENT_MOB_TESTING,
    EVENT_MOB_FAULT,
    EVENT_MOB_ABORT
} MobilityEvent_t;

/* Mobility Kinematic Models */
typedef enum {
    MOB_MODE_DIRECT = 0,    /* Individual motor control */
    MOB_MODE_DIFF,          /* Differential Drive (2WD) */
    MOB_MODE_ACKERMANN,     /* Ackermann Steering */
    MOB_MODE_MECANUM        /* Mecanum Holonomic (4WD) */
} MobilityMode_t;

/* Interaction Interface */
const char* FSM_Mobility_StateToStr(MobilityState_t state);
const char* FSM_Mobility_ModeToStr(MobilityMode_t mode);
void FSM_Mobility_Init(void);
void FSM_Mobility_ProcessLogic(void); /* Called from control task */
void FSM_Mobility_ProcessEvent(MobilityEvent_t event); /* Handle state transitions */
void FSM_Mobility_TransitionToState(MobilityState_t newState); /* Set state immediately */
MobilityState_t FSM_Mobility_GetCurrentState(void);

#endif /* __MOBILITY_FSM_H */
