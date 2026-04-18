#ifndef __ARM_FSM_INTERNAL_H
#define __ARM_FSM_INTERNAL_H

#include "arm_fsm.h"

#define LOG_TAG "ARM"

/* Shared targets and status flags */
extern float target_j1;
extern float target_j2;
extern float target_j3;

extern uint8_t homing_progress;

/**
 * @brief Internal helper to handle transitions with entry/exit logic.
 */
void Arm_TransitionToState(ArmState_t newState);

#endif /* __ARM_FSM_INTERNAL_H */
