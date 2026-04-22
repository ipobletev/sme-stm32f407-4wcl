#ifndef __ERROR_CODES_H
#define __ERROR_CODES_H

#include <stdint.h>

/**
 * @brief 64-bit System Error Flags
 * Each flag represents a single bit in a uint64_t variable.
 * Use 1ULL to ensure the constant is treated as 64-bit.
 */

/* 1. RTOS Errors (Bits 0-7) */
#define ERR_RTOS_TASK    (1ULL << 0)   /* Logic tasks creation failure */
#define ERR_RTOS_QUEUE   (1ULL << 1)   /* Message queues creation failure */
#define ERR_RTOS_TIMER   (1ULL << 2)   /* Software timers creation failure */
#define ERR_RTOS_MUTEX   (1ULL << 3)   /* Mutex/Semaphore creation failure */

/* 2. Hardware / HAL Errors (Bits 8-23) */
#define ERR_HAL_ADC      (1ULL << 8)   /* ADC configuration/start failure */
#define ERR_HAL_UART     (1ULL << 9)   /* UART DMA/IT initialization failure */
#define ERR_HAL_I2C      (1ULL << 10)  /* I2C bus error */
#define ERR_HAL_SPI      (1ULL << 11)  /* SPI bus error */
#define ERR_HAL_FLASH    (1ULL << 12)  /* Flash memory access error */

/* 3. BSP / Peripheral Errors (Bits 24-31) */
#define ERR_BSP_LED      (1ULL << 24)  /* LED GPIO initialization failure */
#define ERR_BSP_BUTTON   (1ULL << 25)  /* Button GPIO initialization failure */
#define ERR_BSP_BUZZER   (1ULL << 26)  /* Buzzer GPIO initialization failure */
#define ERR_BSP_SENSOR   (1ULL << 27)  /* External sensor communication failure */

/* 4. System / Business Logic Errors (Bits 32-47) */
#define ERR_BATT_LOW                        (1ULL << 32)  /* Battery voltage below critical threshold */
#define ERR_OVER_TEMP                       (1ULL << 33)  /* System temperature critical */
#define ERR_HEARTBEAT                       (1ULL << 34)  /* Internal task heartbeat lost */
#define ERR_CONFIG_LOAD                     (1ULL << 35)  /* Configuration parameters loading failure */

#define ERR_MOB_DRIVE                       (1ULL << 36)  /* Mobility local hardware drive error */
#define ERR_MOB_STALL                       (1ULL << 37)  /* Mobility task watchdog stall */
#define ERR_MOB_FAULT                       (1ULL << 38)  /* Robotic arm task watchdog stall */
#define ERR_INVALID_MOBILITY_EVENT          (1ULL << 39)  /* Robotic arm task watchdog stall */

#define ERR_ARM_FAULT                       (1ULL << 40)  /* Robotic arm task watchdog stall */
#define ERR_ARM_SERVO                       (1ULL << 41)  /* Robotic arm servo failure */
#define ERR_ARM_STALL                       (1ULL << 42)  /* Robotic arm task watchdog stall */
#define ERR_INVALID_ARM_EVENT               (1ULL << 43)  /* Robotic arm task watchdog stall */

#define ERR_SUPERVISOR_FAULT                (1ULL << 44)  /* Robotic arm task watchdog stall */
#define ERR_INVALID_SUPERVISOR_STATE        (1ULL << 45)  /* Robotic arm task watchdog stall */
#define ERR_COMMS_LOST                      (1ULL << 46)  /* Remote client communication lost in critical mode */

/* 5. Critical / Fatal Errors (Bits 48-63) */
#define ERR_SYS_PANIC    (1ULL << 63)  /* Unrecoverable system panic */

/**
 * @brief Utility Macros for 64-bit Error Management
 */
#define ERR_CLEAR_ALL(target)        ((target) = 0ULL)
#define ERR_SET(target, flag)       ((target) |= (flag))
#define ERR_CLEAR(target, flag)     ((target) &= ~(flag))
#define ERR_HAS(target, flag)       (((target) & (flag)) != 0ULL)
#define ERR_IS_ANY(target)           ((target) != 0ULL)

#endif /* __ERROR_CODES_H */
