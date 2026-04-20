import { useState, useRef, useCallback, useEffect } from 'react';
import { calculateCRC16, parsePayload, TOPIC_IDS, buildPacket } from '../utils/protocol';

const SYNC1 = 0xAA;
const SYNC2 = 0x55;
const MIN_FRAME = 6;

const CHANNEL_NAME = 'robot_serial_bridge';
const MSG_TYPES = {
  HEARTBEAT: 'HEARTBEAT',
  TELEMETRY_DATA: 'TELEMETRY_DATA',
  COMMAND_REQUEST: 'COMMAND_REQUEST',
  FREQ_UPDATE: 'FREQ_UPDATE',
};

/**
 * Frequency tracker: counts messages per topic and computes Hz every second.
 */
class FrequencyTracker {
  constructor() {
    this.history = {}; // topicId -> [timestamps]
    this.rates = {};
    this._interval = null;
    this.onUpdate = null;
  }

  start() {
    this.stop();
    this._interval = setInterval(() => {
      const now = Date.now();
      const WINDOW_MS = 2000;

      for (const key in this.history) {
        // Clean up old timestamps
        this.history[key] = this.history[key].filter(ts => (now - ts) < WINDOW_MS);
        
        // Calculate Hz: Count in window / Window size in seconds
        const count = this.history[key].length;
        this.rates[key] = count / (WINDOW_MS / 1000.0);
      }
      
      if (this.onUpdate) this.onUpdate(this.rates);
    }, 1000);
  }

  stop() {
    if (this._interval) clearInterval(this._interval);
    this.history = {};
    this.rates = {};
  }

  tick(topicId) {
    const key = `0x${topicId.toString(16).padStart(2, '0')}`;
    if (!this.history[key]) this.history[key] = [];
    this.history[key].push(Date.now());
    if (this.rates[key] === undefined) this.rates[key] = 0;
  }

  setRates(rates) {
    this.rates = rates;
  }

