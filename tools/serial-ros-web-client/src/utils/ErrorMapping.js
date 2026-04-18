/**
 * @file ErrorMapping.js
 * @brief Comprehensive mapping of 64-bit error indices to human-readable labels.
 * Based on Application/Core/Inc/error_codes.h
 */

export const ERROR_MAPPING = {
  // 1. RTOS Errors (0-7)
  0: { label: 'RTOS Task failure', severity: 'error' },
  1: { label: 'RTOS Queue failure', severity: 'error' },
  2: { label: 'RTOS Timer failure', severity: 'error' },
  3: { label: 'RTOS Mutex failure', severity: 'error' },

  // 2. HAL Errors (8-23)
  8: { label: 'Hardware: ADC failed', severity: 'error' },
  9: { label: 'Hardware: UART failed', severity: 'error' },
  10: { label: 'Hardware: I2C failed', severity: 'error' },
  11: { label: 'Hardware: SPI failed', severity: 'error' },
  12: { label: 'Hardware: Flash error', severity: 'error' },

  // 3. BSP Errors (24-31)
  24: { label: 'Component: LED init failed', severity: 'warning' },
  25: { label: 'Component: Button init failed', severity: 'warning' },
  26: { label: 'Component: Buzzer init failed', severity: 'warning' },
  27: { label: 'Component: Sensor error', severity: 'error' },

  // 4. System Logic Errors (32-47)
  32: { label: 'CRITICAL: Low Battery!', severity: 'fatal' },
  33: { label: 'CRITICAL: System Overheating!', severity: 'fatal' },
  34: { label: 'System: Task Heartbeat Lost', severity: 'error' },
  35: { label: 'System: Config Load Failed', severity: 'error' },

  36: { label: 'Mobility: Hardware Drive Error', severity: 'error' },
  37: { label: 'Mobility: Watchdog Stall', severity: 'fatal' },
  38: { label: 'Mobility: General Fault', severity: 'error' },
  39: { label: 'Mobility: Invalid Event', severity: 'warning' },

  40: { label: 'Arm: General Fault', severity: 'error' },
  41: { label: 'Arm: Servo Communication Failure', severity: 'error' },
  42: { label: 'Arm: Watchdog Stall', severity: 'fatal' },
  43: { label: 'Arm: Invalid Event', severity: 'warning' },

  44: { label: 'Supervisor: System Fault', severity: 'error' },
  45: { label: 'Supervisor: Invalid State', severity: 'error' },

  // 5. Fatal (63)
  63: { label: 'FATAL: SYSTEM PANIC', severity: 'fatal' },
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
        label: info ? info.label : `Unknown Error (Bit ${i})`,
        severity: info ? info.severity : 'error'
      });
    }
  }
  return active;
}
