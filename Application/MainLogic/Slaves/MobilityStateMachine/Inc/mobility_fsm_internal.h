#ifndef __MOBILITY_FSM_INTERNAL_H
#define __MOBILITY_FSM_INTERNAL_H

#include "mobility_fsm.h"
#include "encoder_motor.h"
#include "motor_hardware.h"

#define LOG_TAG "MOBILITY"

/* Motor Objects visible to states */
extern EncoderMotorObjectTypeDef *motors[4];

/* Shared targets */
extern float target_linear_x;
extern float target_angular_z;

/* Helper to transition between states */
void FSM_Mobility_TransitionToState(MobilityState_t newState);

#endif /* __MOBILITY_FSM_INTERNAL_H */
