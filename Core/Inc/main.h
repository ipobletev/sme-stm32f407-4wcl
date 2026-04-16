/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define MOTOR3_FI_TIM9_CH1_Pin GPIO_PIN_5
#define MOTOR3_FI_TIM9_CH1_GPIO_Port GPIOE
#define MOTOR3_BI_TIM9_CH2_Pin GPIO_PIN_6
#define MOTOR3_BI_TIM9_CH2_GPIO_Port GPIOE
#define ENC1_A_TIM5_CH1_Pin GPIO_PIN_0
#define ENC1_A_TIM5_CH1_GPIO_Port GPIOA
#define ENC1_B_TIM5_CH2_Pin GPIO_PIN_1
#define ENC1_B_TIM5_CH2_GPIO_Port GPIOA
#define V_BATT_SENSE_Pin GPIO_PIN_0
#define V_BATT_SENSE_GPIO_Port GPIOB
#define SERIAL_SERVO_TX_EN_Pin GPIO_PIN_7
#define SERIAL_SERVO_TX_EN_GPIO_Port GPIOE
#define SERIAL_SERVO_RX_EN_Pin GPIO_PIN_8
#define SERIAL_SERVO_RX_EN_GPIO_Port GPIOE
#define MOTOR2_BI_TIM1_CH1_Pin GPIO_PIN_9
#define MOTOR2_BI_TIM1_CH1_GPIO_Port GPIOE
#define USER_LED_Pin GPIO_PIN_10
#define USER_LED_GPIO_Port GPIOE
#define MOTOR2_FI_TIM1_CH2_Pin GPIO_PIN_11
#define MOTOR2_FI_TIM1_CH2_GPIO_Port GPIOE
#define MOTOR_1_BI_TIM1_CH3_Pin GPIO_PIN_13
#define MOTOR_1_BI_TIM1_CH3_GPIO_Port GPIOE
#define MOTOR_1_FI_TIM1_CH4_Pin GPIO_PIN_14
#define MOTOR_1_FI_TIM1_CH4_GPIO_Port GPIOE
#define IMU_SCL_Pin GPIO_PIN_10
#define IMU_SCL_GPIO_Port GPIOB
#define IMU_SDA_Pin GPIO_PIN_11
#define IMU_SDA_GPIO_Port GPIOB
#define IMU_ITR_Pin GPIO_PIN_12
#define IMU_ITR_GPIO_Port GPIOB
#define SERIAL_SERVO_UART6_TX_Pin GPIO_PIN_6
#define SERIAL_SERVO_UART6_TX_GPIO_Port GPIOC
#define SERIAL_SERVO_UART6_RX_Pin GPIO_PIN_7
#define SERIAL_SERVO_UART6_RX_GPIO_Port GPIOC
#define BUZZER_Pin GPIO_PIN_8
#define BUZZER_GPIO_Port GPIOA
#define ENC2_A_TIM2_CH1_Pin GPIO_PIN_15
#define ENC2_A_TIM2_CH1_GPIO_Port GPIOA
#define USER_SW3_Pin GPIO_PIN_3
#define USER_SW3_GPIO_Port GPIOD
#define ENC2_B_TIM2_CH2_Pin GPIO_PIN_3
#define ENC2_B_TIM2_CH2_GPIO_Port GPIOB
#define ENC4_A_TIM3_CH1_Pin GPIO_PIN_4
#define ENC4_A_TIM3_CH1_GPIO_Port GPIOB
#define ENC4_B_TIM3_CH2_Pin GPIO_PIN_5
#define ENC4_B_TIM3_CH2_GPIO_Port GPIOB
#define ENC3_A_TIM4_CH1_Pin GPIO_PIN_6
#define ENC3_A_TIM4_CH1_GPIO_Port GPIOB
#define ENC3_B_TIM4_CH2_Pin GPIO_PIN_7
#define ENC3_B_TIM4_CH2_GPIO_Port GPIOB
#define MOTOR4_BI_TIM10_CH1_Pin GPIO_PIN_8
#define MOTOR4_BI_TIM10_CH1_GPIO_Port GPIOB
#define MOTOR4_FI_TIM11_CH1_Pin GPIO_PIN_9
#define MOTOR4_FI_TIM11_CH1_GPIO_Port GPIOB
#define USER_K2_BUTTON_Pin GPIO_PIN_0
#define USER_K2_BUTTON_GPIO_Port GPIOE
#define USER_K1_BUTTON_Pin GPIO_PIN_1
#define USER_K1_BUTTON_GPIO_Port GPIOE

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
