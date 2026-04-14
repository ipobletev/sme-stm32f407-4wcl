#include "app_state_machine.h"
#include "States/state_handlers.h"
#include <stdio.h>

/* Internal Private State */
static SystemState_t currentState = STATE_INIT;

/**
 * @brief Helper to handle state transitions with entry/exit actions.
 */
static void TransitionToState(SystemState_t newState) {
    if (currentState == newState) return;

    /* Call Exit Handler of current state */
    switch (currentState) {
        case STATE_INIT:   State_Init_OnExit();   break;
        case STATE_IDLE:   State_Idle_OnExit();   break;
        case STATE_ACTIVE: State_Active_OnExit(); break;
        case STATE_FAULT:  State_Fault_OnExit();  break;
        default: break;
    }

    currentState = newState;

    /* Call Enter Handler of new state */
    switch (currentState) {
        case STATE_INIT:   State_Init_OnEnter();   break;
        case STATE_IDLE:   State_Idle_OnEnter();   break;
        case STATE_ACTIVE: State_Active_OnEnter(); break;
        case STATE_FAULT:  State_Fault_OnEnter();  break;
        default: break;
    }
}

/**
 * @brief Initialize the state machine.
 */
void SM_Init(void) {
    currentState = STATE_INIT;
    State_Init_OnEnter();
    printf("SM: Initialized\r\n");
}

/**
 * @brief Get the current state.
 */
SystemState_t SM_GetCurrentState(void) {
    return currentState;
}

/**
 * @brief Process an incoming event and update the state machine.
 */
void SM_ProcessEvent(SystemEvent_t event) {
    SystemState_t nextState = currentState;

    /* Transition Table Logic */
    switch (currentState)
    {
        case STATE_INIT:
            if (event == EVENT_START) {
                nextState = STATE_IDLE;
            }
            break;

        case STATE_IDLE:
            if (event == EVENT_START) {
                nextState = STATE_ACTIVE;
            } else if (event == EVENT_ERROR) {
                nextState = STATE_FAULT;
            }
            break;

        case STATE_ACTIVE:
            if (event == EVENT_STOP) {
                nextState = STATE_IDLE;
            } else if (event == EVENT_ERROR) {
                nextState = STATE_FAULT;
            }
            break;

        case STATE_FAULT:
            if (event == EVENT_RESET) {
                nextState = STATE_INIT;
            }
            break;

        default:
            nextState = STATE_INIT;
            break;
    }

    /* Execute transition if state changed */
    if (nextState != currentState) {
        TransitionToState(nextState);
    }
}

