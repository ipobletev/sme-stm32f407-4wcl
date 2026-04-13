#include "app_state_machine.h"
#include "bsp_led.h"
#include <stdio.h>

/* Internal Private State */
static SystemState_t currentState = STATE_INIT;

/**
 * @brief Initialize the state machine.
 */
void SM_Init(void) {
    currentState = STATE_INIT;
    printf("SM: Initialized to STATE_INIT\r\n");
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
    /* State Machine Transition Logic */
    switch (currentState)
    {
        case STATE_INIT:
            if (event == EVENT_START) {
                currentState = STATE_IDLE;
                printf("SM: Transition INIT -> IDLE\r\n");
            }
            break;

        case STATE_IDLE:
            if (event == EVENT_START) {
                currentState = STATE_ACTIVE;
                printf("SM: Transition IDLE -> ACTIVE\r\n");
                BSP_LED_SetState(BSP_LED_USER, true);
            } else if (event == EVENT_ERROR) {
                currentState = STATE_FAULT;
                printf("SM: Transition IDLE -> FAULT\r\n");
            }
            break;

        case STATE_ACTIVE:
            if (event == EVENT_STOP) {
                currentState = STATE_IDLE;
                printf("SM: Transition ACTIVE -> IDLE\r\n");
                BSP_LED_SetState(BSP_LED_USER, false);
            } else if (event == EVENT_ERROR) {
                currentState = STATE_FAULT;
                printf("SM: Transition ACTIVE -> FAULT\r\n");
                BSP_LED_SetState(BSP_LED_USER, false);
            }
            break;

        case STATE_FAULT:
            if (event == EVENT_RESET) {
                currentState = STATE_INIT;
                printf("SM: Transition FAULT -> INIT (Reset)\r\n");
            }
            break;

        default:
            currentState = STATE_INIT;
            break;
    }
}
