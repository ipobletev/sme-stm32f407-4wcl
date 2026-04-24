#include "app_rtos.h"
#include "bsp_button.h"
#include "debug_module.h"
#include <stdio.h>
#include <string.h>
#include "supervisor_fsm.h"
#include "robot_state.h"

#define LOG_TAG "HW_INPUT"

void StartHWInputTask(void *argument)
{
    LOG_INFO(LOG_TAG, "Hardware Input Task Started (Safety Focus).\r\n");

    bool k1_prev = false;
    bool k2_prev = false;
    bool sw3_prev = false;

    for(;;)
    {
        /* 
         * Periodical polling for buttons (100ms interval)
         */
        osal_delay(100);

        /* Read Current Button States through BSP */
        bool k1_pressed = BSP_Button_GetState(BSP_BTN_K1);
        bool k2_pressed = BSP_Button_GetState(BSP_BTN_K2);
        bool sw3_pressed = BSP_Button_GetState(BSP_BTN_SW3);

        /* 1. K1: Emergency Stop */
        if (k1_pressed && !k1_prev)
        {
            LOG_ERROR(LOG_TAG, "PHYSICAL E-STOP (K1) TRIGGERED!\r\n");
            Supervisor_SendEvent(EVENT_SUPERVISOR_ERROR, SRC_PHYSICAL_CRITICAL);
        }
        k1_prev = k1_pressed;

        /* 2. K2: System Reset */
        if (k2_pressed && !k2_prev)
        {
            LOG_INFO(LOG_TAG, "PHYSICAL RESET (K2) TRIGGERED\r\n");
            Supervisor_SendEvent(EVENT_SUPERVISOR_RESET, SRC_PHYSICAL);
        }
        k2_prev = k2_pressed;

        /* 3. SW3: Autonomous Mode Permissivity */
        /* If SW3 is ON (pressed in this case), allow Auto Mode */
        if (sw3_pressed != sw3_prev) {
            RobotState_SetAutoPermissivity(sw3_pressed);
            if (sw3_pressed) {
                LOG_INFO(LOG_TAG, "Autonomous Mode: ENABLED (Permissivity ON)\r\n");
            } else {
                LOG_WARNING(LOG_TAG, "Autonomous Mode: ISOLATED (Permissivity OFF)\r\n");
            }
        }
        sw3_prev = sw3_pressed;

        /* K2 can remain as a generic PAUSE button or be disabled */
        if (k2_pressed) {
             /* Optional: generic pause logic */
        }
    }
}
