import { useCallback } from 'react';
import { Zap, Play, Square, Pause, RefreshCw, RotateCcw, AlertTriangle } from 'lucide-react';
import { buildPacket, Encoders, TOPIC_IDS } from '../utils/protocol';

export default function SystemEventsControl({ sendPacket, connected }) {
  const disabled = !connected;

  const sendEvent = useCallback((eventId) => {
    const payload = Encoders.sysEvent(eventId);
    sendPacket(buildPacket(TOPIC_IDS.RX.SYS_EVENT, Array.from(payload)));
  }, [sendPacket]);

  return (
    <div className="card">
      <div className="card-header">
        <h3><Zap size={14} /> System Events</h3>
        <span className="card-badge">0x05</span>
      </div>
      <div className="card-body">
        <div className="event-buttons">
          <button className="event-btn start" disabled={disabled} onClick={() => sendEvent(0x01)}>
            <Play size={12} /> Start
          </button>
          <button className="event-btn stop" disabled={disabled} onClick={() => sendEvent(0x02)}>
            <Square size={12} /> Stop
          </button>
          <button className="event-btn pause" disabled={disabled} onClick={() => sendEvent(0x03)}>
            <Pause size={12} /> Pause
          </button>
          <button className="event-btn resume" disabled={disabled} onClick={() => sendEvent(0x04)}>
            <RefreshCw size={12} /> Resume
          </button>
          <button className="event-btn reset" disabled={disabled} onClick={() => sendEvent(0x05)}>
            <RotateCcw size={12} /> Reset
          </button>
          <button className="event-btn fault" disabled={disabled} onClick={() => sendEvent(0x06)}>
            <AlertTriangle size={12} /> FAULT (E-STOP)
          </button>
          <button className="event-btn test" disabled={disabled} onClick={() => sendEvent(0x07)} style={{ background: 'var(--accent-blue)', color: 'white' }}>
            <Zap size={12} /> TEST/DIAG
          </button>
        </div>
      </div>
    </div>
  );
}
