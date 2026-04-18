#ifndef __MOBILITY_FSM_H
#define __MOBILITY_FSM_H

#include <stdint.h>
#include <stdbool.h>

/* Mobility Subsystem States */
typedef enum {
    STATE_MOB_UNKNOWN = 0,
    STATE_MOB_INIT,         /* Initializing motors */
    STATE_MOB_IDLE,         /* Waiting for commands (Motors enabled) */
    STATE_MOB_BREAK,        /* Active stop / Holding position */
    STATE_MOB_MOVING,       /* Actively executing twist commands */
    STATE_MOB_TESTING,      /* Independent motor testing */
    STATE_MOB_FAULT         /* Hardware error in motor drives */
} MobilityState_t;

typedef enum {
    EVENT_NONE = 0,
    EVENT_INIT,
    EVENT_IDLE,
    EVENT_BREAK,
    EVENT_MOVING,
    EVENT_TESTING,
    EVENT_FAULT
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
const char* FSM_Mobility_ModeToStr(uint8_t mode);
void FSM_Mobility_Init(void);
void FSM_Mobility_ProcessLogic(void); /* Called from control task */
void FSM_Mobility_ProcessEvent(MobilityEvent_t event); /* Handle state transitions */
MobilityState_t FSM_Mobility_GetCurrentState(void);
void FSM_Mobility_UpdateMeasurements(void); /* Called from telemetry task */

/* Command Interface (Called by UART ROS receiver) */
void FSM_Mobility_SetCommandTarget(float linear_x, float angular_z);
void FSM_Mobility_SetRawMotorPulse(uint8_t motor_id, float pulse);

#endif /* __MOBILITY_FSM_H */
