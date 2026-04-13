#ifndef __BSP_LED_H
#define __BSP_LED_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    BSP_LED_USER = 0
} BSP_LED_ID_t;

void BSP_LED_SetState(BSP_LED_ID_t led, bool state);
void BSP_LED_Toggle(BSP_LED_ID_t led);

#endif /* __BSP_LED_H */
