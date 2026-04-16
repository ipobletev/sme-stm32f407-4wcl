#include "app_rtos.h"
#include "bsp_led.h"

/**
 * @brief Heartbeat Timer Callback.
 * Toggles the status LED every 1 second to signify the system is alive.
 */
void HeartbeatTimerCallback(void *argument) {
    (void)argument;
    BSP_LED_Toggle(BSP_LED_USER);
}
