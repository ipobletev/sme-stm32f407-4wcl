import React, { useState, useCallback, memo } from 'react';
import { Send, Joystick, Cog, Shield, Navigation, StopCircle } from 'lucide-react';
import { buildPacket, Encoders, TOPIC_IDS } from '../utils/protocol';
import SystemEventsControl from './SystemEventsControl';

const CommandPanel = memo(function CommandPanel({ sendPacket, connected, sysStatus }) {
  const [linearX, setLinearX] = useState(0);
  const [angularZ, setAngularZ] = useState(0);
  const [j1, setJ1] = useState(0);
  const [j2, setJ2] = useState(0);
  const [j3, setJ3] = useState(0);
  const [mobMode, setMobMode] = useState(0);

  const currentSupState = sysStatus?.state ?? 0;
  const isAutoModeActual = currentSupState === 3;
  const isManualModeActual = currentSupState === 2;

  const disabled = !connected;

  const sendAutonomous = useCallback((auto) => {
    const payload = Encoders.autonomous(auto);
    sendPacket(buildPacket(TOPIC_IDS.RX.AUTONOMOUS, Array.from(payload)));
  }, [sendPacket]);

  const sendCmdVel = useCallback(() => {
    const payload = Encoders.cmdVel(linearX, angularZ);
    sendPacket(buildPacket(TOPIC_IDS.RX.CMD_VEL, Array.from(payload)));
  }, [linearX, angularZ, sendPacket]);

  const stopMobility = useCallback(() => {
    setLinearX(0);
    setAngularZ(0);
    const payload = Encoders.cmdVel(0, 0);
    sendPacket(buildPacket(TOPIC_IDS.RX.CMD_VEL, Array.from(payload)));
  }, [sendPacket]);

  const sendArmGoal = useCallback(() => {
    const payload = Encoders.armGoal(j1, j2, j3);
    sendPacket(buildPacket(TOPIC_IDS.RX.ARM_GOAL, Array.from(payload)));
  }, [j1, j2, j3, sendPacket]);

  const sendMobilityMode = useCallback(() => {
    const payload = Encoders.mobilityMode(mobMode, isAutoModeActual ? 1 : 0);
    sendPacket(buildPacket(TOPIC_IDS.RX.MOBILITY_MODE, Array.from(payload)));
  }, [mobMode, isAutoModeActual, sendPacket]);

  return (
    <>
      {/* ControlBoard Mode — Topic 0x01 */}
      <div className="card">
        <div className="card-header">
          <h3><Shield size={14} /> ControlBoard Mode</h3>
          <span className="card-badge">0x01</span>
        </div>
        <div className="card-body">
          <div className="event-buttons">
            <button
              className={`event-btn ${isManualModeActual ? 'start' : 'stop'}`}
              disabled={disabled || currentSupState === 0 || currentSupState === 5}
              onClick={() => sendAutonomous(0)}
              style={{ opacity: isManualModeActual ? 1 : (currentSupState === 1 || currentSupState === 4 ? 0.7 : 0.4) }}
            >
              <Shield size={12} /> Manual
            </button>
            <button
              className={`event-btn ${isAutoModeActual ? 'resume' : 'stop'}`}
              disabled={disabled || currentSupState === 0 || currentSupState === 5}
              onClick={() => sendAutonomous(1)}
              style={{ opacity: isAutoModeActual ? 1 : (currentSupState === 1 || currentSupState === 4 ? 0.7 : 0.4) }}
            >
              <Navigation size={12} /> Autonomous
            </button>
          </div>
        </div>
      </div>

      <SystemEventsControl sendPacket={sendPacket} connected={connected} />

      {/* Mobility Kinematic Model — Topic 0x02 */}
      <div className="card">
        <div className="card-header">
          <h3><Cog size={14} /> Mobility Model</h3>
          <span className="card-badge">0x02</span>
        </div>
        <div className="card-body">
          <div className="mode-select-group">
            <label>Kinematic Model</label>
            <select value={mobMode} onChange={e => setMobMode(Number(e.target.value))} disabled={disabled}>
              <option value={0}>Direct Drive</option>
              <option value={1}>Differential Drive</option>
              <option value={2}>Ackermann</option>
              <option value={3}>Mecanum</option>
            </select>
          </div>
          <button className="btn btn-primary btn-sm" style={{ width: '100%' }} onClick={sendMobilityMode} disabled={disabled}>
            <Send size={12} /> Send Model
          </button>
        </div>
      </div>

      {/* Command Velocity — with sliders */}
      <div className="card">
        <div className="card-header">
          <h3><Joystick size={14} /> cmd_vel</h3>
          <span className="card-badge">0x03</span>
        </div>
        <div className="card-body">
          <div className="slider-group">
            <div className="slider-label">
              <span>Linear X</span>
              <input 
                type="number" 
                className="value-input" 
                value={linearX} 
                step={0.01}
                min={-2}
                max={2}
                onChange={e => setLinearX(Number(e.target.value))}
                disabled={disabled}
              />
            </div>
            <input type="range" min={-2} max={2} step={0.01} value={linearX}
              onChange={e => setLinearX(Number(e.target.value))}
              disabled={disabled}
              onDoubleClick={() => setLinearX(0)} />
          </div>
          <div className="slider-group">
            <div className="slider-label">
              <span>Angular Z</span>
              <input 
                type="number" 
                className="value-input" 
                value={angularZ} 
                step={0.01}
                min={-3.14}
                max={3.14}
                onChange={e => setAngularZ(Number(e.target.value))}
                disabled={disabled}
              />
            </div>
            <input type="range" min={-3.14} max={3.14} step={0.01} value={angularZ}
              onChange={e => setAngularZ(Number(e.target.value))}
              disabled={disabled}
              onDoubleClick={() => setAngularZ(0)} />
          </div>
          <div className="btn-group">
            <button className="btn btn-primary btn-sm" style={{ flex: 2 }} onClick={sendCmdVel} disabled={disabled}>
              <Send size={12} /> Send Velocity
            </button>
            <button className="btn btn-stop-mob btn-sm" onClick={stopMobility} disabled={disabled}>
              <StopCircle size={12} /> STOP
            </button>
          </div>
        </div>
      </div>

      {/* Arm Goal — with sliders */}
      <div className="card">
        <div className="card-header">
          <h3><Cog size={14} /> arm_goal</h3>
          <span className="card-badge">0x04</span>
        </div>
        <div className="card-body">
          {[
            { label: 'Joint 1', val: j1, set: setJ1 },
            { label: 'Joint 2', val: j2, set: setJ2 },
            { label: 'Joint 3', val: j3, set: setJ3 },
          ].map((joint) => (
            <div className="slider-group" key={joint.label}>
              <div className="slider-label">
                <span>{joint.label}</span>
                <input 
                  type="number" 
                  className="value-input" 
                  value={joint.val} 
                  step={0.5}
                  min={-180}
                  max={180}
                  onChange={e => joint.set(Number(e.target.value))}
                  disabled={disabled}
                />
              </div>
              <input type="range" min={-180} max={180} step={0.5} value={joint.val}
                onChange={e => joint.set(Number(e.target.value))}
                disabled={disabled}
                onDoubleClick={() => joint.set(0)} />
            </div>
          ))}
          <button className="btn btn-primary btn-sm" style={{ width: '100%' }} onClick={sendArmGoal} disabled={disabled}>
            <Send size={12} /> Send Arm Goal
          </button>
        </div>
      </div>
    </>
  );
});

export default CommandPanel;
