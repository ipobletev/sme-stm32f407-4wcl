import { Plug, Unplug, Radio } from 'lucide-react';

export default function Header({ connected, onConnect, onDisconnect }) {
  return (
    <header className="header">
      <div className="header-brand">
        <div className="logo-icon">S</div>
        <div>
          <h1>SerialROS Dashboard</h1>
          <span className="subtitle">STM32 Binary Protocol Client</span>
        </div>
      </div>

      <div className="header-actions">
        <div className={`connection-badge ${connected ? 'connected' : 'disconnected'}`}>
          <span className="dot"></span>
          {connected ? 'Connected · 115200 bps' : 'Disconnected'}
        </div>

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
