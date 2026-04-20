#include "bsp_serial_ros.h"
#include "usart.h"
#include <string.h>

#define SERIAL_ROS_RX_BUF_SIZE 512

static uint8_t rx_dma_buffer[SERIAL_ROS_RX_BUF_SIZE];
static uint16_t last_rx_pos = 0;
static SerialRos_RxCallback_t user_rx_callback = NULL;

extern UART_HandleTypeDef huart3;

void BSP_SerialRos_Init(SerialRos_RxCallback_t callback)
{
    user_rx_callback = callback;
    last_rx_pos = 0;

    /* Enable IDLE Line interrupt for USART3 */
    __HAL_UART_ENABLE_IT(&huart3, UART_IT_IDLE);

    /* Start DMA Reception in circular mode */
    /* NOTE: Ensure the DMA is configured in CIRCULAR mode in MX_DMA_Init */
    HAL_UART_Receive_DMA(&huart3, rx_dma_buffer, SERIAL_ROS_RX_BUF_SIZE);
}

HAL_StatusTypeDef BSP_SerialRos_Transmit(uint8_t *data, uint16_t size)
{
    /* Use the thread-safe/buffer-safe DMA write function */
    UART_DMA_Write(&huart3, data, size);
    return HAL_OK;
}

bool BSP_SerialRos_IsTxReady(void)
{
    /* Check if UART is ready for a new DMA transfer */
    return (huart3.gState == HAL_UART_STATE_READY);
}


/**
 * @brief This function handles USART3 global interrupt.
 * It detects the IDLE line to process incoming packets.
 */
void BSP_SerialRos_IRQHandler(void)
{
    if (__HAL_UART_GET_FLAG(&huart3, UART_FLAG_IDLE) != RESET)
    {
        /* Clear IDLE flag */
        __HAL_UART_CLEAR_IDLEFLAG(&huart3);

        /* Calculate current position in the circular buffer */
        uint16_t current_pos = SERIAL_ROS_RX_BUF_SIZE - __HAL_DMA_GET_COUNTER(huart3.hdmarx);
        
        if (user_rx_callback != NULL)
        {
            if (current_pos != last_rx_pos)
            {
                if (current_pos > last_rx_pos)
                {
                    /* Linear segment: [last_rx_pos ... current_pos-1] */
                    user_rx_callback(&rx_dma_buffer[last_rx_pos], current_pos - last_rx_pos);
                }
                else
                {
                    /* Wrapped segment: [last_rx_pos ... end] then [0 ... current_pos-1] */
                    user_rx_callback(&rx_dma_buffer[last_rx_pos], SERIAL_ROS_RX_BUF_SIZE - last_rx_pos);
                    if (current_pos > 0)
                    {
                        user_rx_callback(&rx_dma_buffer[0], current_pos);
                    }
                }
            }
        }
        
        last_rx_pos = current_pos;
    }
}
