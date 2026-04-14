#ifndef __ARM_FSM_H
#define __ARM_FSM_H

#include <stdint.h>
#include <stdbool.h>

/* Robotic Arm Subsystem States */
typedef enum {
    ARM_DISABLED = 0, /* Servos unpowered */
    ARM_HOMING,       /* Calibration/Zeroing routine */
    ARM_IDLE,         /* Holding position */
    ARM_MOVING,       /* Executing IK trajectory */
    ARM_FAULT         /* Hardware error (e.g., stall) */
} ArmState_t;

/* Interaction Interface */
const char* Arm_StateToStr(ArmState_t state);
void Arm_Init(void);
void Arm_ProcessLogic(void); /* Called from RTOS Task */
ArmState_t Arm_GetCurrentState(void);

/* Command Interface (Called by UART ROS receiver) */
void Arm_SetJointTarget(float j1, float j2, float j3);

#endif /* __ARM_FSM_H */
