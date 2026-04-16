import { useState, useRef, useCallback } from 'react';
import { calculateCRC16, parsePayload, TOPIC_IDS } from '../utils/protocol';

const SYNC1 = 0xAA;
const SYNC2 = 0x55;
const MIN_FRAME = 6;

/**
 * Frequency tracker: counts messages per topic and computes Hz every second.
 */
class FrequencyTracker {
  constructor() {
    this.counters = {};
    this.rates = {};
    this._interval = null;
  }

  start() {
    this.stop();
    this._interval = setInterval(() => {
      for (const key in this.counters) {
        this.rates[key] = this.counters[key];
        this.counters[key] = 0;
      }
    }, 1000);
  }

  stop() {
    if (this._interval) clearInterval(this._interval);
    this.counters = {};
    this.rates = {};
  }

  tick(topicId) {
    const key = `0x${topicId.toString(16).padStart(2, '0')}`;
    this.counters[key] = (this.counters[key] || 0) + 1;
    if (this.rates[key] === undefined) this.rates[key] = 0;
  }

  getRates() {
    return { ...this.rates };
  }
}

/**
 * State‐machine parser for the serial stream.
 * Handles byte-by-byte accumulation across chunk boundaries.
 */
class FrameParser {
  constructor(onFrame) {
    this.onFrame = onFrame;
    this.state = 'SYNC1';
    this.id = 0;
    this.len = 0;
    this.payload = [];
    this.idx = 0;
    this.crcBuf = [];
  }

  feed(byte) {
    switch (this.state) {
      case 'SYNC1':
        if (byte === SYNC1) this.state = 'SYNC2';
        break;
      case 'SYNC2':
        if (byte === SYNC2) this.state = 'ID';
        else this.state = 'SYNC1';
        break;
      case 'ID':
        this.id = byte;
        this.state = 'LEN';
        break;
      case 'LEN':
        this.len = byte;
        this.payload = [];
        this.idx = 0;
        if (this.len === 0) {
          this.crcBuf = [];
          this.state = 'CRC';
        } else {
          this.state = 'DATA';
        }
        break;
      case 'DATA':
        this.payload.push(byte);
        this.idx++;
        if (this.idx >= this.len) {
          this.crcBuf = [];
          this.state = 'CRC';
        }
        break;
      case 'CRC':
        this.crcBuf.push(byte);
        if (this.crcBuf.length >= 2) {
          this._validate();
          this.state = 'SYNC1';
        }
        break;
    }
  }

  _validate() {
    const header = [SYNC1, SYNC2, this.id, this.len];
    const pktData = new Uint8Array([...header, ...this.payload]);
    const calcCrc = calculateCRC16(pktData);
    const rxCrc = this.crcBuf[0] | (this.crcBuf[1] << 8);
    if (calcCrc === rxCrc) {
      this.onFrame(this.id, new Uint8Array(this.payload));
    }
  }
}

export function useSerial() {
  const [connected, setConnected] = useState(false);
  const [telemetry, setTelemetry] = useState({
    sysStatus: null,
    imu: null,
    odometry: null,
  });
  const [frequencies, setFrequencies] = useState({});
  const [log, setLog] = useState([]);

  const portRef = useRef(null);
  const readerRef = useRef(null);
  const writerRef = useRef(null);
  const readLoopRef = useRef(false);
  const freqTrackerRef = useRef(new FrequencyTracker());
  const freqIntervalRef = useRef(null);

  const addLog = useCallback((dir, topicId, raw, parsed) => {
    const entry = {
      ts: Date.now(),
      dir,
      topicId,
      raw: Array.from(raw).map(b => b.toString(16).padStart(2, '0')).join(' '),
      parsed,
    };
    setLog(prev => [entry, ...prev].slice(0, 200));
  }, []);

  const handleFrame = useCallback((topicId, data) => {
    freqTrackerRef.current.tick(topicId);
    const parsed = parsePayload(topicId, data);
    if (!parsed) return;

    addLog('RX', topicId, data, parsed);

    switch (topicId) {
      case TOPIC_IDS.TX.SYS_STATUS:
        setTelemetry(prev => ({ ...prev, sysStatus: parsed }));
        break;
      case TOPIC_IDS.TX.IMU:
        setTelemetry(prev => ({ ...prev, imu: parsed }));
        break;
      case TOPIC_IDS.TX.ODOMETRY:
        setTelemetry(prev => ({ ...prev, odometry: parsed }));
        break;
    }
  }, [addLog]);

  const connect = useCallback(async () => {
    try {
      const port = await navigator.serial.requestPort();
      await port.open({ baudRate: 115200 });
      portRef.current = port;

      const parser = new FrameParser(handleFrame);
      readLoopRef.current = true;

      const reader = port.readable.getReader();
      readerRef.current = reader;

      if (port.writable) {
        writerRef.current = port.writable.getWriter();
      }

      freqTrackerRef.current.start();
      freqIntervalRef.current = setInterval(() => {
        setFrequencies(freqTrackerRef.current.getRates());
      }, 500);

      setConnected(true);

      // Read loop
      (async () => {
        try {
          while (readLoopRef.current) {
            const { value, done } = await reader.read();
            if (done) break;
            if (value) {
              for (let i = 0; i < value.length; i++) {
                parser.feed(value[i]);
              }
            }
          }
        } catch (e) {
          if (readLoopRef.current) console.error('Read error:', e);
        } finally {
          reader.releaseLock();
        }
      })();
    } catch (e) {
      console.error('Connection failed:', e);
    }
  }, [handleFrame]);

  const disconnect = useCallback(async () => {
    readLoopRef.current = false;
    freqTrackerRef.current.stop();
    if (freqIntervalRef.current) clearInterval(freqIntervalRef.current);

    try {
      if (readerRef.current) {
        await readerRef.current.cancel();
        readerRef.current = null;
      }
      if (writerRef.current) {
        writerRef.current.releaseLock();
        writerRef.current = null;
      }
      if (portRef.current) {
        await portRef.current.close();
        portRef.current = null;
      }
    } catch (e) {
      console.error('Disconnect error:', e);
    }
    setConnected(false);
    setFrequencies({});
  }, []);

  const sendPacket = useCallback(async (packet) => {
    if (!writerRef.current) return;
    try {
      await writerRef.current.write(packet);
      addLog('TX', packet[2], packet, null);
    } catch (e) {
      console.error('Write error:', e);
    }
  }, [addLog]);

  return {
    connected,
    connect,
    disconnect,
    sendPacket,
    telemetry,
    frequencies,
    linkActive: frequencies['0x81'] > 0,
    log,
  };
}
