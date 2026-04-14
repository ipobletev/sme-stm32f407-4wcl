#include "app_rtos.h"
#include <stdio.h>

/**
 * @brief  Function implementing the defaultTask thread.
 * @param  argument: Not used
 */
void StartDefaultTask(void *argument)
{
  printf("Application Default Task Started (RTOS Layer)\r\n");
  for(;;)
  {
    /* Keep task running, can be used for low priority background tasks or watchdog feeding */
    osal_delay(5000);
  }
}

