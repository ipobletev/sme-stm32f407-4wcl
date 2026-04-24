#include "app_rtos.h"
#include "bsp_button.h"
#include "debug_module.h"
#include <stdio.h>
#include <string.h>
#include "supervisor_fsm.h"

#define LOG_TAG "CONTROLLER"


void StartControllerTask(void *argument)
{
    LOG_INFO(LOG_TAG, "Controller Task Started.\r\n");

    bool k1_prev = false;
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

        /* K1 corresponds to START or RESUME event (Defaulting to MANUAL) */
        if (k1_pressed && !k1_prev)
        {
            if (Supervisor_GetCurrentState() == STATE_SUPERVISOR_PAUSED) {
                LOG_INFO(LOG_TAG, "K1 pressed, requesting RESUME\r\n");
                Supervisor_SendEvent(EVENT_SUPERVISOR_RESUME, SRC_PHYSICAL);
            } else {
                LOG_INFO(LOG_TAG, "K1 pressed, requesting MANUAL mode\r\n");
                Supervisor_SendEvent(EVENT_SUPERVISOR_START, SRC_PHYSICAL);
            }
        }
        k1_prev = k1_pressed;

        /* K2 corresponds to PAUSE or STOP event (Timer logic) */
        static uint32_t k2_press_ticks = 0;
        if (k2_pressed) {
            k2_press_ticks++;
            if (k2_press_ticks == 10) { /* 10 ticks * 100ms = 1 second */
                LOG_INFO(LOG_TAG, "K2 SAFETY long press (>1s), E-STOP triggered\r\n");
                Supervisor_SendEvent(EVENT_SUPERVISOR_STOP, SRC_PHYSICAL);
            }
        } else {
            if (k2_press_ticks > 0 && k2_press_ticks < 10) {
                LOG_INFO(LOG_TAG, "K2 short press, PAUSE requested\r\n");
                Supervisor_SendEvent(EVENT_SUPERVISOR_PAUSE, SRC_PHYSICAL);
            }
            k2_press_ticks = 0;
        }


        /* SW3 corresponds to ERROR or RESET */
        if (sw3_pressed && !sw3_prev)
        {
            static uint8_t error_flag = 0;
            if (!error_flag) {
                LOG_INFO(LOG_TAG, "SW3 pressed, publishing EVENT_ERROR\r\n");
                Supervisor_SendEvent(EVENT_SUPERVISOR_ERROR, SRC_PHYSICAL);
                error_flag = 1;
            } else {
                LOG_INFO(LOG_TAG, "SW3 pressed, publishing EVENT_RESET\r\n");
                Supervisor_SendEvent(EVENT_SUPERVISOR_RESET, SRC_PHYSICAL);
                error_flag = 0;
            }
        }

        sw3_prev = sw3_pressed;
    }
}

