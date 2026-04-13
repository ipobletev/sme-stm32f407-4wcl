#include "bsp_button.h"
#include "main.h"

bool BSP_Button_GetState(BSP_BTN_ID_t btn) {
    GPIO_PinState state = GPIO_PIN_SET;
    
    switch (btn) {
        case BSP_BTN_K1:
            state = HAL_GPIO_ReadPin(USER_K1_BUTTON_GPIO_Port, USER_K1_BUTTON_Pin);
            break;
        case BSP_BTN_K2:
            state = HAL_GPIO_ReadPin(USER_K2_BUTTON_GPIO_Port, USER_K2_BUTTON_Pin);
            break;
        case BSP_BTN_SW3:
            state = HAL_GPIO_ReadPin(USER_SW3_GPIO_Port, USER_SW3_Pin);
            break;
        default:
            break;
    }
    
    return (state == GPIO_PIN_RESET);
}
