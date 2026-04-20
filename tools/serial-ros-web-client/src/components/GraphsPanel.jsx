import { LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, Legend } from 'recharts';
import { Activity, Zap, Compass, Move, BarChart2 } from 'lucide-react';

function calculateStats(history, key) {
  const values = history.map(p => p[key]).filter(v => v !== undefined && v !== 0);
  if (values.length === 0) return { min: 0, max: 0, avg: 0, std: 0, jitter: 0 };
  
  const min = Math.min(...values);
  const max = Math.max(...values);
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
  
  return (
    <div className="stats-table">
      <div className="stats-grid-header" style={{ 
        gridTemplateColumns: isFreq ? '2fr 1fr 1fr 1fr 1fr 1.2fr' : '2fr 1fr 1fr 1fr 1.2fr'
      }}>
        <span>Signal</span>
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
            gridTemplateColumns: isFreq ? '2fr 1fr 1fr 1fr 1fr 1.2fr' : '2fr 1fr 1fr 1fr 1.2fr'
          }}>
            <span className="stats-label" style={{ color: dk.color }}>{dk.name}</span>
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

export default function GraphsPanel({ history }) {
  const chartConfigs = [
    {
      title: 'IMU Sample Rate (Hz)',
      icon: Activity,
      color: 'var(--accent-cyan)',
      dataKeys: [{ key: 'freq_imu', color: 'var(--accent-cyan)', name: 'IMU Hz' }],
      unit: ' Hz'
    },
    {
      title: 'Odometry Rate (Hz)',
      icon: Move,
      color: 'var(--accent-emerald)',
      dataKeys: [{ key: 'freq_odom', color: 'var(--accent-emerald)', name: 'Odom Hz' }],
      unit: ' Hz'
    },
    {
      title: 'Sys Status Rate (Hz)',
      icon: Zap,
      color: 'var(--accent-rose)',
      dataKeys: [{ key: 'freq_sys', color: 'var(--accent-rose)', name: 'Sys Hz' }],
      unit: ' Hz'
    },
    {
      title: 'PID Debug Rate (Hz)',
      icon: BarChart2,
      color: 'var(--accent-indigo)',
      dataKeys: [{ key: 'freq_pid', color: 'var(--accent-indigo)', name: 'PID Hz' }],
      unit: ' Hz'
    },
    {
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
      title: 'Battery Voltage',
      icon: Zap,
      color: 'var(--accent-emerald)',
      dataKeys: [
        { key: 'battery', color: '#43e97b', name: 'Voltage (V)' },
      ],
      unit: 'V'
    },
    {
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
      <div className="section-header">
        <Activity size={20} className="icon-pulse" />
        <h2>Real-time Telemetry Analysis</h2>
      </div>

      <div className="graphs-grid">
        {chartConfigs.map((config, idx) => (
          <div key={idx} className="chart-card glass-card">
            <div className="chart-header">
              <config.icon size={18} style={{ color: config.color }} />
              <h3>{config.title}</h3>
            </div>
            
            <div className="chart-container">
              <ResponsiveContainer width="100%" height={250}>
                <LineChart data={history}>
                  <CartesianGrid strokeDasharray="3 3" stroke="rgba(255,255,255,0.05)" vertical={false} />
                  <XAxis 
                    dataKey="timeLabel" 
                    stroke="rgba(255,255,255,0.3)" 
                    fontSize={10}
                    tick={{ fill: 'rgba(255,255,255,0.5)' }}
                  />
                  <YAxis 
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
                  {config.dataKeys.map(dk => (
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
                </LineChart>
              </ResponsiveContainer>
            </div>

            <StatisticsTable history={history} dataKeys={config.dataKeys} unit={config.unit} />
          </div>
        ))}
      </div>
    </div>
  );
}
