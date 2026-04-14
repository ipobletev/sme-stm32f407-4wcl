#ifndef __STATE_HANDLERS_H
#define __STATE_HANDLERS_H

#include "supervisor_fsm.h"

/* State Handler Interface */

/* INIT State */
void State_Init_OnEnter(void);
void State_Init_OnExit(void);

/* IDLE State */
void State_Idle_OnEnter(void);
void State_Idle_OnExit(void);

/* MANUAL State */
void State_Manual_OnEnter(void);
void State_Manual_OnExit(void);

/* AUTO State */
void State_Auto_OnEnter(void);
void State_Auto_OnExit(void);

/* PAUSED State */
void State_Paused_OnEnter(void);
void State_Paused_OnExit(void);

/* FAULT State */
void State_Fault_OnEnter(void);
void State_Fault_OnExit(void);

#endif /* __STATE_HANDLERS_H */

