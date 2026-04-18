#ifndef __STATE_HANDLERS_H
#define __STATE_HANDLERS_H

#include "supervisor_fsm.h"
#define LOG_TAG "SUPERVISOR"

/* State Handler Interface */

/* Standard logic that runs in most states (watchdogs, global errors) */
void Supervisor_RunStandardChecks(void);

/* INIT State */
void State_Init_OnEnter(void);
void State_Init_Run(void);
void State_Init_OnExit(void);

/* IDLE State */
void State_Idle_OnEnter(void);
void State_Idle_Run(void);
void State_Idle_OnExit(void);

/* MANUAL State */
void State_Manual_OnEnter(void);
void State_Manual_Run(void);
void State_Manual_OnExit(void);

/* AUTO State */
void State_Auto_OnEnter(void);
void State_Auto_Run(void);
void State_Auto_OnExit(void);

/* PAUSED State */
void State_Paused_OnEnter(void);
void State_Paused_Run(void);
void State_Paused_OnExit(void);

/* FAULT State */
void State_Fault_OnEnter(void);
void State_Fault_Run(void);
void State_Fault_OnExit(void);

#endif /* __STATE_HANDLERS_H */
