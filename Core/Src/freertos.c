/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>
#include "usart.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 3000 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN StartDefaultTask */
  uint8_t k1_prev = GPIO_PIN_SET;
  uint8_t k2_prev = GPIO_PIN_SET;
  uint8_t sw3_prev = GPIO_PIN_SET;

  /* Infinite loop */
  printf("UART1 DMA (backed) Test OK - System Started\r\n");
  for(;;)
  {
    /* Read Current Button States */
    GPIO_PinState k1_state = HAL_GPIO_ReadPin(USER_K1_BUTTON_GPIO_Port, USER_K1_BUTTON_Pin);
    GPIO_PinState k2_state = HAL_GPIO_ReadPin(USER_K2_BUTTON_GPIO_Port, USER_K2_BUTTON_Pin);
    GPIO_PinState sw3_state = HAL_GPIO_ReadPin(USER_SW3_GPIO_Port, USER_SW3_Pin);

    /* USER_K1 Logic: Toggle LED on press */
    if (k1_state == GPIO_PIN_RESET && k1_prev == GPIO_PIN_SET)
    {
       HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
       printf("K1 Pressed: LED Toggled - Counter: %lu\r\n", (unsigned long)xTaskGetTickCount());
    }
    k1_prev = k1_state;

    /* USER_K2 Logic: Short beep on press */
    if (k2_state == GPIO_PIN_RESET && k2_prev == GPIO_PIN_SET)
    {
       HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
       osDelay(100);
       HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
       printf("K2 Pressed: Buzzer Beep - Tick: %lu\r\n", (unsigned long)osKernelGetTickCount());
    }
    k2_prev = k2_state;

    /* USER_SW3 Logic: Double beep on press */
    if (sw3_state == GPIO_PIN_RESET && sw3_prev == GPIO_PIN_SET)
    {
       for(int i=0; i<2; i++) {
         HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
         osDelay(50);
         HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
         if(i==0) osDelay(50);
       }
       printf("SW3 Pressed: Double Beep\r\n");
       
       /* UART3 DMA Test - Direct HAL usage */
       UART_Printf_DMA(&huart3, "UART3 DMA View: SW3 Pressed at %lu ms\r\n", (unsigned long)osKernelGetTickCount());
    }
    sw3_prev = sw3_state;

    osDelay(50); /* Poll every 50ms */
  }
  /* USER CODE END StartDefaultTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

