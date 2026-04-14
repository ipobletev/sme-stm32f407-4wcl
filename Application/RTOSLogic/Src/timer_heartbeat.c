#include "app_rtos.h"
#include "control_board.h"
#include "bsp_led.h"
#include "debug_module.h"
#include <stdio.h>

#define LOG_TAG "HEARTBEAT"

/**
 * @brief  Heartbeat Timer Callback (Soft-Timer).
 * @param  argument: Not used
 */
void HeartbeatTimerCallback(void *argument)
{
    /* Toggle User LED */
    BSP_LED_Toggle(BSP_LED_USER);

    /* Update Control Board Status */
    ControlBoard_4wcl.current_state = SM_GetCurrentState();
    ControlBoard_4wcl.heartbeat_count++;

    /* Report Status via DEBUG COM with Stack Diagnostics */
    uint32_t stack_uart = osThreadGetStackSpace(uartListenerTaskHandle);
    uint32_t stack_ctrl = osThreadGetStackSpace(controllerTaskHandle);
    uint32_t stack_def  = osThreadGetStackSpace(defaultTaskHandle);

    LOG_INFO(LOG_TAG, "HB: %lu | State: %d | Errors: 0x%02lX | FreeStack: [CTRL:%lu UART:%lu DEF:%lu]", 
           (unsigned long)ControlBoard_4wcl.heartbeat_count,
           (int)ControlBoard_4wcl.current_state,
           (unsigned long)ControlBoard_4wcl.error_flags,
           (unsigned long)stack_ctrl, (unsigned long)stack_uart, (unsigned long)stack_def);
}
