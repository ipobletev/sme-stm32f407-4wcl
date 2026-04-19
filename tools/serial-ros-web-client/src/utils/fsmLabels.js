/** FSM state code → label (must match firmware enums). */

export const SUPERVISOR_STATE_NAMES = ['INIT', 'IDLE', 'MANUAL', 'AUTO', 'PAUSED', 'FAULT', 'TESTING'];
export const MOBILITY_STATE_NAMES = ['INIT', 'IDLE', 'BREAK', 'MOVING', 'TESTING', 'FAULT', 'ABORT'];
export const ARM_STATE_NAMES = ['INIT', 'HOMING', 'IDLE', 'MOVING', 'TESTING', 'FAULT', 'ABORT'];

export const SUPERVISOR_STATE_CLASSES = ['state-idle', 'state-ready', 'state-running', 'state-running', 'state-paused', 'state-fault', 'state-running'];
export const MOBILITY_STATE_CLASSES = ['state-idle', 'state-ready', 'state-paused', 'state-running', 'state-running', 'state-fault', 'state-paused'];
export const ARM_STATE_CLASSES = ['state-idle', 'state-ready', 'state-ready', 'state-running', 'state-running', 'state-fault', 'state-paused'];

export function getSupervisorStateName(code) {
  return SUPERVISOR_STATE_NAMES[code] ?? `UNK(${code})`;
}

export function getSupervisorStateClass(code) {
  return SUPERVISOR_STATE_CLASSES[code] ?? 'state-idle';
}

export function getMobilityStateName(code) {
  return MOBILITY_STATE_NAMES[code] ?? `UNK(${code})`;
}

export function getMobilityStateClass(code) {
  return MOBILITY_STATE_CLASSES[code] ?? 'state-idle';
}

export function getArmStateName(code) {
  return ARM_STATE_NAMES[code] ?? `UNK(${code})`;
}

export function getArmStateClass(code) {
  return ARM_STATE_CLASSES[code] ?? 'state-idle';
}

/** @param {bigint | number | string} v */
export function formatErrorMask(v) {
  const b = typeof v === 'bigint' ? v : BigInt(v ?? 0);
  return '0x' + b.toString(16).toUpperCase().padStart(16, '0');
}
