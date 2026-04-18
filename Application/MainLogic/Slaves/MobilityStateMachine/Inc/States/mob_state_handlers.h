#ifndef __MOB_STATE_HANDLERS_H
#define __MOB_STATE_HANDLERS_H

#include "mobility_fsm.h"

/* State Handler Interface */

/* INIT State */
void MobState_Init_OnEnter(void);
void MobState_Init_Run(void);
void MobState_Init_OnExit(void);

/* IDLE State */
void MobState_Idle_OnEnter(void);
void MobState_Idle_Run(void);
void MobState_Idle_OnExit(void);

/* BREAK State */
void MobState_Break_OnEnter(void);
void MobState_Break_Run(void);
void MobState_Break_OnExit(void);

/* MOVING State */
void MobState_Moving_OnEnter(void);
void MobState_Moving_Run(void);
void MobState_Moving_OnExit(void);

/* TESTING State */
void MobState_Testing_OnEnter(void);
void MobState_Testing_Run(void);
void MobState_Testing_OnExit(void);

/* FAULT State */
void MobState_Fault_OnEnter(void);
void MobState_Fault_Run(void);
void MobState_Fault_OnExit(void);

#endif /* __MOB_STATE_HANDLERS_H */
