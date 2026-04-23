#include "bsp_buzzer.h"
#include "main.h"
#include "osal.h"

void BSP_Buzzer_On(void) {
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
}

void BSP_Buzzer_Off(void) {
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
}

void BSP_Buzzer_Toggle(void) {
    HAL_GPIO_TogglePin(BUZZER_GPIO_Port, BUZZER_Pin);
}
