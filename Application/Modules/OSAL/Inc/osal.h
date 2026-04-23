#ifndef __OSAL_H
#define __OSAL_H

#include <stdint.h>
#include <stddef.h>

/**
 * @brief OSAL Status codes
 */
typedef enum {
    OSAL_OK = 0,
    OSAL_ERROR,
    OSAL_TIMEOUT,
    OSAL_NOMEM
} osal_status_t;

/**
 * @brief OSAL Priority levels
 */
typedef enum {
    OSAL_PRIO_LOW = 0,
    OSAL_PRIO_NORMAL,
    OSAL_PRIO_HIGH,
    OSAL_PRIO_REALTIME
} osal_priority_t;

/**
 * @brief Opaque handles for OS objects
 */
typedef void* osal_thread_h;
typedef void* osal_queue_h;
typedef void* osal_timer_h;

/**
 * @brief Thread Attributes
 */
typedef struct {
    const char* name;
    uint32_t stack_size;
    osal_priority_t priority;
} osal_thread_attr_t;

#define OSAL_WAIT_FOREVER (0xFFFFFFFFU)

/* --- System --- */
uint32_t osal_get_tick(void);
void osal_delay(uint32_t ms);
void osal_delay_until(uint32_t *previous_wake_tick, uint32_t ms);

/* --- Threads --- */
osal_thread_h osal_thread_create(void (*func)(void *), void *arg, const osal_thread_attr_t *attr);
osal_thread_h osal_thread_get_self(void);
osal_status_t osal_thread_flags_set(osal_thread_h thread, uint32_t flags);
uint32_t osal_thread_flags_wait(uint32_t flags, uint32_t timeout_ms);
uint32_t osal_thread_get_stack_space(osal_thread_h thread);

/* --- Queues --- */
osal_queue_h osal_queue_create(uint32_t count, uint32_t item_size);
osal_status_t osal_queue_put(osal_queue_h queue, const void *item, uint32_t timeout_ms);
osal_status_t osal_queue_get(osal_queue_h queue, void *item, uint32_t timeout_ms);

/* --- Timers --- */
typedef enum {
    OSAL_TIMER_ONCE = 0,
    OSAL_TIMER_PERIODIC
} osal_timer_type_t;

osal_timer_h osal_timer_create(void (*func)(void *), osal_timer_type_t type, void *arg);
osal_status_t osal_timer_start(osal_timer_h timer, uint32_t period_ms);
osal_status_t osal_timer_stop(osal_timer_h timer);

#endif /* __OSAL_H */
