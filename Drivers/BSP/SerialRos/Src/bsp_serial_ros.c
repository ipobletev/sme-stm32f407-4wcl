#include "bsp_serial_ros.h"
#include "usart.h"
#include <string.h>

#define SERIAL_ROS_RX_BUF_SIZE 256

static uint8_t rx_dma_buffer[SERIAL_ROS_RX_BUF_SIZE];
static SerialRos_RxCallback_t user_rx_callback = NULL;

extern UART_HandleTypeDef huart3;

void BSP_SerialRos_Init(SerialRos_RxCallback_t callback)
{
    user_rx_callback = callback;

    /* Enable IDLE Line interrupt for USART3 */
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);

    /* Start DMA Reception in circular mode */
    HAL_UART_Receive_DMA(&huart3, rx_dma_buffer, SERIAL_ROS_RX_BUF_SIZE);
}

HAL_StatusTypeDef BSP_SerialRos_Transmit(uint8_t *data, uint16_t size)
{
    /* Use the thread-safe/buffer-safe DMA write function */
    UART_DMA_Write(&huart3, data, size);
    return HAL_OK;
}


/**
 * @brief This function handles USART3 global interrupt.
 * It detects the IDLE line to process incoming packets.
 */
void USART3_IRQHandler(void)
{
    if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_IDLE) != RESET)
    {
        /* Clear IDLE flag by reading status then data */
        __HAL_UART_CLEAR_IDLEFLAG(&huart3);

        /* Calculate packet size */
        uint16_t rx_pos = SERIAL_ROS_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart3.hdmarx);
        
        if (user_rx_callback != NULL && rx_pos > 0)
        {
            /* Trigger callback with data */
            user_rx_callback(rx_dma_buffer, rx_pos);
        }

        /* Reset DMA for next packet */
        HAL_UART_DMAStop(&huart3);
        HAL_UART_Receive_DMA(&huart3, rx_dma_buffer, SERIAL_ROS_RX_BUF_SIZE);
    }
    HAL_UART_IRQHandler(&huart3);
}
