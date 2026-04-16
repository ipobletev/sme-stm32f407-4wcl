#include "osal.h"
#include "cmsis_os2.h"

/* --- System --- */
uint32_t osal_get_tick(void) {
    return osKernelGetTickCount();
}

void osal_delay(uint32_t ms) {
    osDelay(ms);
}

void osal_delay_until(uint32_t *previous_wake_tick, uint32_t ms) {
    osDelayUntil(*previous_wake_tick + ms);
    *previous_wake_tick = osKernelGetTickCount();
}

/* --- Threads --- */

static osPriority_t map_priority(osal_priority_t prio) {
    switch(prio) {
        case OSAL_PRIO_LOW:      return osPriorityLow;
        case OSAL_PRIO_NORMAL:   return osPriorityNormal;
        case OSAL_PRIO_HIGH:     return osPriorityHigh;
        case OSAL_PRIO_REALTIME: return osPriorityRealtime;
        default:                 return osPriorityNormal;
    }
}

osal_thread_h osal_thread_create(void (*func)(void *), void *arg, const osal_thread_attr_t *attr) {
    osThreadAttr_t cmsis_attr = {0};
    if (attr) {
        cmsis_attr.name = attr->name;
        cmsis_attr.stack_size = attr->stack_size;
        cmsis_attr.priority = map_priority(attr->priority);
    }
    return (osal_thread_h)osThreadNew(func, arg, attr ? &cmsis_attr : NULL);
}

osal_thread_h osal_thread_get_self(void) {
    return (osal_thread_h)osThreadGetId();
}

osal_status_t osal_thread_flags_set(osal_thread_h thread, uint32_t flags) {
    uint32_t ret = osThreadFlagsSet((osThreadId_t)thread, flags);
    return (ret & 0x80000000U) ? OSAL_ERROR : OSAL_OK;
}

uint32_t osal_thread_flags_wait(uint32_t flags, uint32_t timeout_ms) {
    return osThreadFlagsWait(flags, osFlagsWaitAny, timeout_ms);
}

uint32_t osal_thread_get_stack_space(osal_thread_h thread) {
    return osThreadGetStackSpace((osThreadId_t)thread);
}


/* --- Queues --- */

osal_queue_h osal_queue_create(uint32_t count, uint32_t item_size) {
    return (osal_queue_h)osMessageQueueNew(count, item_size, NULL);
}

osal_status_t osal_queue_put(osal_queue_h queue, const void *item, uint32_t timeout_ms) {
    osStatus_t status = osMessageQueuePut((osMessageQueueId_t)queue, item, 0U, timeout_ms);
    return (status == osOK) ? OSAL_OK : OSAL_ERROR;
}

osal_status_t osal_queue_get(osal_queue_h queue, void *item, uint32_t timeout_ms) {
    osStatus_t status = osMessageQueueGet((osMessageQueueId_t)queue, item, NULL, timeout_ms);
    if (status == osOK) return OSAL_OK;
    if (status == osErrorTimeout) return OSAL_TIMEOUT;
    return OSAL_ERROR;
}

/* --- Timers --- */

osal_timer_h osal_timer_create(void (*func)(void *), osal_timer_type_t type, void *arg) {
    osTimerType_t cmsis_type = (type == OSAL_TIMER_PERIODIC) ? osTimerPeriodic : osTimerOnce;
    return (osal_timer_h)osTimerNew((osTimerFunc_t)func, cmsis_type, arg, NULL);
}

osal_status_t osal_timer_start(osal_timer_h timer, uint32_t period_ms) {
    osStatus_t status = osTimerStart((osTimerId_t)timer, period_ms);
    return (status == osOK) ? OSAL_OK : OSAL_ERROR;
}
