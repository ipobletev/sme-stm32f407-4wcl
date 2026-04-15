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

    /* Manually format floats to avoid %f library issues */
    float batt = RobotState_4wcl.Telemetry.battery_voltage;
    int batt_i = (int)batt;
    int batt_f = (int)((batt - (float)batt_i) * 100.0f);
    if (batt_f < 0) batt_f = -batt_f;

    float temp = RobotState_4wcl.Telemetry.uc_temperature;
    int temp_i = (int)temp;
    int temp_f = (int)((temp - (float)temp_i) * 10.0f);
    if (temp_f < 0) temp_f = -temp_f;

    LOG_INFO(LOG_TAG, "HB: %lu | State: [SUP:%s MOB:%s ARM:%s] | Batt: %d.%02dV | MCU: %d.%01dC | Errors: %s | FreeStack: [CTRL:%lu UART:%lu]\r\n", 
           (unsigned long)RobotState_GetHeartbeat(),
           Supervisor_StateToStr(RobotState_GetSystemState()),
           Mobility_StateToStr(RobotState_GetMobilityState()),
           Arm_StateToStr(RobotState_GetArmState()),
           batt_i, batt_f,
           temp_i, temp_f,
           err_str,
           (unsigned long)stack_ctrl, (unsigned long)stack_uart);
}
