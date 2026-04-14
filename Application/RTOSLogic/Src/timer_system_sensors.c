#include "app_rtos.h"
#include "robot_state.h"
#include "bsp_battery.h"
#include "bsp_mcu_sensors.h"

/**
 * @brief  System Sensors Timer Callback (Board Health Monitoring).
 * Runs periodically to sample battery voltage and MCU internal diagnostics.
 * @param  argument: Not used
 */
void SystemSensorsTimerCallback(void *argument)
{
    /* Read from BSPs (Hardware Layer) */
    float batt_v = BSP_Battery_GetVoltage();
    float batt_i = BSP_Battery_GetCurrent();
    float mcu_t  = BSP_MCU_GetInternalTemp();

    /* Update Robot State (Data Layer) */
    RobotState_SetBatteryVoltage(batt_v);
    RobotState_SetBatteryCurrent(batt_i);
    RobotState_SetUCTemperature(mcu_t);
}
