#include "app_rtos.h"
#include "robot_state.h"
#include "supervisor_fsm.h"
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

    /* Update Robot State Status */
    RobotState_UpdateSystemState(Supervisor_GetCurrentState());
    RobotState_IncrementHeartbeat();

    /* Report Status via DEBUG COM with Stack Diagnostics */
    uint32_t stack_uart = osal_thread_get_stack_space(uartListenerTaskHandle);
    uint32_t stack_ctrl = osal_thread_get_stack_space(controllerTaskHandle);

    uint64_t errs = RobotState_GetErrorFlags();
    char err_str[19];
    /* Manually format to 0x + 16 hex chars to avoid %llX library issues */
    snprintf(err_str, sizeof(err_str), "0x%08lX%08lX", 
             (unsigned long)(errs >> 32), (unsigned long)(errs & 0xFFFFFFFF));

    LOG_INFO(LOG_TAG, "HB: %lu | State: [SUP:%s MOB:%s ARM:%s] | Batt: %.2fV | MCU: %.1fC | Errors: %s | FreeStack: [CTRL:%lu UART:%lu]\n", 
           (unsigned long)RobotState_GetHeartbeat(),
           Supervisor_StateToStr(RobotState_GetSystemState()),
           Mobility_StateToStr(RobotState_GetMobilityState()),
           Arm_StateToStr(RobotState_GetArmState()),
           RobotState_4wcl.Telemetry.battery_voltage,
           RobotState_4wcl.Telemetry.uc_temperature,
           err_str,
           (unsigned long)stack_ctrl, (unsigned long)stack_uart);
}
