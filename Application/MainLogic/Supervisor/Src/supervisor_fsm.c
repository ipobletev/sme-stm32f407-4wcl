#include "supervisor_fsm.h"
#include "States/state_handlers.h"
#include "arm_fsm.h"
#include "robot_state.h"
#include <stdio.h>

const char* Supervisor_StateToStr(SystemState_t state) {
    switch(state) {
        case STATE_INIT:   return "INIT";
        case STATE_IDLE:   return "IDLE";
        case STATE_MANUAL: return "MANUAL";
        case STATE_AUTO:   return "AUTO";
        case STATE_PAUSED: return "PAUSED";
        case STATE_FAULT:  return "FAULT";
        default:           return "UNKNOWN";
    }
}

/* Internal Private State */
static SystemState_t currentState = STATE_INIT;
static SystemState_t previousMode = STATE_IDLE; /* To restore after Pause */
static uint8_t pauseAuthLevel = 0;              /* Authority level that triggered the pause */

/**
 * @brief Helper to handle state transitions with entry/exit actions.
 */
static void TransitionToState(SystemState_t newState) {
    if (currentState == newState) return;

    /* Call Exit Handler of current state */
    switch (currentState) {
        case STATE_INIT:   State_Init_OnExit();   break;
        case STATE_IDLE:   State_Idle_OnExit();   break;
        case STATE_MANUAL: State_Manual_OnExit(); break;
        case STATE_AUTO:   State_Auto_OnExit();   break;
        case STATE_PAUSED: State_Paused_OnExit(); break;
        case STATE_FAULT:  State_Fault_OnExit();  break;
        default: break;
    }

    currentState = newState;
    RobotState_UpdateSystemState(currentState);

    /* Call Enter Handler of new state */
    switch (currentState) {
        case STATE_INIT:   State_Init_OnEnter();   break;
        case STATE_IDLE:   State_Idle_OnEnter();   break;
        case STATE_MANUAL: State_Manual_OnEnter(); break;
        case STATE_AUTO:   State_Auto_OnEnter();   break;
        case STATE_PAUSED: State_Paused_OnEnter(); break;
        case STATE_FAULT:  State_Fault_OnEnter();  break;
        default: break;
    }
}

/**
 * @brief Initialize the supervisor state machine.
 */
void Supervisor_Init(void) {
    currentState = STATE_INIT;
    RobotState_UpdateSystemState(currentState);
    State_Init_OnEnter();
    printf("Supervisor: Initialized\r\n");
}

/**
 * @brief Get the current state.
 */
SystemState_t Supervisor_GetCurrentState(void) {
    return currentState;
}

/**
 * @brief Process an incoming event and update the supervisor state machine.
 */
void Supervisor_ProcessEvent(SystemEvent_t event, uint8_t source) {
    SystemState_t nextState = currentState;

    /* Handle Non-Transitioning Global Events */
    if (event == EVENT_REHOME) {
        if (currentState == STATE_IDLE || currentState == STATE_MANUAL || currentState == STATE_AUTO) {
            Arm_RequestRehome();
        }
        return; /* Event consumed, no state change */
    }

    /* Transition Table Logic */
    switch (currentState)
    {
        case STATE_INIT:
            if (event == EVENT_START) {
                nextState = STATE_IDLE;
            }
            break;

        case STATE_IDLE:
            if (event == EVENT_START || event == EVENT_MODE_MANUAL) {
                nextState = STATE_MANUAL;
            } else if (event == EVENT_MODE_AUTO) {
                nextState = STATE_AUTO;
            } else if (event == EVENT_ERROR) {
                nextState = STATE_FAULT;
            }
            break;
        
        case STATE_MANUAL:
            if (event == EVENT_STOP) {
                nextState = STATE_IDLE;
            } else if (event == EVENT_PAUSE) {
                previousMode = STATE_MANUAL;
                pauseAuthLevel = source;
                nextState = STATE_PAUSED;
            } else if (event == EVENT_MODE_AUTO) {
                nextState = STATE_AUTO;
            } else if (event == EVENT_ERROR) {
                nextState = STATE_FAULT;
            }
            break;

        case STATE_AUTO:
            if (event == EVENT_STOP) {
                nextState = STATE_IDLE;
            } else if (event == EVENT_PAUSE) {
                previousMode = STATE_AUTO;
                pauseAuthLevel = source;
                nextState = STATE_PAUSED;
            } else if (event == EVENT_MODE_MANUAL) {
                nextState = STATE_MANUAL;
            } else if (event == EVENT_ERROR) {
                nextState = STATE_FAULT;
            }
            break;

        case STATE_PAUSED:
            if (event == EVENT_STOP) {
                nextState = STATE_IDLE; // Global override
            } else if (event == EVENT_RESUME) {
                // Hierarchical Authority Check
                if (source >= pauseAuthLevel) {
                    nextState = previousMode; // Restore where we were
                } else {
                    printf("Supervisor: Resume REJECTED. Source (%d) lower than Auth (%d)\r\n", source, pauseAuthLevel);
                }
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

