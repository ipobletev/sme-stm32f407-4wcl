#ifndef __BSP_BUZZER_H
#define __BSP_BUZZER_H

#include <stdint.h>
#include <stdbool.h>

void BSP_Buzzer_Init(void);
void BSP_Buzzer_On(void);
void BSP_Buzzer_Off(void);
void BSP_Buzzer_Toggle(void);

#endif /* __BSP_BUZZER_H */
