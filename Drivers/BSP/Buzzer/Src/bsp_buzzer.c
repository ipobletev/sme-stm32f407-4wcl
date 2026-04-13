#include "bsp_buzzer.h"
#include "main.h"

void BSP_Buzzer_SetState(bool state) {
    HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
}

void BSP_Buzzer_Beep(uint32_t delay_ms) {
    BSP_Buzzer_SetState(true);
    HAL_Delay(delay_ms);
    BSP_Buzzer_SetState(false);
}
