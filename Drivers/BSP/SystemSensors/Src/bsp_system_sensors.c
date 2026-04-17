#include "bsp_battery.h"
#include "bsp_mcu_sensors.h"
#include "adc.h"

/* --- Hardware Definitions --- */
#define ADC_CHANNELS            3       /* Batt, Vrefint, Tempsensor */
#define ADC_MAX_VALUE           4095.0f
#define VREFINT_VOLTAGE_CAL     3.3f    /* Calibration voltage for STM32F4 */

/* Battery Voltage Divider: R_high = 100k, R_low = 10k -> ratio = (100+10)/10 = 11 */
#define BATT_DIVIDER_RATIO      11.0f 

/* Factory Calibration Addresses for STM32F4 */
#ifndef VREFINT_CAL_ADDR
#define VREFINT_CAL_ADDR        ((uint16_t*)((uint32_t)0x1FFF7A2A))
#endif

#ifndef TS_CAL1_ADDR
#define TS_CAL1_ADDR            ((uint16_t*)((uint32_t)0x1FFF7A2C))
#endif

#ifndef TS_CAL2_ADDR
#define TS_CAL2_ADDR            ((uint16_t*)((uint32_t)0x1FFF7A2E))
#endif

/* --- Static Variables --- */
static uint16_t adc_dma_buffer[ADC_CHANNELS];
static float battery_volt_filtered = 0.0f;

/**
 * @brief Initialize all System Sensors (ADC-based).
 */
void BSP_SystemSensors_Init(void)
{
    /* Start ADC with DMA */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t*)adc_dma_buffer, ADC_CHANNELS);
}

/**
 * @brief Calculate real VREF based on factory calibration.
 */
static float GetRealVdda(void)
{
    if (adc_dma_buffer[1] == 0) return 3.3f;
    return VREFINT_VOLTAGE_CAL * ((float)(*VREFINT_CAL_ADDR) / (float)adc_dma_buffer[1]);
}

/**
 * @brief Get Battery Voltage.
 */
float BSP_Battery_GetVoltage(void)
{
    float vdda = GetRealVdda();
    float v_batt_pin = ((float)adc_dma_buffer[0] * vdda) / ADC_MAX_VALUE;
    float volt_now = v_batt_pin * BATT_DIVIDER_RATIO;

    /* First order filter: 5% new value, 95% previous */
    if (battery_volt_filtered == 0.0f) {
        battery_volt_filtered = volt_now;
    } else {
        battery_volt_filtered = (battery_volt_filtered * 0.85f) + (volt_now * 0.15f);
    }

    return battery_volt_filtered;
}

/**
 * @brief Get Battery Current (Dummy Not Implemented).
 */
float BSP_Battery_GetCurrent(void)
{
    return 0.0f;
}

/**
 * @brief Get MCU Internal Temperature in Celsius.
 */
float BSP_MCU_GetInternalTemp(void)
{
    float ts_cal1 = (float)(*TS_CAL1_ADDR);
    float ts_cal2 = (float)(*TS_CAL2_ADDR);
    float vdda = GetRealVdda();
    float ts_data = (float)adc_dma_buffer[2] * (vdda / 3.3f);

    if (ts_cal2 == ts_cal1) return 0.0f; 

    /* Temperature formula using factory calibration scaled to 3.3V */
    return ((110.0f - 30.0f) / (ts_cal2 - ts_cal1)) * (ts_data - ts_cal1) + 30.0f;
}

/**
 * @brief Get MCU Internal Reference Voltage.
 */
float BSP_MCU_GetInternalVref(void)
{
    return GetRealVdda();
}
