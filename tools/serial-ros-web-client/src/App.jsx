import { useState } from 'react';
import { useSerial } from './hooks/useSerial';
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
import ConfigPanel from './components/ConfigPanel';
import PidTuner from './components/PidTuner';
import { Activity, Power, Gamepad2, Settings, Target } from 'lucide-react';
import './index.css';

const TOPIC_LABELS = {
  '0x81': 'sys_status',
  '0x82': 'imu',
  '0x83': 'odometry',
};

function FrequencyBar({ frequencies, lastTopicTicks }) {
  const topics = ['0x81', '0x82', '0x83'];
  const now = Date.now();

  return (
    <div className="frequency-bar">
      {topics.map(tid => {
        const hz = frequencies[tid] || 0;
        const lastTick = lastTopicTicks?.[tid] || 0;
        const active = (now - lastTick) < 5000;
        
        return (
          <div className="freq-chip" data-active={active ? 'true' : 'false'} key={tid}>
            <Activity 
              size={12} 
              className={hz > 0 ? 'icon-pulse' : ''}
              style={{ color: active ? 'var(--accent-cyan)' : 'var(--accent-rose)' }} 
            />
            <span className="topic-name">{TOPIC_LABELS[tid] || tid}</span>
            <span 
              className="freq-value" 
              style={{ color: active ? 'var(--accent-emerald)' : 'var(--accent-rose)' }}
            >
              {hz.toFixed(1)}
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
    telemetry, history, clearHistory, frequencies, lastTopicTicks, linkActive, log 
  } = useSerial();
  const [maxPoints, setMaxPoints] = useState(1000);
  const fsmTransitionLog = useFsmTransitionLog(telemetry.sysStatus);

  // Persistent PID Tuner State
  const [pidHistory, setPidHistory] = useState([]);
  const [pidChartData, setPidChartData] = useState([]);
  const [pidSettings, setPidSettings] = useState({
    speed: 1.0,
    lead: 500,
    duration: 2000,
    tail: 1000,
    motor: 0
  });
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
          sysStatus={telemetry.sysStatus}
          onConnect={connect} 
          onDisconnect={disconnect} 
          sendPacket={sendPacket}
          sidebarCollapsed={sidebarCollapsed}
          setSidebarCollapsed={setSidebarCollapsed}
        />
        
        <div className="app-layout">
          {/* Frequency bar — full width */}
          <FrequencyBar frequencies={frequencies} lastTopicTicks={lastTopicTicks} />

          {activeTab === 'dashboard' ? (
            <>
              {/* Main telemetry area */}
              <TelemetryPanel telemetry={telemetry} frequencies={frequencies} />

              {/* Right sidebar: scrollable control panel */}
              <div className="sidebar">
                <div className="sidebar-scroll">
                  <CommandPanel sendPacket={sendPacket} connected={connected} sysStatus={telemetry.sysStatus} appConfig={telemetry.appConfig} />
                </div>
              </div>

              {/* Full-width bottom row: Packet Log + Error Log side by side */}
              <div className="log-footer">
                <LogPanel log={log} onClear={() => {}} />
                <ErrorLogPanel sysStatus={telemetry.sysStatus} />
              </div>
            </>
          ) : activeTab === 'graphs' ? (
            <div className="graphs-view full-width-view">
              <GraphsPanel 
                history={history} 
                onClear={clearHistory} 
                maxPoints={maxPoints} 
                setMaxPoints={setMaxPoints} 
                appConfig={telemetry.appConfig}
              />
            </div>

          ) : activeTab === 'fsm' ? (
            <div className="fsm-view full-width-view">
              <SystemStatusMap
                sysStatus={telemetry.sysStatus}
                sendPacket={sendPacket}
                connected={connected}
              />
            </div>
          ) : activeTab === 'fsm-log' ? (
            <div className="fsm-log-view full-width-view">
              <FsmTransitionLogPanel
                rows={fsmTransitionLog.rows}
                onClear={fsmTransitionLog.clear}
                connected={connected}
                sysStatus={telemetry.sysStatus}
              />
            </div>
          ) : activeTab === 'actuator-tool' ? (
            <div className="diagnostics-view full-width-view" style={{ margin: '0 auto', width: '100%' }}>
              <ActuatorControl 
                sendPacket={sendPacket} 
                connected={connected} 
                sysStatus={telemetry.sysStatus} 
                appConfig={telemetry.appConfig} 
              />
            </div>
          ) : activeTab === 'operator-control' ? (
            <OperatorControl 
              sendPacket={sendPacket} 
              connected={connected} 
              sysStatus={telemetry.sysStatus} 
              appConfig={telemetry.appConfig}
            />
          ) : activeTab === 'settings' ? (
            <div className="settings-view full-width-view" style={{ margin: '0 auto', width: '100%' }}>
              <ConfigPanel 
                appConfig={telemetry.appConfig} 
                joystick={telemetry.joystick}
                sendPacket={sendPacket} 
                connected={connected} 
              />
            </div>
          ) : activeTab === 'pid-tuner' ? (
            <PidTuner 
              history={history} 
              appConfig={telemetry.appConfig} 
              sendPacket={sendPacket} 
              connected={connected} 
              sysStatus={telemetry.sysStatus}
              onClear={clearHistory}
              // Persistent Session State
              tuningHistory={pidHistory}
              setTuningHistory={setPidHistory}
              persistentData={pidChartData}
              setPersistentData={setPidChartData}
              settings={pidSettings}
              setSettings={setPidSettings}
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
