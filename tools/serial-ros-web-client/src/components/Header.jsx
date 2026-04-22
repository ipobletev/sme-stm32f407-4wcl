import { useState } from 'react';
import { Plug, Unplug, AlertTriangle, Menu, ShieldCheck, Share2, Anchor, Cpu } from 'lucide-react';
import { TOPIC_IDS, Encoders, buildPacket } from '../utils/protocol';

export default function Header({ 
  connected, 
  isMaster,
  linkActive,
  onConnect, 
  onDisconnect, 
  sendPacket, 
  sidebarCollapsed, 
  setSidebarCollapsed 
}) {
  const [baudRate, setBaudRate] = useState(230400);
  
  const handleEmergencyStop = () => {
    if (!connected || !sendPacket) return;
    const payload = Encoders.sysEvent(0x06);
    const packet = buildPacket(TOPIC_IDS.RX.SYS_EVENT, payload);
    sendPacket(packet);
    console.warn("EMERGENCY STOP SENT");
  };

  return (
    <header className="header">
      <div className="header-left">
        <button 
          className="btn-icon btn-ghost sidebar-toggle" 
          onClick={() => setSidebarCollapsed(!sidebarCollapsed)}
          title={sidebarCollapsed ? "Expand Sidebar" : "Collapse Sidebar"}
        >
          <Menu size={20} />
        </button>
        <div className="header-brand">
          <div className="logo-icon">S</div>
          <div className="brand-text">
            <span className="brand-primary">SME</span>
            <span className="brand-secondary">ROBOTICS</span>
          </div>
        </div>
        <div className="header-page-title">
          <h2>Dashboard Overview</h2>
          <span className="breadcrumb">Telemetry / Real-time</span>
        </div>
      </div>

      <div className="header-actions">
        {/* Connection Mode Indicator */}
        {connected && (
          <div className={`session-indicator ${isMaster ? 'master' : 'follower'}`}>
            {isMaster ? <Anchor size={14} /> : <Share2 size={14} />}
            <span>{isMaster ? 'PRIMARY' : 'LINKED'}</span>
          </div>
        )}

        {/* Emergency Stop Button */}
        {connected && (
          <button className="btn btn-estop" onClick={handleEmergencyStop}>
            <AlertTriangle size={16} />
            E-STOP
          </button>
        )}
        
        {/* Robot Link badge */}
        {connected && (
          <div className={`connection-badge ${linkActive ? 'robot-active' : 'robot-inactive'}`}>
            <span className="dot pulse"></span>
            {linkActive ? 'Link Active' : 'No Robot'}
          </div>
        )}

        {/* Baud Rate Selector */}
        {!connected && (
          <div className="baud-selector">
            <Cpu size={14} />
            <select 
              value={baudRate} 
              onChange={(e) => setBaudRate(Number(e.target.value))}
              className="select-transparent"
            >
              <option value={9600}>9600 bps</option>
              <option value={57600}>57600 bps</option>
              <option value={115200}>115200 bps</option>
              <option value={230400}>230400 bps</option>
              <option value={460800}>460800 bps</option>
              <option value={921600}>921600 bps</option>
            </select>
          </div>
        )}

        {/* Action Button */}
        {connected ? (
          isMaster ? (
            <button className="btn btn-danger" onClick={onDisconnect}>
              <Unplug size={14} />
              Disconnect
            </button>
          ) : (
            <button className="btn btn-primary" onClick={() => onConnect(baudRate)}>
              <ShieldCheck size={14} />
              Take Control
            </button>
          )
        ) : (
          <button className="btn btn-primary" onClick={() => onConnect(baudRate)}>
            <Plug size={14} />
            Connect
          </button>
        )}
      </div>
    </header>
  );
}
