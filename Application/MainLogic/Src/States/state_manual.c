#include "States/state_handlers.h"
#include "bsp_led.h"
#include "control_board.h"
#include <stdio.h>

void State_Manual_OnEnter(void) {
    printf("SM: Entering MANUAL State (Operator Control)\r\n");
    ControlBoard_4wcl.is_autonomous = 0;
    /* Manual Mode: Solid LED or steady feedback */
    BSP_LED_SetState(BSP_LED_USER, true);
}


void State_Manual_OnExit(void) {
    printf("SM: Exiting MANUAL State\r\n");
    BSP_LED_SetState(BSP_LED_USER, false);
}
