#ifndef __STATE_HANDLERS_H
#define __STATE_HANDLERS_H

#include "app_state_machine.h"

/* State Handler Interface */

/* INIT State */
void State_Init_OnEnter(void);
void State_Init_OnExit(void);

/* IDLE State */
void State_Idle_OnEnter(void);
void State_Idle_OnExit(void);

/* ACTIVE State */
void State_Active_OnEnter(void);
void State_Active_OnExit(void);

/* FAULT State */
void State_Fault_OnEnter(void);
void State_Fault_OnExit(void);

#endif /* __STATE_HANDLERS_H */
