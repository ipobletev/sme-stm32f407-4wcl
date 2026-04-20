import React, { useState, useEffect } from 'react';
import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, Legend, ReferenceLine } from 'recharts';
import { Activity, Zap, Compass, Move, BarChart2, Trash2, Clock, Settings2, RotateCcw } from 'lucide-react';

function calculateStats(history, key) {
  const values = history.map(p => p[key]).filter(v => v !== undefined);
  if (values.length === 0) return { min: 0, max: 0, avg: 0, std: 0, jitter: 0 };
  
  const filtered = values.filter(v => v !== 0);
  const min = filtered.length > 0 ? Math.min(...filtered) : 0;
  const max = filtered.length > 0 ? Math.max(...filtered) : 0;
  const avg = values.reduce((a, b) => a + b, 0) / values.length;
  const std = Math.sqrt(values.reduce((a, b) => a + Math.pow(b - avg, 2), 0) / values.length);
  
  // Calculate jitter in ms only for frequency keys
  let jitter = 0;
  if (key.startsWith('freq_') && min > 0) {
    const dt_max = 1000 / min;
    const dt_min = 1000 / max;
    jitter = dt_max - dt_min;
  }
  
  return { min, max, avg, std, jitter };
}

function StatisticsTable({ history, dataKeys, unit }) {
  const isFreq = dataKeys.some(dk => dk.key.startsWith('freq_'));
  const gridTemplate = isFreq 
    ? '1.8fr 0.8fr 0.8fr 0.8fr 1fr 0.8fr 1.2fr' 
    : '2fr 1fr 1fr 1fr 1.2fr';
  
  return (
    <div className="stats-table">
      <div className="stats-grid-header" style={{ gridTemplateColumns: gridTemplate }}>
        <span>Signal</span>
        {isFreq && <span>Target</span>}
        <span>Min</span>
        <span>Max</span>
        <span>Avg</span>
        <span>σ (Std)</span>
        {isFreq && <span>Jitter (ms)</span>}
      </div>
      {dataKeys.map((dk, idx) => {
        const stats = calculateStats(history, dk.key);
        return (
          <div key={idx} className="stats-grid-row" style={{ 
            borderLeftColor: dk.color,
            gridTemplateColumns: gridTemplate
          }}>
            <span className="stats-label" style={{ color: dk.color }}>{dk.name}</span>
            {isFreq && (
              <span className="stats-val target" style={{ opacity: dk.target ? 1 : 0.4 }}>
                {dk.target ? `${dk.target.toFixed(0)}` : '--'}
              </span>
            )}
            <span className="stats-val">{stats.min.toFixed(2)}</span>
            <span className="stats-val">{stats.max.toFixed(2)}</span>
            <span className="stats-val highlight">{stats.avg.toFixed(2)}</span>
            <span className="stats-val std">{stats.std.toFixed(3)}</span>
            {isFreq && <span className="stats-val jitter">{stats.jitter.toFixed(2)}</span>}
          </div>
        );
      })}
    </div>
  );
}

