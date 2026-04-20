import React, { useState, useEffect } from 'react';
import { Settings, Save, RefreshCw, AlertTriangle, ShieldCheck, Undo2 } from 'lucide-react';
import { TOPIC_IDS, Encoders, buildPacket } from '../utils/protocol';

const PARAM_GROUPS = [
  {
    title: 'System & Debug',
    params: [
      { id: 0x01, key: 'debug_level', label: 'Debug Level', type: 'select', options: [
        { label: 'NONE', value: 0 },
        { label: 'ERROR', value: 1 },
        { label: 'WARN', value: 2 },
        { label: 'INFO', value: 3 },
        { label: 'DEBUG', value: 4 },
      ]},
      { id: 0x02, key: 'telemetry_period', label: 'Telemetry Period (ms)', type: 'number', min: 1, max: 1000 },
      { id: 0x04, key: 'sys_vars_period', label: 'System Vars Period (ms)', type: 'number', min: 50, max: 5000 },
      { id: 0x05, key: 'imu_period', label: 'IMU Sample Period (ms)', type: 'number', min: 5, max: 500 },
      { id: 0x06, key: 'odom_period', label: 'Odom Publish Period (ms)', type: 'number', min: 5, max: 1000 },
    ]
  },
  {
    title: 'Motor Control',
    params: [
      { id: 0x10, key: 'pid_enabled', label: 'PID Enabled by default', type: 'boolean' },
      { id: 0x11, key: 'motor_ticks', label: 'Motor Ticks/Rev', type: 'number', min: 1 },
      { id: 0x12, key: 'motor_rps_limit', label: 'Max Speed (RPS)', type: 'number', step: 0.1, min: 0.1 },
      { id: 0x13, key: 'motor_deadzone', label: 'PWM Deadzone', type: 'number', min: 0, max: 65535 },
      { id: 0x14, key: 'motor_pwm_max', label: 'PWM Max Range', type: 'number', min: 1000, max: 65535 },
    ]
  },
  {
    title: 'Kinematics & Chassis',
    params: [
      { id: 0x20, key: 'wheel_diameter', label: 'Wheel Diameter (m)', type: 'number', step: 0.001, min: 0.01 },
      { id: 0x21, key: 'shaft_width', label: 'Shaft Width (m)', type: 'number', step: 0.001, min: 0.05 },
      { id: 0x22, key: 'wheelbase_length', label: 'Wheelbase length (m)', type: 'number', step: 0.001, min: 0.05 },
    ]
  },
  {
    title: 'Motor Multipliers (Inversion)',
    params: [
      { id: 0x31, key: 'motor1_inv', label: 'Motor 1 Dir', type: 'select', options: [{label: 'Normal (1)', value: 1}, {label: 'Inverted (-1)', value: -1}] },
      { id: 0x32, key: 'motor2_inv', label: 'Motor 2 Dir', type: 'select', options: [{label: 'Normal (1)', value: 1}, {label: 'Inverted (-1)', value: -1}] },
      { id: 0x33, key: 'motor3_inv', label: 'Motor 3 Dir', type: 'select', options: [{label: 'Normal (1)', value: 1}, {label: 'Inverted (-1)', value: -1}] },
      { id: 0x34, key: 'motor4_inv', label: 'Motor 4 Dir', type: 'select', options: [{label: 'Normal (1)', value: 1}, {label: 'Inverted (-1)', value: -1}] },
    ]
  }
];

