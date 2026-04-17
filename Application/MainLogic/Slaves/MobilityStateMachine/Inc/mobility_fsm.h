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
    MOB_FAULT         /* Hardware error in motor drives */
} MobilityState_t;

/* Interaction Interface */
const char* Mobility_StateToStr(MobilityState_t state);
void Mobility_Init(void);
void Mobility_ProcessLogic(void); /* Called from control task */
void Mobility_UpdateMeasurements(void); /* Called from telemetry task */
MobilityState_t Mobility_GetCurrentState(void);

/* Command Interface (Called by UART ROS receiver) */
void Mobility_SetCommandTarget(float linear_x, float angular_z);

#endif /* __MOBILITY_FSM_H */