export default function GraphsPanel({ history, onClear, maxPoints, setMaxPoints, appConfig }) {
  const [freqFilter, setFreqFilter] = useState('all');
  const [yDomains, setYDomains] = useState(() => {
    const saved = localStorage.getItem('telemetry_y_domains');
    return saved ? JSON.parse(saved) : {};
  });
  const [editingId, setEditingId] = useState(null);

  useEffect(() => {
    localStorage.setItem('telemetry_y_domains', JSON.stringify(yDomains));
  }, [yDomains]);

  const handleDomainChange = (id, field, value) => {
    setYDomains(prev => ({
      ...prev,
      [id]: {
        ...(prev[id] || { min: 'auto', max: 'auto' }),
        [field]: value === '' ? 'auto' : (isNaN(value) ? 'auto' : Number(value))
      }
    }));
  };

  const resetDomain = (id) => {
    setYDomains(prev => {
      const next = { ...prev };
      delete next[id];
      return next;
    });
  };

  const targets = {
    imu: (appConfig && appConfig.imu_period > 0) ? (1000 / appConfig.imu_period) : null,
    odom: (appConfig && appConfig.odom_period > 0) ? (1000 / appConfig.odom_period) : null,
    sys: (appConfig && appConfig.sys_vars_period > 0) ? (1000 / appConfig.sys_vars_period) : null,
  };

  const chartConfigs = [
    {
      id: 'freq',
      title: 'System Diagnostic Frequencies (Hz)',
      icon: BarChart2,
      color: 'var(--accent-indigo)',
      dataKeys: [
        { key: 'freq_imu', color: 'var(--accent-cyan)', name: 'IMU Hz', filter: 'imu', target: targets.imu },
        { key: 'freq_odom', color: 'var(--accent-emerald)', name: 'Odom Hz', filter: 'odom', target: targets.odom },
        { key: 'freq_sys', color: 'var(--accent-rose)', name: 'Sys Hz', filter: 'sys', target: targets.sys },
      ],
      unit: ' Hz'
    },
    {
      id: 'kin',
      title: 'Drive Kinematics (Velocity)',
      icon: Move,
      color: 'var(--accent-cyan)',
      dataKeys: [
        { key: 'vx', color: '#00f2fe', name: 'Linear X (m/s)' },
        { key: 'wz', color: '#4facfe', name: 'Angular Z (rad/s)' },
      ],
      unit: ''
    },
    {
      id: 'enc',
      title: 'Motor Feedback (Encoders)',
      icon: Activity,
      color: 'var(--accent-indigo)',
      dataKeys: [
        { key: 'enc1', color: '#6366f1', name: 'Enc 1' },
        { key: 'enc2', color: '#818cf8', name: 'Enc 2' },
        { key: 'enc3', color: '#a5b4fc', name: 'Enc 3' },
        { key: 'enc4', color: '#c7d2fe', name: 'Enc 4' },
      ],
      unit: ' pulses'
    },
    {
      id: 'rpy',
      title: 'IMU Orientation (RPY)',
      icon: Compass,
      color: 'var(--accent-rose)',
      dataKeys: [
        { key: 'roll', color: '#f093fb', name: 'Roll' },
        { key: 'pitch', color: '#f5576c', name: 'Pitch' },
        { key: 'yaw', color: '#48c6ef', name: 'Yaw' },
      ],
      unit: '°'
    },
    {
      id: 'accel',
      title: 'Linear Acceleration (m/s²)',
      icon: Activity,
      color: 'var(--accent-violet)',
      dataKeys: [
        { key: 'ax', color: '#8e2de2', name: 'Accel X' },
        { key: 'ay', color: '#4389af', name: 'Accel Y' },
        { key: 'az', color: '#25aae1', name: 'Accel Z' },
      ],
      unit: ' m/s²'
    },
    {
      id: 'gyro',
      title: 'Angular Velocity (rad/s)',
      icon: Compass,
      color: 'var(--accent-amber)',
      dataKeys: [
        { key: 'gx', color: '#ff9a9e', name: 'Gyro X' },
        { key: 'gy', color: '#a18cd1', name: 'Gyro Y' },
        { key: 'gz', color: '#fbc2eb', name: 'Gyro Z' },
      ],
      unit: ' rad/s'
    },
    {
      id: 'batt',
      title: 'Battery Voltage',
      icon: Zap,
      color: 'var(--accent-emerald)',
      dataKeys: [
        { key: 'battery', color: '#43e97b', name: 'Voltage (V)' },
      ],
      unit: 'V'
    },
    {
      id: 'temp',
      title: 'MCU Temperature',
      icon: Activity,
      color: '#fa709a',
      dataKeys: [
        { key: 'temp', color: '#fa709a', name: 'Temp (°C)' },
      ],
      unit: '°C'
    }
  ];

  return (
    <div className="graphs-panel">
      <div className="section-header" style={{ justifyContent: 'space-between' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
          <Activity size={20} className="icon-pulse" />
          <h2>Real-time Telemetry Analysis</h2>
        </div>
        
        <div className="graph-controls">
          <div className="control-group">
            <Clock size={14} style={{ color: 'var(--accent-cyan)' }} />
            <span style={{ fontSize: '0.75rem', color: 'var(--text-secondary)' }}>Scale:</span>
            <select 
              value={maxPoints} 
              onChange={(e) => setMaxPoints(Number(e.target.value))}
              className="control-select"
            >
              <option value={50}>50 pts</option>
              <option value={100}>100 pts</option>
              <option value={200}>200 pts</option>
              <option value={500}>500 pts</option>
              <option value={1000}>1000 pts</option>
            </select>
          </div>
          
          <button className="btn btn-danger btn-sm" onClick={onClear}>
            <Trash2 size={14} />
            <span>Clear History</span>
          </button>
        </div>
      </div>

      <div className="graphs-grid">
        {chartConfigs.map((config, idx) => {
          const dataKeys = config.id === 'freq' && freqFilter !== 'all'
            ? config.dataKeys.filter(dk => dk.filter === freqFilter)
            : config.dataKeys;

          const domain = yDomains[config.id] || { min: 'auto', max: 'auto' };
          const isAuto = domain.min === 'auto' && domain.max === 'auto';

          return (
            <div key={idx} className="chart-card glass-card">
              <div className="chart-header" style={{ justifyContent: 'space-between', paddingBottom: editingId === config.id ? '4px' : '12px' }}>
                <div style={{ display: 'flex', alignItems: 'center', gap: '12px' }}>
                  <config.icon size={18} style={{ color: config.color }} />
                  <h3>{config.title}</h3>
                </div>
                
                <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
                  {config.id === 'freq' && (
                    <div className="segmented-control">
                      {['all', 'imu', 'odom', 'sys'].map(f => (
                        <button 
                          key={f}
                          className={`segment-btn ${freqFilter === f ? 'active' : ''}`}
                          onClick={() => setFreqFilter(f)}
                          style={f !== 'all' ? { '--segment-color': `var(--accent-${f === 'imu' ? 'cyan' : f === 'odom' ? 'emerald' : 'rose'})` } : {}}
                        >
                          {f.toUpperCase()}
                        </button>
                      ))}
                    </div>
                  )}
                  <button 
                    className={`icon-btn ${!isAuto ? 'active' : ''}`} 
                    onClick={() => setEditingId(editingId === config.id ? null : config.id)}
                    title="Y-Axis Limits"
                  >
                    <Settings2 size={16} />
                  </button>
                </div>
              </div>

              {editingId === config.id && (
                <div className="limit-controls-panel">
                  <div className="limit-input-group">
                    <span>Min:</span>
                    <input 
                      type="text" 
                      placeholder="auto" 
                      value={domain.min === 'auto' ? '' : domain.min}
                      onChange={(e) => handleDomainChange(config.id, 'min', e.target.value)}
                    />
                  </div>
                  <div className="limit-input-group">
                    <span>Max:</span>
                    <input 
                      type="text" 
                      placeholder="auto" 
                      value={domain.max === 'auto' ? '' : domain.max}
                      onChange={(e) => handleDomainChange(config.id, 'max', e.target.value)}
                    />
                  </div>
                  <button className="reset-btn" onClick={() => resetDomain(config.id)}>
                    <RotateCcw size={12} />
                  </button>
                </div>
              )}
              
              <div className="chart-container">
                <ResponsiveContainer width="100%" height={250}>
                  <LineChart data={history}>
                    <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.05)" vertical={false} />
                    <XAxis 
                      dataKey="timeLabel" 
                      stroke="rgba(255,255,255,0.3)" 
                      fontSize={10}
                      tick={{ fill: 'rgba(255,255,255,0.5)' }}
                      hide={config.id === 'freq'}
                    />
                    <YAxis 
                      domain={[domain.min, domain.max]}
                      stroke="rgba(255,255,255,0.3)" 
                      fontSize={10}
                      tick={{ fill: 'rgba(255,255,255,0.5)' }}
                    />
                    <Tooltip 
                      contentStyle={{ 
                        backgroundColor: 'rgba(15, 23, 42, 0.9)', 
                        border: '1px solid rgba(255,255,255,0.1)',
                        borderRadius: '8px',
                        fontSize: '12px'
                      }} 
                    />
                    <Legend />
                    {dataKeys.map(dk => (
                      <Line 
                        key={dk.key}
                        type="monotone" 
                        dataKey={dk.key} 
                        name={dk.name}
                        stroke={dk.color} 
                        strokeWidth={2}
                        dot={false}
                        isAnimationActive={false}
                      />
                    ))}
                    {/* Render reference lines after data lines to ensure they are on top */}
                    {dataKeys.map(dk => dk.target && (
                      <ReferenceLine 
                        key={`${dk.key}-target`}
                        y={dk.target} 
                        stroke={dk.color} 
                        strokeDasharray="4 4" 
                        strokeOpacity={0.7}
                        strokeWidth={1.5}
                        label={{ 
                          value: `Target: ${dk.target}Hz`, 
                          position: 'insideRight', 
                          fill: dk.color, 
                          fontSize: 10,
                          fontWeight: 'bold',
                          opacity: 0.9,
                          dy: -10
                        }} 
                      />
                    ))}
                  </LineChart>
                </ResponsiveContainer>
              </div>

              <StatisticsTable history={history} dataKeys={dataKeys} unit={config.unit} />
            </div>
          );
        })}
      </div>
    </div>
  );
}



