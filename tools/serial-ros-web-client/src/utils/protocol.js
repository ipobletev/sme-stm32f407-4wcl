/**
 * CRC16 Modbus (0xA001) Implementation
 * Matches the STM32 C implementation
 */
export function calculateCRC16(data) {
  let crc = 0xFFFF;
  for (let i = 0; i < data.length; i++) {
    crc ^= data[i];
    for (let j = 8; j !== 0; j--) {
      if ((crc & 0x0001) !== 0) {
        crc >>= 1;
        crc ^= 0xA001;
      } else {
        crc >>= 1;
      }
    }
  }
  return crc;
}

export const TOPIC_IDS = {
  RX: {
    HEARTBEAT: 0x00,
    AUTONOMOUS: 0x01,
    MOBILITY_MODE: 0x02,
    CMD_VEL: 0x03,
    ARM_GOAL: 0x04,
    SYS_EVENT: 0x05,
    ACTUATOR_PWM: 0x06,
    ACTUATOR_VEL: 0x07,
    SYS_EVENT_TEST: 0x07, // Added for testing transition
    SET_CONFIG: 0x08,
    GET_CONFIG: 0x09,
    SAVE_CONFIG: 0x0A,
  },
  TX: {
    SYS_STATUS: 0x81,
    IMU: 0x82,
    ODOMETRY: 0x83,
    APP_CONFIG_DATA: 0x84,
  }
};

/**
 * Builds a SerialRos binary packet
 * Format: [SYNC1, SYNC2, ID, LEN, DATA..., CRC_L, CRC_H]
 */
export function buildPacket(topicId, payload = []) {
  const sync1 = 0xAA;
  const sync2 = 0x55;
  const len = payload.length;
  
  const header = [sync1, sync2, topicId, len];
  const packetWithoutCrc = new Uint8Array([...header, ...payload]);
  
  const crc = calculateCRC16(packetWithoutCrc);
  const crcL = crc & 0xFF;
  const crcH = (crc >> 8) & 0xFF;
  
  const finalPacket = new Uint8Array([...packetWithoutCrc, crcL, crcH]);
  return finalPacket;
}

/**
 * Parses a Float32 from a buffer (Little Endian)
 */
function readFloat32(view, offset) {
  return view.getFloat32(offset, true);
}

/**
 * Parses a Uint64 (BigInt) from a buffer (Little Endian)
 */
function readUint64(view, offset) {
  return view.getBigUint64(offset, true);
}

/**
 * Parses an Int32 from a buffer (Little Endian)
 */
function readInt32(view, offset) {
  return view.getInt32(offset, true);
}

export function parsePayload(topicId, data) {
  const buffer = data.buffer.slice(data.byteOffset, data.byteOffset + data.byteLength);
  const view = new DataView(buffer);

  try {
    switch (topicId) {
      case TOPIC_IDS.TX.SYS_STATUS: // 0x81
        return {
          errors: readUint64(view, 0),
          temp: readFloat32(view, 8),
          v_batt: readFloat32(view, 12),
          state: view.getUint8(16),
          mobility_state: view.getUint8(17),
          arm_state: view.getUint8(18),
          mobility_mode: view.getUint8(19)
        };

      case TOPIC_IDS.TX.IMU: // 0x82
        return {
          qx: readFloat32(view, 0),
          qy: readFloat32(view, 4),
          qz: readFloat32(view, 8),
          qw: readFloat32(view, 12),
          gyro: { x: readFloat32(view, 16), y: readFloat32(view, 20), z: readFloat32(view, 24) },
          accel: { x: readFloat32(view, 28), y: readFloat32(view, 32), z: readFloat32(view, 36) },
          roll: readFloat32(view, 40),
          pitch: readFloat32(view, 44),
          yaw: readFloat32(view, 48)
        };

      case TOPIC_IDS.TX.ODOMETRY: // 0x83
        return {
          linear_x: readFloat32(view, 0),
          angular_z: readFloat32(view, 4),
          encoders: [
            readInt32(view, 8),
            readInt32(view, 12),
            readInt32(view, 16),
            readInt32(view, 20)
          ]
        };
      
      case TOPIC_IDS.TX.APP_CONFIG_DATA: // 0x84
        return {
          magic: view.getUint32(0, true),
          debug_level: view.getUint32(4, true),
          telemetry_period: view.getUint32(8, true),
          sys_vars_period: view.getUint32(12, true),
          imu_period: view.getUint32(16, true),
          odom_period: view.getUint32(20, true),
          pid_enabled: view.getUint32(24, true),
          motor_ticks: readFloat32(view, 28),
          motor_rps_limit: readFloat32(view, 32),
          motor_deadzone: readFloat32(view, 36),
          motor_pwm_max: readFloat32(view, 40),
          wheel_diameter: readFloat32(view, 44),
          shaft_width: readFloat32(view, 48),
          wheelbase_length: readFloat32(view, 52),
          motor1_inv: view.getInt32(56, true),
          motor2_inv: view.getInt32(60, true),
          motor3_inv: view.getInt32(64, true),
          motor4_inv: view.getInt32(68, true),
          crc: view.getUint32(72, true)
        };

      default:
        return null;
    }
  } catch (e) {
    console.error(`Error parsing payload for topic 0x${topicId.toString(16)}:`, e);
    return null;
  }
}

/**
 * Encoders for Rx topics
 */
export const Encoders = {
  autonomous: (isAuto) => new Uint8Array([isAuto ? 1 : 0]),
  
  mobilityMode: (mode, isAuto) => new Uint8Array([mode, isAuto ? 1 : 0]),
  
  cmdVel: (linearX, angularZ) => {
    const buf = new ArrayBuffer(8);
    const view = new DataView(buf);
    view.setFloat32(0, linearX, true);
    view.setFloat32(4, angularZ, true);
    return new Uint8Array(buf);
  },
  
  armGoal: (j1, j2, j3) => {
    const buf = new ArrayBuffer(12);
    const view = new DataView(buf);
    view.setFloat32(0, j1, true);
    view.setFloat32(4, j2, true);
    view.setFloat32(8, j3, true);
    return new Uint8Array(buf);
  },
  
  sysEvent: (eventId) => new Uint8Array([eventId]),
  
  actuatorTest: (id, pulse) => {
    const buf = new ArrayBuffer(5);
    const view = new DataView(buf);
    view.setUint8(0, id);
    view.setFloat32(1, pulse, true);
    return new Uint8Array(buf);
  },

  actuatorVel: (id, rps) => {
    const buf = new ArrayBuffer(5);
    const view = new DataView(buf);
    view.setUint8(0, id);
    view.setFloat32(1, rps, true);
    return new Uint8Array(buf);
  },

  setConfig: (id, value) => {
    const buf = new ArrayBuffer(5);
    const view = new DataView(buf);
    view.setUint8(0, id);
    view.setFloat32(1, value, true);
    return new Uint8Array(buf);
  }
};
