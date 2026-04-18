#ifndef __MOB_STATE_HANDLERS_H
#define __MOB_STATE_HANDLERS_H

#include "mobility_fsm.h"

/* --- Init State Handlers --- */
void MobState_Init_OnEnter(void);
void MobState_Init_Run(void);
void MobState_Init_OnExit(void);

/* --- Idle State Handlers --- */
void MobState_Idle_OnEnter(void);
void MobState_Idle_Run(void);
void MobState_Idle_OnExit(void);

/* --- Break State Handlers --- */
void MobState_Break_OnEnter(void);
void MobState_Break_Run(void);
void MobState_Break_OnExit(void);

/* --- Moving State Handlers --- */
void MobState_Moving_OnEnter(void);
void MobState_Moving_Run(void);
void MobState_Moving_OnExit(void);

/* --- Testing State Handlers --- */
void MobState_Testing_OnEnter(void);
void MobState_Testing_Run(void);
void MobState_Testing_OnExit(void);

/* --- Fault State Handlers --- */
void MobState_Fault_OnEnter(void);
void MobState_Fault_Run(void);
void MobState_Fault_OnExit(void);

/* --- Abort State Handlers --- */
void MobState_Abort_OnEnter(void);
void MobState_Abort_Run(void);
void MobState_Abort_OnExit(void);

#endif /* __MOB_STATE_HANDLERS_H */
