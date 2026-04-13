#ifndef __BSP_BUZZER_H
#define __BSP_BUZZER_H

#include <stdint.h>
#include <stdbool.h>

void BSP_Buzzer_SetState(bool state);
void BSP_Buzzer_Beep(uint32_t delay_ms);

#endif /* __BSP_BUZZER_H */
