import { Activity, Cpu, Thermometer, Battery, Zap, Gauge, RotateCcw } from 'lucide-react';
import ImuVisualizer from './ImuVisualizer';

const STATE_NAMES = ['IDLE', 'READY', 'MANUAL', 'AUTO', 'PAUSED', 'FAULT'];
const STATE_CLASSES = ['state-idle', 'state-ready', 'state-running', 'state-running', 'state-paused', 'state-fault'];

function getStateName(code) {
  return STATE_NAMES[code] || `UNK(${code})`;
}
function getStateClass(code) {
  return STATE_CLASSES[code] || 'state-idle';
}

function getBatteryPercent(voltage) {
  const min = 10.0, max = 12.6;
  return Math.max(0, Math.min(100, ((voltage - min) / (max - min)) * 100));
}

function getBatteryColor(pct) {
  if (pct > 60) return 'var(--accent-emerald)';
  if (pct > 25) return 'var(--accent-amber)';
  return 'var(--accent-rose)';
}

function fmt(v, d = 2) {
  if (v === null || v === undefined) return '—';
  return Number(v).toFixed(d);
}

export default function TelemetryPanel({ telemetry, frequencies }) {
  const { sysStatus, imu, odometry } = telemetry;

  const batPct = sysStatus ? getBatteryPercent(sysStatus.v_batt) : 0;
  const batColor = getBatteryColor(batPct);

  return (
    <div className="main-content">
      {/* System Status */}
      <div className="card">
        <div className="card-header">
          <h3><Cpu size={14} /> System Status</h3>
          <span className="card-badge">0x81</span>
        </div>
        <div className="card-body">
          {sysStatus ? (
            <>
              <div className="telemetry-grid">
                <div className="telemetry-item">
                  <div className="label">State</div>
                  <span className={`state-badge ${getStateClass(sysStatus.state)}`}>
                    {getStateName(sysStatus.state)}
                  </span>
                </div>
                <div className="telemetry-item">
                  <div className="label">MCU Temp</div>
                  <div className="value value-amber">
                    {fmt(sysStatus.temp, 1)}<span className="unit">°C</span>
                  </div>
                </div>
                <div className="telemetry-item">
                  <div className="label">Battery</div>
                  <div className="value value-emerald">
                    {fmt(sysStatus.v_batt, 1)}<span className="unit">V</span>
                  </div>
                  <div className="battery-bar">
                    <div className="battery-fill" style={{ width: `${batPct}%`, background: batColor }}></div>
                  </div>
                </div>
                <div className="telemetry-item">
                  <div className="label">Current</div>
                  <div className="value value-rose">
                    {fmt(sysStatus.i_batt, 2)}<span className="unit">A</span>
                  </div>
                </div>
                <div className="telemetry-item" style={{ gridColumn: '1 / -1' }}>
                  <div className="label">Error Flags</div>
                  <div className="value value-rose" style={{ fontSize: '0.75rem', wordBreak: 'break-all' }}>
                    0x{(sysStatus.errors || 0n).toString(16).toUpperCase().padStart(16, '0')}
                  </div>
                </div>
              </div>
            </>
          ) : (
            <div className="empty-state">
              <div className="empty-icon">📡</div>
              <p>Waiting for system status telemetry...</p>
            </div>
          )}
        </div>
      </div>

      {/* IMU 3D + Data */}
      <div className="card">
        <div className="card-header">
          <h3><RotateCcw size={14} /> IMU Orientation</h3>
          <span className="card-badge">0x82</span>
        </div>
        <div className="card-body">
          <ImuVisualizer imu={imu} />
          {imu ? (
            <>
              <div className="imu-angles">
                <div className="imu-angle">
                  <div className="angle-label">Roll</div>
                  <div className="angle-value value-cyan">{fmt(imu.roll, 1)}°</div>
                </div>
                <div className="imu-angle">
                  <div className="angle-label">Pitch</div>
                  <div className="angle-value value-violet">{fmt(imu.pitch, 1)}°</div>
                </div>
                <div className="imu-angle">
                  <div className="angle-label">Yaw</div>
                  <div className="angle-value value-emerald">{fmt(imu.yaw, 1)}°</div>
                </div>
              </div>
              <div className="telemetry-grid" style={{ marginTop: '10px' }}>
                <div className="telemetry-item">
                  <div className="label">Gyro X</div>
                  <div className="value value-cyan" style={{ fontSize: '0.85rem' }}>{fmt(imu.gyro?.x)}<span className="unit">°/s</span></div>
                </div>
                <div className="telemetry-item">
                  <div className="label">Gyro Y</div>
                  <div className="value value-cyan" style={{ fontSize: '0.85rem' }}>{fmt(imu.gyro?.y)}<span className="unit">°/s</span></div>
                </div>
                <div className="telemetry-item">
                  <div className="label">Gyro Z</div>
                  <div className="value value-cyan" style={{ fontSize: '0.85rem' }}>{fmt(imu.gyro?.z)}<span className="unit">°/s</span></div>
                </div>
                <div className="telemetry-item">
                  <div className="label">Accel X</div>
                  <div className="value value-amber" style={{ fontSize: '0.85rem' }}>{fmt(imu.accel?.x)}<span className="unit">m/s²</span></div>
                </div>
                <div className="telemetry-item">
                  <div className="label">Accel Y</div>
                  <div className="value value-amber" style={{ fontSize: '0.85rem' }}>{fmt(imu.accel?.y)}<span className="unit">m/s²</span></div>
                </div>
                <div className="telemetry-item">
                  <div className="label">Accel Z</div>
                  <div className="value value-amber" style={{ fontSize: '0.85rem' }}>{fmt(imu.accel?.z)}<span className="unit">m/s²</span></div>
                </div>
              </div>
            </>
          ) : (
            <div className="imu-angles" style={{ justifyContent: 'center', padding: '12px 0' }}>
              <p style={{ color: 'var(--text-muted)', fontSize: '0.75rem' }}>Waiting for IMU data...</p>
            </div>
          )}
        </div>
      </div>

      {/* Odometry */}
      <div className="card">
        <div className="card-header">
          <h3><Gauge size={14} /> Odometry</h3>
          <span className="card-badge">0x83</span>
        </div>
        <div className="card-body">
          {odometry ? (
            <div className="telemetry-grid">
              <div className="telemetry-item">
                <div className="label">Linear X</div>
                <div className="value value-cyan">{fmt(odometry.linear_x, 3)}<span className="unit">m/s</span></div>
              </div>
              <div className="telemetry-item">
                <div className="label">Angular Z</div>
                <div className="value value-violet">{fmt(odometry.angular_z, 3)}<span className="unit">rad/s</span></div>
              </div>
              {odometry.encoders?.map((enc, i) => (
                <div className="telemetry-item" key={i}>
                  <div className="label">Encoder {i + 1}</div>
                  <div className="value value-indigo" style={{ fontSize: '0.85rem' }}>{enc}</div>
                </div>
              ))}
            </div>
          ) : (
            <div className="empty-state">
              <div className="empty-icon">⚙️</div>
              <p>Waiting for odometry data...</p>
            </div>
          )}
        </div>
      </div>
    </div>
  );
}
