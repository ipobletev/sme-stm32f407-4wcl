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
    AUTONOMOUS: 0x01,
    MOBILITY_MODE: 0x02,
    CMD_VEL: 0x03,
    ARM_GOAL: 0x04,
    SYS_EVENT: 0x05,
  },
  TX: {
    SYS_STATUS: 0x81,
    IMU: 0x82,
    ODOMETRY: 0x83,
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
          state: view.getUint8(0),
          errors: readUint64(view, 1),
          temp: readFloat32(view, 9),
          v_batt: readFloat32(view, 13),
          i_batt: readFloat32(view, 17)
        };

      case TOPIC_IDS.TX.IMU: // 0x82
        return {
          roll: readFloat32(view, 0),
          pitch: readFloat32(view, 4),
          yaw: readFloat32(view, 8),
          gyro: { x: readFloat32(view, 12), y: readFloat32(view, 16), z: readFloat32(view, 20) },
          accel: { x: readFloat32(view, 24), y: readFloat32(view, 28), z: readFloat32(view, 32) }
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
  
  sysEvent: (eventId) => new Uint8Array([eventId])
};
