#ifndef __BSP_MCU_SENSORS_H
#define __BSP_MCU_SENSORS_H

#include <stdint.h>

/**
 * @brief Initialize all System Sensors (ADC-based).
 */
void BSP_SystemSensors_Init(void);

/**
 * @brief Get MCU Internal Temperature in Celsius.
 * @return float Temperature
 */
float BSP_MCU_GetInternalTemp(void);

/**
 * @brief Get MCU Internal Reference Voltage.
 * @return float Vref
 */
float BSP_MCU_GetInternalVref(void);

#endif /* __BSP_MCU_SENSORS_H */
