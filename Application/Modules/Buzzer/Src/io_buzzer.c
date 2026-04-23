#include "io_buzzer.h"
#include "bsp_buzzer.h"
#include "osal.h"
#include "debug_module.h"

#define BEEP_ON_DURATION_MS 100

static osal_timer_h buzzerTimerHandle;
static uint16_t current_freq = 0;
static uint16_t current_period_ms = 0;
static uint32_t elapsed_ms = 0;

static void buzzerTimerCallback(void *argument) {
    (void)argument;
    
    if (current_freq == 0) {
        BSP_Buzzer_Off();
        return;
    }

    uint32_t toggle_interval = 1000 / (2 * current_freq);
    if (toggle_interval == 0) toggle_interval = 1;

    if (current_period_ms == 0) {
        /* Continuous mode */
        BSP_Buzzer_Toggle();
    } else {
        /* Intermittent mode */
        if (elapsed_ms < BEEP_ON_DURATION_MS) {
            BSP_Buzzer_Toggle();
        } else {
            BSP_Buzzer_Off();
        }
        
        elapsed_ms += toggle_interval;
        if (elapsed_ms >= current_period_ms) {
            elapsed_ms = 0;
        }
    }
}

// IO initialization
void io_buzzer_init(void) {
    BSP_Buzzer_Off();

    /* Create a periodic timer (1ms) for tone generation (500Hz) */
    buzzerTimerHandle = osal_timer_create(buzzerTimerCallback, OSAL_TIMER_PERIODIC, NULL);
    if (buzzerTimerHandle == NULL) {
        LOG_ERROR("IO_Buzzer", "Failed to create buzzer timer\r\n");
    }
    osal_timer_stop(buzzerTimerHandle);
}

// Buzzer control
void io_buzzer(uint16_t freq, uint16_t period_ms) {
    if (buzzerTimerHandle != NULL) {
        osal_timer_stop(buzzerTimerHandle);
        
        current_freq = freq;
        current_period_ms = period_ms;
        elapsed_ms = 0;

        if (freq > 0) {
            uint32_t toggle_interval = 1000 / (2 * freq);
            if (toggle_interval == 0) toggle_interval = 1;
            
            BSP_Buzzer_On();
            osal_timer_start(buzzerTimerHandle, toggle_interval);
        } else {
            BSP_Buzzer_Off();
        }
    }
}

// Stop buzzer
void io_buzzer_stop(void) {
    if (buzzerTimerHandle != NULL) {
        osal_timer_stop(buzzerTimerHandle);
    }
    BSP_Buzzer_Off();
}