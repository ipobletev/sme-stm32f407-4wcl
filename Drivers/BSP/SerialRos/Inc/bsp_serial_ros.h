#ifndef __BSP_SERIAL_ROS_H
#define __BSP_SERIAL_ROS_H

#include "stm32f4xx_hal.h"
#include <stdint.h>
#include <stdbool.h>

/**
 * @brief Callback type for SerialRos Rx events
 * @param data Pointer to the received buffer
 * @param size Size of the received data
 */
typedef void (*SerialRos_RxCallback_t)(uint8_t *data, uint16_t size);

/**
 * @brief Initialize USART3 for Serial ROS communication (Jetson)
 * Configures DMA and IDLE line detection.
 * @param callback Function to call when a packet or IDLE line is detected
 */
void BSP_SerialRos_Init(SerialRos_RxCallback_t callback);

/**
 * @brief Send raw binary data over USART3 using DMA
 * @param data Pointer to data buffer
 * @param size Data size
 * @return HAL_StatusTypeDef
 */
HAL_StatusTypeDef BSP_SerialRos_Transmit(uint8_t *data, uint16_t size);
void BSP_SerialRos_IRQHandler(void);
bool BSP_SerialRos_IsTxReady(void);

#endif /* __BSP_SERIAL_ROS_H */
