#ifndef __BSP_CONSOLE_H
#define __BSP_CONSOLE_H

#include "main.h"

/**
 * @brief Callback type for console events.
 */
typedef void (*BSP_Console_Callback_t)(uint16_t size);

/**
 * @brief Sends data through the debug console (UART1).
 * @param data Pointer to the data buffer.
 * @param len Length of the data to send.
 * @return HAL_StatusTypeDef status of the operation.
 */
HAL_StatusTypeDef BSP_Console_Send(uint8_t *data, uint16_t len);

/**
 * @brief Initializes the console for DMA reception with Idle detection.
 * @param callback Function to call when data is received.
 */
void BSP_Console_InitRx(BSP_Console_Callback_t callback);

/**
 * @brief Get received data from the internal BSP buffer.
 * @param dest Destination buffer.
 * @param max_len Maximum length to copy.
 * @return uint16_t Actual length copied.
 */
uint16_t BSP_Console_GetData(uint8_t *dest, uint16_t max_len);

/**
 * @brief Restarts the DMA reception.
 */
void BSP_Console_AcceptNext(void);

/**
 * @brief Container for Console packets in RTOS queues
 */
typedef struct {
    uint8_t data[256];
    uint16_t size;
} Console_Packet_t;

#endif /* __BSP_CONSOLE_H */
