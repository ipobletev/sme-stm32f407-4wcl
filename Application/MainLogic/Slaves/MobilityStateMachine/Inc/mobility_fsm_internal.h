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

/**
 * @brief Internal helper to handle transitions with entry/exit logic.
 */
// Declaration moved to mobility_fsm.h for Supervisor access

#endif /* __MOBILITY_FSM_INTERNAL_H */
