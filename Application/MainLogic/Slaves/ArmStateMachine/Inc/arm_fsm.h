#ifndef __ARM_FSM_H
#define __ARM_FSM_H

#include <stdint.h>
#include <stdbool.h>

/* Robotic Arm Subsystem States */
typedef enum {
    STATE_ARM_UNKNOWN = 0, 
    STATE_ARM_INIT,     /* Initializing / Servos unpowered */
    STATE_ARM_HOMING,   /* Calibration/Zeroing routine */
    STATE_ARM_IDLE,     /* Holding position */
    STATE_ARM_MOVING,   /* Executing IK trajectory */
    STATE_ARM_TESTING,  /* Raw servo control for debugging */
    STATE_ARM_FAULT     /* Hardware error (e.g., stall) */
} ArmState_t;

typedef enum {
    EVENT_ARM_NONE = 0,
    EVENT_ARM_INIT,
    EVENT_ARM_IDLE,
    EVENT_ARM_HOMING,
    EVENT_ARM_MOVING,
    EVENT_ARM_TESTING,
    EVENT_ARM_FAULT
} ArmEvent_t;

/* Interaction Interface */
const char* FSM_Arm_StateToStr(ArmState_t state);
void FSM_Arm_Init(void);
void FSM_Arm_ProcessLogic(void); /* Called from RTOS Task */
void FSM_Arm_ProcessEvent(ArmEvent_t event); /* Handle state transitions */
void FSM_Arm_SetRawServoPulse(uint8_t servo_id, int16_t pulse); /* Trigger testing state */
ArmState_t FSM_Arm_GetCurrentState(void);

/* Command Interface (Called by UART ROS receiver) */
void FSM_Arm_SetJointTarget(float j1, float j2, float j3);

#endif /* __ARM_FSM_H */
