import { useState } from 'react';
import { useSerial } from './hooks/useSerial';
import { useTelemetryHistory } from './hooks/useTelemetryHistory';
import { useFsmTransitionLog } from './hooks/useFsmTransitionLog';
import Header from './components/Header';
import PageSidebar from './components/PageSidebar';
import TelemetryPanel from './components/TelemetryPanel';
import GraphsPanel from './components/GraphsPanel';
import SystemStatusMap from './components/SystemStatusMap';
import FsmTransitionLogPanel from './components/FsmTransitionLogPanel';
import CommandPanel from './components/CommandPanel';
import ActuatorControl from './components/ActuatorControl';
import LogPanel from './components/LogPanel';
import ErrorLogPanel from './components/ErrorLogPanel';
import OperatorControl from './components/OperatorControl';
import { Activity, Power, Gamepad2 } from 'lucide-react';
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
            <Activity 
              size={12} 
              className={active ? 'icon-pulse' : ''}
              style={{ color: active ? 'var(--accent-cyan)' : 'var(--accent-rose)' }} 
            />
            <span className="topic-name">{TOPIC_LABELS[tid] || tid}</span>
            <span 
              className="freq-value" 
              style={{ color: active ? 'var(--accent-emerald)' : 'var(--accent-rose)' }}
            >
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
  const { 
    connected, isMaster, connect, disconnect, sendPacket, 
    telemetry, frequencies, linkActive, log 
  } = useSerial();
  const history = useTelemetryHistory(telemetry, 50);
  const fsmTransitionLog = useFsmTransitionLog(telemetry.sysStatus);
  
  const [sidebarCollapsed, setSidebarCollapsed] = useState(true);
  const [activeTab, setActiveTab] = useState('dashboard');

  return (
    <div className="page-wrapper">
      {/* Global Sidebar - Level Page */}
      <PageSidebar 
        collapsed={sidebarCollapsed} 
        activeTab={activeTab} 
        onTabChange={setActiveTab} 
        sysStatus={telemetry.sysStatus}
      />

      {/* Main Content Area */}
      <main className="main-container">
        <Header 
          connected={connected} 
          isMaster={isMaster}
          linkActive={linkActive}
          onConnect={connect} 
          onDisconnect={disconnect} 
          sendPacket={sendPacket}
          sidebarCollapsed={sidebarCollapsed}
          setSidebarCollapsed={setSidebarCollapsed}
        />
        
        <div className="app-layout">
          {/* Frequency bar — full width */}
          <FrequencyBar frequencies={frequencies} />

          {activeTab === 'dashboard' ? (
            <>
              {/* Main telemetry area */}
              <TelemetryPanel telemetry={telemetry} frequencies={frequencies} />

              {/* Right sidebar: scrollable control panel */}
              <div className="sidebar">
                <div className="sidebar-scroll">
                  <CommandPanel sendPacket={sendPacket} connected={connected} sysStatus={telemetry.sysStatus} />
                </div>
              </div>

              {/* Full-width bottom row: Packet Log + Error Log side by side */}
              <div className="log-footer">
                <LogPanel log={log} onClear={() => {}} />
                <ErrorLogPanel sysStatus={telemetry.sysStatus} />
              </div>
            </>
          ) : activeTab === 'graphs' ? (
            <div className="graphs-view">
              <GraphsPanel history={history} />
            </div>
          ) : activeTab === 'fsm' ? (
            <div className="fsm-view">
              <SystemStatusMap
                sysStatus={telemetry.sysStatus}
                sendPacket={sendPacket}
                connected={connected}
              />
            </div>
          ) : activeTab === 'fsm-log' ? (
            <div className="fsm-log-view">
              <FsmTransitionLogPanel
                rows={fsmTransitionLog.rows}
                onClear={fsmTransitionLog.clear}
                connected={connected}
                sysStatus={telemetry.sysStatus}
              />
            </div>
          ) : activeTab === 'actuator-tool' ? (
            <div className="diagnostics-view" style={{ maxWidth: '600px', margin: '0 auto', width: '100%' }}>
              <ActuatorControl sendPacket={sendPacket} connected={connected} sysStatus={telemetry.sysStatus} />
            </div>
          ) : activeTab === 'operator-control' ? (
            <OperatorControl 
              sendPacket={sendPacket} 
              connected={connected} 
              sysStatus={telemetry.sysStatus} 
            />
          ) : (
            <div className="empty-state">
              <p>Unknown tab: {activeTab}</p>
            </div>
          )}
        </div>
      </main>
    </div>
  );
}
