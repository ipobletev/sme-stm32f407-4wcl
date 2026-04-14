#include "bsp_console.h"
#include "usart.h"
#include "robot_state.h"
#include <string.h>

#define CONSOLE_RX_BUF_SIZE 128
static uint8_t console_rx_buf[CONSOLE_RX_BUF_SIZE];
static BSP_Console_Callback_t rx_callback = NULL;

HAL_StatusTypeDef BSP_Console_Send(uint8_t *data, uint16_t len) {
    if (data == NULL || len == 0) {
        return HAL_ERROR;
    }
    
    // Use the existing DMA-based transmission
    UART_DMA_Write(&huart1, data, len);
    
    return HAL_OK;
}

void BSP_Console_InitRx(BSP_Console_Callback_t callback) {
    rx_callback = callback;
    
    memset(console_rx_buf, 0, CONSOLE_RX_BUF_SIZE);
    if (HAL_UARTEx_ReceiveToIdle_DMA(&huart1, console_rx_buf, CONSOLE_RX_BUF_SIZE) != HAL_OK) {
        RobotState_SetErrorFlag(ERR_HAL_UART);
    }
    
    /* Disable Half Transfer Interrupt to only get event on Idle or Full buffer */
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
}


uint16_t BSP_Console_GetData(uint8_t *dest, uint16_t max_len) {
    if (dest == NULL) return 0;
    
    uint16_t len = (max_len < CONSOLE_RX_BUF_SIZE) ? max_len : CONSOLE_RX_BUF_SIZE;
    memcpy(dest, console_rx_buf, len);
    return len;
}

void BSP_Console_AcceptNext(void) {
    memset(console_rx_buf, 0, CONSOLE_RX_BUF_SIZE);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, console_rx_buf, CONSOLE_RX_BUF_SIZE);
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
}

/**
 * @brief HAL UART Rx Event Callback (DMA + IDLE)
 * This sits in the BSP layer now.
 */
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart->Instance == USART1)
    {
        if (rx_callback != NULL) {
            rx_callback(Size);
        }
    }
}
