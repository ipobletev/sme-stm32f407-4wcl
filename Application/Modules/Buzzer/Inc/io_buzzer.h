#ifndef __IO_BUZZER_H
#define __IO_BUZZER_H

#include <stdint.h>

/**
 * @brief Initialize the Buzzer IO module.
 * Creates the RTOS timer for buzzer control.
 */
void io_buzzer_init(void);

/**
 * @brief Activate the buzzer with specific frequency and pattern.
 * @param freq Frequency of oscillation (Hz). Use 0 for silence.
 * @param period_ms Cycle period in milliseconds. Use 0 for continuous sound.
 */
void io_buzzer(uint16_t freq, uint16_t period_ms);


/**
 * @brief Stop the buzzer.
 */
void io_buzzer_stop(void);

/**
 * @brief Activate the buzzer with a specific period.
 * @param period_ms Period in milliseconds.
 */
void io_buzzer_start(uint16_t period_ms);

#endif /* __IO_BUZZER_H */
