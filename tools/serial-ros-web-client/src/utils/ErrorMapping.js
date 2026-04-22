/**
 * @file ErrorMapping.js
 * @brief Comprehensive mapping of 64-bit error indices to human-readable labels.
 * Based on Application/Core/Inc/error_codes.h
 */

export const ERROR_MAPPING = {
  // 1. RTOS Errors (0-7)
  0: { macro: 'ERR_RTOS_TASK', label: 'RTOS Task failure', severity: 'error' },
  1: { macro: 'ERR_RTOS_QUEUE', label: 'RTOS Queue failure', severity: 'error' },
  2: { macro: 'ERR_RTOS_TIMER', label: 'RTOS Timer failure', severity: 'error' },
  3: { macro: 'ERR_RTOS_MUTEX', label: 'RTOS Mutex failure', severity: 'error' },

  // 2. HAL Errors (8-23)
  8: { macro: 'ERR_HAL_ADC', label: 'Hardware: ADC failed', severity: 'error' },
  9: { macro: 'ERR_HAL_UART', label: 'Hardware: UART failed', severity: 'error' },
  10: { macro: 'ERR_HAL_I2C', label: 'Hardware: I2C failed', severity: 'error' },
  11: { macro: 'ERR_HAL_SPI', label: 'Hardware: SPI failed', severity: 'error' },
  12: { macro: 'ERR_HAL_FLASH', label: 'Hardware: Flash error', severity: 'error' },

  // 3. BSP Errors (24-31)
  24: { macro: 'ERR_BSP_LED', label: 'Component: LED init failed', severity: 'warning' },
  25: { macro: 'ERR_BSP_BUTTON', label: 'Component: Button init failed', severity: 'warning' },
  26: { macro: 'ERR_BSP_BUZZER', label: 'Component: Buzzer init failed', severity: 'warning' },
  27: { macro: 'ERR_BSP_SENSOR', label: 'Component: Sensor error', severity: 'error' },

  // 4. System Logic Errors (32-47)
  32: { macro: 'ERR_BATT_LOW', label: 'CRITICAL: Low Battery!', severity: 'fatal' },
  33: { macro: 'ERR_OVER_TEMP', label: 'CRITICAL: System Overheating!', severity: 'fatal' },
  34: { macro: 'ERR_HEARTBEAT', label: 'System: Task Heartbeat Lost', severity: 'error' },
  35: { macro: 'ERR_CONFIG_LOAD', label: 'System: Config Load Failed', severity: 'error' },

  36: { macro: 'ERR_MOB_DRIVE', label: 'Mobility: Hardware Drive Error', severity: 'error' },
  37: { macro: 'ERR_MOB_STALL', label: 'Mobility: Watchdog Stall', severity: 'fatal' },
  38: { macro: 'ERR_MOB_FAULT', label: 'Mobility: General Fault', severity: 'error' },
  39: { macro: 'ERR_INVALID_MOBILITY_EVENT', label: 'Mobility: Invalid Event', severity: 'warning' },

  40: { macro: 'ERR_ARM_FAULT', label: 'Arm: General Fault', severity: 'error' },
  41: { macro: 'ERR_ARM_SERVO', label: 'Arm: Servo Communication Failure', severity: 'error' },
  42: { macro: 'ERR_ARM_STALL', label: 'Arm: Watchdog Stall', severity: 'fatal' },
  43: { macro: 'ERR_INVALID_ARM_EVENT', label: 'Arm: Invalid Event', severity: 'warning' },

  44: { macro: 'ERR_SUPERVISOR_FAULT', label: 'Supervisor: System Fault', severity: 'error' },
  45: { macro: 'ERR_INVALID_SUPERVISOR_STATE', label: 'Supervisor: Invalid State', severity: 'error' },
  46: { macro: 'ERR_COMMS_LOST', label: 'Communication: Link Lost', severity: 'error' },

  // 5. Fatal (63)
  63: { macro: 'ERR_SYS_PANIC', label: 'FATAL: SYSTEM PANIC', severity: 'fatal' },
};

/**
 * Parses a 64-bit error flag and returns an array of human-readable objects.
 * @param {bigint|number|string} flags 
 */
export function getActiveErrors(flags) {
  if (flags === null || flags === undefined) return [];
  
  const bigFlags = typeof flags === 'bigint' ? flags : BigInt(flags || 0);
  if (bigFlags === 0n) return [];

  const active = [];
  for (let i = 0; i < 64; i++) {
    if ((bigFlags >> BigInt(i)) & 1n) {
      const info = ERROR_MAPPING[i];
      active.push({
        id: i,
        label: info ? info.macro : `BIT_${i}`,
        description: info ? info.label : `Unknown Error (Bit ${i})`,
        severity: info ? info.severity : 'error'
      });
    }
  }
  return active;
}
