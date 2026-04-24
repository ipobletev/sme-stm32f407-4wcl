import React, { memo } from 'react';
import { Cpu, Gauge, RotateCcw, ShieldAlert, ToggleLeft } from 'lucide-react';
import ImuVisualizer from './ImuVisualizer';
import {
  getSupervisorStateName as getStateName,
  getSupervisorStateClass as getStateClass,
  getMobilityStateName as getMobStateName,
  getMobilityStateClass as getMobStateClass,
  getArmStateName as getArmStateName,
  getArmStateClass as getArmStateClass,
  getMobilityModeName,
} from '../utils/fsmLabels';

function getBatteryPercent(voltage) {
  const min = 10.0, max = 12.6;
  return Math.max(0, Math.min(100, ((voltage - min) / (max - min)) * 100));
}

function getBatteryColor(pct) {
  if (pct > 60) return 'var(--accent-emerald)';
  if (pct > 25) return 'var(--accent-amber)';
  return 'var(--accent-rose)';
}

/** Render a number with max 2 decimals */
function fmt(v, d = 2) {
  if (v === null || v === undefined) return '—';
  return Number(v).toFixed(d);
}function toDeg(rad) {
  if (rad === null || rad === undefined) return 0;
  return rad * (180 / Math.PI);
}

const TelemetryPanel = memo(function TelemetryPanel({ telemetry, frequencies }) {
  const { sysStatus, imu, odometry } = telemetry;

  const batPct = sysStatus ? getBatteryPercent(sysStatus.v_batt) : 0;
  const batColor = getBatteryColor(batPct);

  return (
    <div className="main-content">

      {/* Row 1: System Status + Odometry side by side */}
      <div className="telemetry-row">

        {/* System Status */}
        <div className="card flex-card">
          <div className="card-header">
            <h3><Cpu size={14} /> System Status</h3>
            <span className="card-badge">0x81</span>
          </div>
          <div className="card-body">
            {sysStatus ? (
              <>
                <div className="telemetry-grid">
                  <div className="telemetry-item">
                    <div className="label">System State</div>
                    <span className={`state-badge ${getStateClass(sysStatus.state)}`}>
                      {getStateName(sysStatus.state)}
                    </span>
                  </div>
                  <div className="telemetry-item">
                    <div className="label">Mobility</div>
                    <span className={`state-badge ${getMobStateClass(sysStatus.mobility_state)}`}>
                      {getMobStateName(sysStatus.mobility_state)}
                    </span>
                  </div>
                  <div className="telemetry-item">
                    <div className="label">Arm</div>
                    <span className={`state-badge ${getArmStateClass(sysStatus.arm_state)}`}>
                      {getArmStateName(sysStatus.arm_state)}
                    </span>
                  </div>
                  <div className="telemetry-item">
                    <div className="label">Mobility Model</div>
                    <div className="value value-cyan" style={{ fontSize: '0.9rem' }}>
                      {getMobilityModeName(sysStatus.mobility_mode)}
                    </div>
                  </div>
                  <div className="telemetry-item">
                    <div className="label">MCU Temp</div>
                    <div className="value value-amber">
                      {fmt(sysStatus.temp, 2)}<span className="unit">°C</span>
                    </div>
                  </div>
                  <div className="telemetry-item">
                    <div className="label">Battery</div>
                    <div className="value value-emerald">
                      {fmt(sysStatus.v_batt, 2)}<span className="unit">V</span>
                    </div>
                    <div className="battery-bar">
                      <div className="battery-fill" style={{ width: `${batPct}%`, background: batColor }}></div>
                    </div>
                  </div>
                  <div className="telemetry-item">
                    <div className="label">Emergency Stop</div>
                    <span className={`state-badge ${sysStatus.emergency_active ? 'state-fault pulse-red' : 'state-ready'}`} 
                          style={sysStatus.emergency_active ? { fontWeight: 'bold', boxShadow: '0 0 10px var(--accent-rose)' } : {}}>
                      {sysStatus.emergency_active ? 'ON' : 'OFF'}
                    </span>
                  </div>
                  <div className="telemetry-item">
                    <div className="label">Enable Autonomous</div>
                    <span className={`state-badge ${sysStatus.enable_autonomous ? 'state-ready' : 'state-idle'}`}>
                      {sysStatus.enable_autonomous ? 'ON' : 'OFF'}
                    </span>
                  </div>
                  {/* Debug only */}
                  <div style={{ display: 'none' }}>{JSON.stringify(sysStatus)}</div>
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

        {/* Odometry */}
        <div className="card flex-card">
          <div className="card-header">
            <h3><Gauge size={14} /> Odometry</h3>
            <span className="card-badge">0x83</span>
          </div>
          <div className="card-body">
            {odometry ? (
              <div className="telemetry-grid">
                <div className="telemetry-item">
                  <div className="label">Linear X</div>
                  <div className="value value-cyan">{fmt(odometry.linear_x, 2)}<span className="unit">m/s</span></div>
                </div>
                <div className="telemetry-item">
                  <div className="label">Angular Z</div>
                  <div className="value value-violet">{fmt(odometry.angular_z, 2)}<span className="unit">rad/s</span></div>
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

      {/* Row 2: IMU full width, growing to fill space */}
      <div className="card imu-card-flexible">
        <div className="card-header">
          <h3><RotateCcw size={14} /> IMU Orientation</h3>
          <span className="card-badge">0x82</span>
        </div>
        <div className="card-body imu-card-body-flexible">
          <ImuVisualizer imu={imu} />
          {imu ? (
            <>
              <div className="imu-angles">
                <div className="imu-angle">
                  <div className="angle-label">Roll</div>
                  <div className="angle-value value-cyan">{fmt(toDeg(imu.roll), 1)}°</div>
                </div>
                <div className="imu-angle">
                  <div className="angle-label">Pitch</div>
                  <div className="angle-value value-violet">{fmt(toDeg(imu.pitch), 1)}°</div>
                </div>
                <div className="imu-angle">
                  <div className="angle-label">Yaw</div>
                  <div className="angle-value value-emerald">{fmt(toDeg(imu.yaw), 1)}°</div>
                </div>
              </div>
              <div className="telemetry-grid" style={{ marginTop: '10px' }}>
                <div className="telemetry-item">
                  <div className="label">Gyro X</div>
                  <div className="value value-cyan" style={{ fontSize: '0.85rem' }}>{fmt(imu.gyro?.x, 3)}<span className="unit">rad/s</span></div>
                </div>
                <div className="telemetry-item">
                  <div className="label">Gyro Y</div>
                  <div className="value value-cyan" style={{ fontSize: '0.85rem' }}>{fmt(imu.gyro?.y, 3)}<span className="unit">rad/s</span></div>
                </div>
                <div className="telemetry-item">
                  <div className="label">Gyro Z</div>
                  <div className="value value-cyan" style={{ fontSize: '0.85rem' }}>{fmt(imu.gyro?.z, 3)}<span className="unit">rad/s</span></div>
                </div>
                <div className="telemetry-item">
                  <div className="label">Accel X</div>
                  <div className="value value-amber" style={{ fontSize: '0.85rem' }}>{fmt(imu.accel?.x, 2)}<span className="unit">m/s²</span></div>
                </div>
                <div className="telemetry-item">
                  <div className="label">Accel Y</div>
                  <div className="value value-amber" style={{ fontSize: '0.85rem' }}>{fmt(imu.accel?.y, 2)}<span className="unit">m/s²</span></div>
                </div>
                <div className="telemetry-item">
                  <div className="label">Accel Z</div>
                  <div className="value value-amber" style={{ fontSize: '0.85rem' }}>{fmt(imu.accel?.z, 2)}<span className="unit">m/s²</span></div>
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

    </div>
  );
});

export default TelemetryPanel;
