#ifndef __ARM_STATE_HANDLERS_H
#define __ARM_STATE_HANDLERS_H

#include "arm_fsm.h"

/* STATE: INIT */
void ArmState_Init_OnEnter(void);
void ArmState_Init_Run(void);
void ArmState_Init_OnExit(void);

/* STATE: HOMING */
void ArmState_Homing_OnEnter(void);
void ArmState_Homing_Run(void);
void ArmState_Homing_OnExit(void);

/* STATE: IDLE */
void ArmState_Idle_OnEnter(void);
void ArmState_Idle_Run(void);
void ArmState_Idle_OnExit(void);

/* STATE: MOVING */
void ArmState_Moving_OnEnter(void);
void ArmState_Moving_Run(void);
void ArmState_Moving_OnExit(void);

/* STATE: TESTING */
void ArmState_Testing_OnEnter(void);
void ArmState_Testing_Run(void);
void ArmState_Testing_OnExit(void);

/* STATE: FAULT */
void ArmState_Fault_OnEnter(void);
void ArmState_Fault_Run(void);
void ArmState_Fault_OnExit(void);

/* STATE: ABORT */
void ArmState_Abort_OnEnter(void);
void ArmState_Abort_Run(void);
void ArmState_Abort_OnExit(void);

#endif /* __ARM_STATE_HANDLERS_H */
