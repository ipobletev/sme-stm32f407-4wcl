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
    PID_DEBUG: 0x85,
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
 * Binary Read Helpers
 */
const readFloat32 = (view, offset) => view.getFloat32(offset, true);
const readUint32 = (view, offset) => view.getUint32(offset, true);
const readInt32 = (view, offset) => view.getInt32(offset, true);
const readUint64 = (view, offset) => {
  try {
    return view.getBigUint64(offset, true);
  } catch (e) {
    const low = view.getUint32(offset, true);
    const high = view.getUint32(offset + 4, true);
    return (BigInt(high) << 32n) | BigInt(low);
  }
};

export function parsePayload(topicId, data) {
  const buffer = data.buffer.slice(data.byteOffset, data.byteOffset + data.byteLength);
  const view = new DataView(buffer);

  try {
    switch (topicId) {
      case TOPIC_IDS.TX.SYS_STATUS: // 0x81
        return {
          errors: readUint64(view, 0).toString(),
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
          /* roll, pitch, yaw removed from binary (calculate from Q if needed) */
        };
 
      case TOPIC_IDS.TX.ODOMETRY: {
        return {
          linear_x: view.getFloat32(0, true),
          angular_z: view.getFloat32(4, true),
          encoders: [
            view.getInt32(8, true),
            view.getInt32(12, true),
            view.getInt32(16, true),
            view.getInt32(20, true)
          ],
          measuredRps: [
            view.getFloat32(24, true), view.getFloat32(28, true),
            view.getFloat32(32, true), view.getFloat32(36, true)
          ]
          /* targetRps and pwmOutput moved to Topic 0x85 */
        };
      }
      
      case TOPIC_IDS.TX.PID_DEBUG: // 0x85
        return {
          targetRps: [
            view.getFloat32(0, true), view.getFloat32(4, true),
            view.getFloat32(8, true), view.getFloat32(12, true)
          ],
          pwmOutput: [
            view.getFloat32(16, true), view.getFloat32(20, true),
            view.getFloat32(24, true), view.getFloat32(28, true)
          ]
        };

      case TOPIC_IDS.TX.APP_CONFIG_DATA: // 0x84
        return {
          magic: readUint32(view, 0),
          debug_level: readUint32(view, 4),
          telemetry_period: readUint32(view, 8),
          sys_vars_period: readUint32(view, 12),
          imu_period: readUint32(view, 16),
          odom_period: readUint32(view, 20),
          pid_enabled: readUint32(view, 24),
          motor_ticks: readFloat32(view, 28),
          motor_rps_limit: readFloat32(view, 32),
          motor_pwm_max: readFloat32(view, 36),
          wheel_diameter: readFloat32(view, 40),
          shaft_width: readFloat32(view, 44),
          wheelbase_length: readFloat32(view, 48),
          motor1_inv: readInt32(view, 52),
          motor2_inv: readInt32(view, 56),
          motor3_inv: readInt32(view, 60),
          motor4_inv: readInt32(view, 64),
          motor1_kp: readFloat32(view, 68), motor1_ki: readFloat32(view, 72), motor1_kd: readFloat32(view, 76), motor1_deadzone: readFloat32(view, 80),
          motor2_kp: readFloat32(view, 84), motor2_ki: readFloat32(view, 88), motor2_kd: readFloat32(view, 92), motor2_deadzone: readFloat32(view, 96),
          motor3_kp: readFloat32(view, 100), motor3_ki: readFloat32(view, 104), motor3_kd: readFloat32(view, 108), motor3_deadzone: readFloat32(view, 112),
          motor4_kp: readFloat32(view, 116), motor4_ki: readFloat32(view, 120), motor4_kd: readFloat32(view, 124), motor4_deadzone: readFloat32(view, 128),
          mobility_mode: readUint32(view, 132),
          crc: readUint32(view, 136)
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
