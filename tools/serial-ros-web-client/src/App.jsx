import { useSerial } from './hooks/useSerial';
import Header from './components/Header';
import TelemetryPanel from './components/TelemetryPanel';
import CommandPanel from './components/CommandPanel';
import LogPanel from './components/LogPanel';
import { Activity } from 'lucide-react';
import './index.css';

const TOPIC_LABELS = {
  '0x81': 'sys_status',
  '0x82': 'imu',
  '0x83': 'odometry',
};

function FrequencyBar({ frequencies }) {
  const topics = ['0x81', '0x82', '0x83'];

  return (
    <div className="frequency-bar">
      {topics.map(tid => {
        const hz = frequencies[tid] || 0;
        const active = hz > 0;
        return (
          <div className="freq-chip" data-active={active ? 'true' : 'false'} key={tid}>
            <Activity size={12} style={{ color: active ? 'var(--accent-cyan)' : 'var(--text-muted)' }} />
            <span className="topic-name">{TOPIC_LABELS[tid] || tid}</span>
            <span className="freq-value" style={{ color: active ? 'var(--accent-emerald)' : 'var(--text-muted)' }}>
              {hz}
            </span>
            <span className="freq-unit">Hz</span>
          </div>
        );
      })}
    </div>
  );
}

export default function App() {
  const { connected, connect, disconnect, sendPacket, telemetry, frequencies, log } = useSerial();

  return (
    <>
      <Header connected={connected} onConnect={connect} onDisconnect={disconnect} />
      <div className="app-layout">
        <FrequencyBar frequencies={frequencies} />
        <TelemetryPanel telemetry={telemetry} frequencies={frequencies} />
        <div className="sidebar">
          <CommandPanel sendPacket={sendPacket} connected={connected} />
          <LogPanel log={log} onClear={() => {}} />
        </div>
      </div>
    </>
  );
}