  getRates() {
    return { ...this.rates };
  }
}

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
  const [connected, setConnected] = useState(false); // Only true for Physical Master
  const [sharedConnected, setSharedConnected] = useState(false); // True if any tab is master
  const [networkConnected, setNetworkConnected] = useState(false); // True if WS relay is active
  
  const [telemetry, setTelemetry] = useState({
    telemetry_period: 0,
    sysStatus: null,
    imu: null,
    odometry: null,
    appConfig: null,
  });
  const [frequencies, setFrequencies] = useState({});
  const [log, setLog] = useState([]);

  const portRef = useRef(null);
  const readerRef = useRef(null);
  const writerRef = useRef(null);
  const readLoopRef = useRef(false);
  const freqTrackerRef = useRef(new FrequencyTracker());
  const freqIntervalRef = useRef(null);
  const bcRef = useRef(null);
  const wsRef = useRef(null);
  const lastTeleTickRef = useRef(0);
  const lastTopicTicksRef = useRef({});
  const lastHeartbeatRef = useRef(0);

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

  const handleFrame = useCallback((topicId, data, isShared = false) => {
    freqTrackerRef.current.tick(topicId);
    const now = Date.now();
    lastTeleTickRef.current = now;
    lastTopicTicksRef.current[`0x${topicId.toString(16).padStart(2, '0')}`] = now;
    
    const parsed = parsePayload(topicId, data);
    if (!parsed) return;

    if (!isShared) {
      addLog('RX', topicId, data, parsed);
      const msg = { type: MSG_TYPES.TELEMETRY_DATA, topicId, data: Array.from(data) };
      
      // Broadcast to local tabs
      bcRef.current?.postMessage(msg);
      
      // Broadcast to network (Relay Server)
      if (wsRef.current?.readyState === WebSocket.OPEN) {
        wsRef.current.send(JSON.stringify(msg));
      }
    }

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
      case TOPIC_IDS.TX.APP_CONFIG_DATA:
        setTelemetry(prev => ({ ...prev, appConfig: parsed }));
        break;
    }
  }, [addLog]);

  const sendPacket = useCallback(async (packet) => {
    const rawPacket = Array.from(packet);
    // Use the current state values but avoid them being dependencies if they trigger loops
    // In this specific hook, we can rely on the fact that sendPacket is called by user actions
    if (connected && writerRef.current) {
      try {
        await writerRef.current.write(packet);
        addLog('TX', packet[2], packet, null);
      } catch (e) {
        console.error('Write error:', e);
      }
    } else if (sharedConnected && bcRef.current) {
      bcRef.current.postMessage({ type: MSG_TYPES.COMMAND_REQUEST, packet: rawPacket });
      addLog('TX (Tab Proxy)', packet[2], packet, null);
    } else if (networkConnected && wsRef.current?.readyState === WebSocket.OPEN) {
      wsRef.current.send(JSON.stringify({ type: MSG_TYPES.COMMAND_REQUEST, packet: rawPacket }));
      addLog('TX (Net Proxy)', packet[2], packet, null);
    }
  }, [connected, sharedConnected, networkConnected, addLog]);

  // Network Relay Initialization - STABILIZED
  useEffect(() => {
    const protocol = window.location.protocol === 'https:' ? 'wss:' : 'ws:';
    const wsUrl = `${protocol}//${window.location.host}/ws-robot`;
    const ws = new WebSocket(wsUrl);
    wsRef.current = ws;

    ws.onmessage = (event) => {
      try {
        const msg = JSON.parse(event.data);
        const { type, topicId, data, packet, rates } = msg;

        // Use a functional check for connected to avoid it being a sync dependency
        if (connected) {
          if (type === MSG_TYPES.COMMAND_REQUEST) {
            sendPacket(new Uint8Array(packet));
          }
          return;
        }

        switch (type) {
          case MSG_TYPES.HEARTBEAT:
            setNetworkConnected(true);
            lastHeartbeatRef.current = Date.now();
            break;
          case MSG_TYPES.TELEMETRY_DATA:
            handleFrame(topicId, new Uint8Array(data), true);
            break;
          case MSG_TYPES.FREQ_UPDATE:
            setFrequencies(rates);
            break;
        }
      } catch (e) {}
    };

    ws.onopen = () => console.log('[Relay] Connected to network bridge');
    ws.onclose = () => setNetworkConnected(false);

    return () => {
      wsRef.current = null;
      ws.close();
    };
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [connected]); // ONLY re-run if this tab BECOMES the master or stops being one

  // BroadcastChannel Initialization - STABILIZED
  useEffect(() => {
    const bc = new BroadcastChannel(CHANNEL_NAME);
    bcRef.current = bc;

    bc.onmessage = (msg) => {
      const { type, topicId, data, packet, rates } = msg.data;

      if (connected) {
        if (type === MSG_TYPES.COMMAND_REQUEST) {
          sendPacket(new Uint8Array(packet));
        }
        return;
      }

      switch (type) {
        case MSG_TYPES.HEARTBEAT:
          lastHeartbeatRef.current = Date.now();
          setSharedConnected(true);
          break;
        case MSG_TYPES.TELEMETRY_DATA:
          handleFrame(topicId, new Uint8Array(data), true);
          break;
        case MSG_TYPES.FREQ_UPDATE:
          setFrequencies(rates);
          break;
      }
    };

    const checker = setInterval(() => {
      if (!connected && Date.now() - lastHeartbeatRef.current > 2000) {
        setSharedConnected(false);
        setNetworkConnected(false);
      }
    }, 1000);

    return () => {
      clearInterval(checker);
      bc.close();
    };
  }, [connected, handleFrame, sendPacket]);

  // Master Heartbeat Broadcast
  useEffect(() => {
    if (connected) {
      const hbInterval = setInterval(() => {
        const hb = { type: MSG_TYPES.HEARTBEAT };
        bcRef.current?.postMessage(hb);
        if (wsRef.current?.readyState === WebSocket.OPEN) {
          wsRef.current.send(JSON.stringify(hb));
        }
        
        // Also send binary heartbeat to the robot
        console.log('[Heartbeat] Pumping heartbeat packet (0x00) to serial...');
        sendPacket(buildPacket(TOPIC_IDS.RX.HEARTBEAT));
      }, 500);

      freqTrackerRef.current.onUpdate = (rates) => {
        const msg = { type: MSG_TYPES.FREQ_UPDATE, rates };
        bcRef.current?.postMessage(msg);
        if (wsRef.current?.readyState === WebSocket.OPEN) {
          wsRef.current.send(JSON.stringify(msg));
        }
        setFrequencies(rates);
      };

      return () => {
        clearInterval(hbInterval);
        freqTrackerRef.current.onUpdate = null;
      };
    }
  }, [connected]);

  const connect = useCallback(async () => {
    if (connected) return;
    try {
      const port = await navigator.serial.requestPort();
      
      // If the port is already open, we might have lost state but keep the lock
      // We check if we can open it; if it fails with 'already open', we try to use it if it's ours
      try {
        await port.open({ baudRate: 115200 });
      } catch (err) {
        if (err.name === 'InvalidStateError') {
          console.warn('Port already open, attempting to use current state');
        } else {
          throw err;
        }
      }
      
      portRef.current = port;
      const parser = new FrameParser(handleFrame);
      readLoopRef.current = true;
      const reader = port.readable.getReader();
      readerRef.current = reader;
      if (port.writable) writerRef.current = port.writable.getWriter();
      freqTrackerRef.current.start();
      setConnected(true);
      setSharedConnected(true);
      (async () => {
        try {
          while (readLoopRef.current) {
            const { value, done } = await reader.read();
            if (done) break;
            if (value) {
              for (let i = 0; i < value.length; i++) parser.feed(value[i]);
            }
          }
        } catch (e) {} finally { reader.releaseLock(); }
      })();
    } catch (e) {
      console.error('Connection failed:', e);
    }
  }, [handleFrame]);

  const disconnect = useCallback(async () => {
    readLoopRef.current = false;
    freqTrackerRef.current.stop();
    try {
      if (readerRef.current) await readerRef.current.cancel();
      if (writerRef.current) writerRef.current.releaseLock();
      if (portRef.current) await portRef.current.close();
    } catch (e) {}
    setConnected(false);
    setFrequencies({});
    setTelemetry({ sysStatus: null, imu: null, odometry: null });
  }, []);

  return {
    connected: connected || sharedConnected || networkConnected,
    isMaster: connected,
    connect,
    disconnect,
    sendPacket,
    telemetry,
    frequencies,
    lastTopicTicks: lastTopicTicksRef.current,
    linkActive: (Date.now() - lastTeleTickRef.current) < ((telemetry.appConfig?.sys_vars_period || 1000) + 1000),
    log,
  };
}
