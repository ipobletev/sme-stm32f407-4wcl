import { Plug, Unplug, AlertTriangle, Menu } from 'lucide-react';
import { TOPIC_IDS, Encoders, buildPacket } from '../utils/protocol';

export default function Header({ 
  connected, 
  onConnect, 
  onDisconnect, 
  sendPacket, 
  sidebarCollapsed, 
  setSidebarCollapsed 
}) {
  
  const handleEmergencyStop = () => {
    if (!connected || !sendPacket) return;
    
    // Send SYS_EVENT 0x02 (STOP)
    const payload = Encoders.sysEvent(0x02);
    const packet = buildPacket(TOPIC_IDS.RX.SYS_EVENT, payload);
    sendPacket(packet);
    
    // Visual feedback
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
        <div className="header-page-title">
          <h2>Dashboard Overview</h2>
          <span className="breadcrumb">Telemetry / Real-time</span>
        </div>
      </div>

      <div className="header-actions">
        {/* Emergency Stop Button - Always visible if connected */}
        {connected && (
          <button 
            className="btn btn-estop" 
            onClick={handleEmergencyStop}
            title="Emergency Stop (Topic 0x05, Val 0x02)"
          >
            <AlertTriangle size={16} />
            E-STOP
          </button>
        )}
        
        {/* Connection status badge */}
        <div className={`connection-badge ${connected ? 'connected' : 'disconnected'}`}>
          <span className="dot"></span>
          {connected ? 'Connected · 115200 bps' : 'Disconnected'}
        </div>

        {/* Connect/Disconnect Buttons */}
        {connected ? (
          <button className="btn btn-danger" onClick={onDisconnect}>
            <Unplug size={14} />
            Disconnect
          </button>
        ) : (
          <button className="btn btn-primary" onClick={onConnect}>
            <Plug size={14} />
            Connect
          </button>
        )}
      </div>
    </header>
  );
}
