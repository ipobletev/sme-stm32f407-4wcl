#ifndef __BSP_BUTTON_H
#define __BSP_BUTTON_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    BSP_BTN_K1 = 0,
    BSP_BTN_K2,
    BSP_BTN_SW3
} BSP_BTN_ID_t;

bool BSP_Button_GetState(BSP_BTN_ID_t btn);

#endif /* __BSP_BUTTON_H */
