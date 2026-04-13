#include "app_rtos.h"
#include "bsp_button.h"
#include "debug_module.h"
#include <stdio.h>
#include <string.h>

#define LOG_TAG "CONTROLLER"

/* Helper to publish events */
static void publish_event(SystemEvent_t event)
{
    StateChangeMsg_t msg;
    msg.event = event;
    msg.timestamp = osKernelGetTickCount();
    
    osStatus_t status = osMessageQueuePut(stateMsgQueueHandle, &msg, 0U, 0U);
    if (status != osOK) {
        LOG_ERROR(LOG_TAG, "Failed to publish event %d", event);
    }
}

void StartControllerTask(void *argument)
{
    LOG_INFO(LOG_TAG, "Controller Task Started.");

    bool k1_prev = false;
    bool k2_prev = false;
    bool sw3_prev = false;

    for(;;)
    {
        StateChangeMsg_t uart_msg;
        /* 
         * Wait for events from UART Listener with a 100ms timeout.
         * This timeout acts as our polling interval for buttons.
         */
        osStatus_t status = osMessageQueueGet(uartEventQueueHandle, &uart_msg, NULL, 100U);
        
        if (status == osOK)
        {
            LOG_INFO(LOG_TAG, "Centralized Event Received from UART: %d", uart_msg.event);
            /* Here you could add logic to filter or modify events before forwarding */
            publish_event(uart_msg.event);
        }

        /* Read Current Button States through BSP */
        bool k1_pressed = BSP_Button_GetState(BSP_BTN_K1);
        bool k2_pressed = BSP_Button_GetState(BSP_BTN_K2);
        bool sw3_pressed = BSP_Button_GetState(BSP_BTN_SW3);

        /* K1 corresponds to START event */
        if (k1_pressed && !k1_prev)
        {
            LOG_INFO(LOG_TAG, "K1 pressed, publishing EVENT_START");
            publish_event(EVENT_START);
        }
        k1_prev = k1_pressed;

        /* K2 corresponds to STOP event */
        if (k2_pressed && !k2_prev)
        {
            LOG_INFO(LOG_TAG, "K2 pressed, publishing EVENT_STOP");
            publish_event(EVENT_STOP);
        }
        k2_prev = k2_pressed;

        /* SW3 corresponds to ERROR or RESET */
        if (sw3_pressed && !sw3_prev)
        {
            static uint8_t error_flag = 0;
            if (!error_flag) {
                LOG_INFO(LOG_TAG, "SW3 pressed, publishing EVENT_ERROR");
                publish_event(EVENT_ERROR);
                error_flag = 1;
            } else {
                LOG_INFO(LOG_TAG, "SW3 pressed, publishing EVENT_RESET");
                publish_event(EVENT_RESET);
                error_flag = 0;
            }
        }
        sw3_prev = sw3_pressed;

        /* 
         * Note: Removed explicit osDelay(100) because osMessageQueueGet 
         * provides the timing (blocking up to 100ms).
         */
    }
}
