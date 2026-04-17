#ifndef __MOBILITY_FSM_H
#define __MOBILITY_FSM_H

#include <stdint.h>
#include <stdbool.h>

/* Mobility Subsystem States */
typedef enum {
    MOB_DISABLED = 0, /* Motors powered off / Safe */
    MOB_IDLE,         /* Waiting for commands (Motors enabled) */
    MOB_BREAK,        /* Active stop / Holding position */
    MOB_MOVING,       /* Actively executing twist commands */
    MOB_TESTING,      /* Independent motor testing */
    MOB_FAULT         /* Hardware error in motor drives */
} MobilityState_t;

/* Mobility Kinematic Models */
typedef enum {
    MOB_MODE_DIRECT = 0,    /* Individual motor control */
    MOB_MODE_DIFF,          /* Differential Drive (2WD) */
    MOB_MODE_ACKERMANN,     /* Ackermann Steering */
    MOB_MODE_MECANUM        /* Mecanum Holonomic (4WD) */
} MobilityMode_t;

/* Interaction Interface */
const char* Mobility_StateToStr(MobilityState_t state);
const char* Mobility_ModeToStr(uint8_t mode);
void Mobility_Init(void);
void Mobility_ProcessLogic(void); /* Called from control task */
void Mobility_UpdateMeasurements(void); /* Called from telemetry task */
MobilityState_t Mobility_GetCurrentState(void);

/* Command Interface (Called by UART ROS receiver) */
void Mobility_SetCommandTarget(float linear_x, float angular_z);
void Mobility_SetRawMotorPulse(uint8_t motor_id, float pulse);

#endif /* __MOBILITY_FSM_H */