export default function ConfigPanel({ appConfig, sendPacket, connected }) {
  const [localConfig, setLocalConfig] = useState(null);
  const [pendingChanges, setPendingChanges] = useState({});

  useEffect(() => {
    if (appConfig) {
      setLocalConfig(appConfig);
    }
  }, [appConfig]);

  const handleFetchConfig = () => {
    sendPacket(buildPacket(TOPIC_IDS.RX.GET_CONFIG));
  };

  const handleUpdateParam = (id, key, value) => {
    // Send single param packet to robot (Update RAM)
    sendPacket(buildPacket(TOPIC_IDS.RX.SET_CONFIG, Encoders.setConfig(id, parseFloat(value))));
    
    // Update local state temporarily
    setLocalConfig(prev => ({ ...prev, [key]: value }));
    setPendingChanges(prev => ({ ...prev, [key]: true }));
  };

  const handleSaveToFlash = () => {
    if (window.confirm('Are you sure you want to write these settings to PERMANENT Flash memory?')) {
      // Send dedicated SAVE_CONFIG topic (0x0A)
      sendPacket(buildPacket(TOPIC_IDS.RX.SAVE_CONFIG, []));
      setPendingChanges({});
    }
  };

  const handleRevertParam = (id, key) => {
    if (!appConfig) return;
    const originalValue = appConfig[key];
    
    // Sync robot RAM back to original value
    sendPacket(buildPacket(TOPIC_IDS.RX.SET_CONFIG, Encoders.setConfig(id, originalValue)));
    
    // Update local UI
    setLocalConfig(prev => ({ ...prev, [key]: originalValue }));
    setPendingChanges(prev => {
      const next = { ...prev };
      delete next[key];
      return next;
    });
  };

  const isDirty = (key) => {
    if (!localConfig || !appConfig) return false;
    return localConfig[key] != appConfig[key];
  };

  if (!localConfig) {
    return (
      <div className="card empty-config">
        <div className="card-header">
          <Settings className="header-icon" />
          <h2>Device Configuration</h2>
        </div>
        <div className="card-content centered">
          <p className="description">Configuration hasn't been fetched from the robot yet.</p>
          <button 
            className="btn btn-primary btn-with-icon" 
            onClick={handleFetchConfig}
            disabled={!connected}
          >
            <RefreshCw size={16} />
            Fetch Configuration
          </button>
        </div>
      </div>
    );
  }

  return (
    <div className="config-grid">
      <div className="config-header-row">
        <div className="title-area">
          <Settings className="icon-pulse" style={{ color: 'var(--accent-cyan)' }} />
          <h1>Configuration Manager</h1>
        </div>
        <div className="action-area">
          <button className="btn btn-ghost btn-with-icon" onClick={handleFetchConfig}>
            <RefreshCw size={14} />
            Refresh
          </button>
          <button 
            className="btn btn-accent btn-with-icon" 
            onClick={handleSaveToFlash}
            data-pending={Object.keys(pendingChanges).length > 0 ? 'true' : 'false'}
          >
            <Save size={14} />
            Save to Flash
          </button>
        </div>
      </div>

      {Object.keys(pendingChanges).length > 0 && (
        <div className="alert alert-warn">
          <AlertTriangle size={18} />
          <span>You have unsaved changes in RAM. Click "Save to Flash" to make them persistent.</span>
        </div>
      )}

      <div className="groups-container">
        {PARAM_GROUPS.map(group => (
          <div className="config-card" key={group.title}>
            <h3>{group.title}</h3>
            <div className="params-list">
              {group.params.map(p => (
                <div className="param-item" key={p.id}>
                  <div className="param-info">
                    <span className="param-label">{p.label}</span>
                    <div className="param-meta">
                      <span className="param-key">{p.key}</span>
                      {p.type === 'number' && (p.min !== undefined || p.max !== undefined) && (
                        <span className="param-range">[{p.min ?? '—'}, {p.max ?? '—'}]</span>
                      )}
                    </div>
                  </div>
                  <div className="param-control">
                    <div className="param-control-group">
                      {p.type === 'select' ? (
                        <select 
                          value={localConfig[p.key]} 
                          onChange={(e) => handleUpdateParam(p.id, p.key, e.target.value)}
                        >
                          {p.options.map(opt => (
                            <option key={opt.value} value={opt.value}>{opt.label}</option>
                          ))}
                        </select>
                      ) : p.type === 'boolean' ? (
                        <button 
                          className={`toggle ${localConfig[p.key] ? 'active' : ''}`}
                          onClick={() => handleUpdateParam(p.id, p.key, localConfig[p.key] ? 0 : 1)}
                        >
                          {localConfig[p.key] ? 'Enabled' : 'Disabled'}
                        </button>
                      ) : (
                        <input 
                          type="number" 
                          value={localConfig[p.key]}
                          step={p.step || 1}
                          min={p.min}
                          max={p.max}
                          onChange={(e) => handleUpdateParam(p.id, p.key, e.target.value)}
                        />
                      )}
                      
                      {isDirty(p.key) && (
                        <>
                          <button 
                            className="btn-revert" 
                            title="Revert to fetched value"
                            onClick={() => handleRevertParam(p.id, p.key)}
                          >
                            <Undo2 size={12} />
                          </button>
                          <div className="dirty-dot" />
                        </>
                      )}
                    </div>
                  </div>
                </div>
              ))}
            </div>
          </div>
        ))}
      </div>

      <div className="config-footer">
        <div className="security-tag">
          <ShieldCheck size={14} />
          <span>Magic: 0x{localConfig.magic.toString(16).toUpperCase()}</span>
          <span className="separator">|</span>
          <span>CRC: 0x{localConfig.crc.toString(16).toUpperCase()}</span>
        </div>
      </div>
    </div>
  );
}
