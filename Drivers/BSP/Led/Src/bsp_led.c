#include "bsp_led.h"
#include "main.h"

void BSP_LED_SetState(BSP_LED_ID_t led, bool state) {
    if (led == BSP_LED_USER) {
        HAL_GPIO_WritePin(USER_LED_GPIO_Port, USER_LED_Pin, state ? GPIO_PIN_SET : GPIO_PIN_RESET);
    }
}

void BSP_LED_Toggle(BSP_LED_ID_t led) {
    if (led == BSP_LED_USER) {
        HAL_GPIO_TogglePin(USER_LED_GPIO_Port, USER_LED_Pin);
    }
}
