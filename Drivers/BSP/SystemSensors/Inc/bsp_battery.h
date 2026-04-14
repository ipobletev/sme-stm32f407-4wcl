#ifndef __BSP_BATTERY_H
#define __BSP_BATTERY_H

#include <stdint.h>

/**
 * @brief Get Battery Voltage in Volts.
 * @return float Battery voltage
 */
float BSP_Battery_GetVoltage(void);

/**
 * @brief Get Battery Current (Dummy for now).
 * @return float Battery current
 */
float BSP_Battery_GetCurrent(void);

#endif /* __BSP_BATTERY_H */
