import { useState, useCallback } from 'react';
import { Send, Joystick, Cog, Zap, Play, Square, Pause, RotateCcw, RefreshCw, Shield, Navigation } from 'lucide-react';
import { buildPacket, Encoders, TOPIC_IDS } from '../utils/protocol';

export default function CommandPanel({ sendPacket, connected }) {
  const [linearX, setLinearX] = useState(0);
  const [angularZ, setAngularZ] = useState(0);
  const [j1, setJ1] = useState(0);
  const [j2, setJ2] = useState(0);
  const [j3, setJ3] = useState(0);
  const [mobMode, setMobMode] = useState(0);
  const [isAuto, setIsAuto] = useState(0);

  const disabled = !connected;

  const sendAutonomous = useCallback((auto) => {
    setIsAuto(auto);
    const payload = Encoders.autonomous(auto);
    sendPacket(buildPacket(TOPIC_IDS.RX.AUTONOMOUS, Array.from(payload)));
  }, [sendPacket]);

  const sendCmdVel = useCallback(() => {
    const payload = Encoders.cmdVel(linearX, angularZ);
    sendPacket(buildPacket(TOPIC_IDS.RX.CMD_VEL, Array.from(payload)));
  }, [linearX, angularZ, sendPacket]);

  const sendArmGoal = useCallback(() => {
    const payload = Encoders.armGoal(j1, j2, j3);
    sendPacket(buildPacket(TOPIC_IDS.RX.ARM_GOAL, Array.from(payload)));
  }, [j1, j2, j3, sendPacket]);

  const sendMobilityMode = useCallback(() => {
    const payload = Encoders.mobilityMode(mobMode, isAuto);
    sendPacket(buildPacket(TOPIC_IDS.RX.MOBILITY_MODE, Array.from(payload)));
  }, [mobMode, isAuto, sendPacket]);

  const sendEvent = useCallback((eventId) => {
    const payload = Encoders.sysEvent(eventId);
    sendPacket(buildPacket(TOPIC_IDS.RX.SYS_EVENT, Array.from(payload)));
  }, [sendPacket]);

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
              className={`event-btn ${isAuto === 0 ? 'start' : 'stop'}`}
              disabled={disabled}
              onClick={() => sendAutonomous(0)}
              style={{ opacity: isAuto === 0 ? 1 : 0.5 }}
            >
              <Shield size={12} /> Manual
            </button>
            <button
              className={`event-btn ${isAuto === 1 ? 'resume' : 'stop'}`}
              disabled={disabled}
              onClick={() => sendAutonomous(1)}
              style={{ opacity: isAuto === 1 ? 1 : 0.5 }}
            >
              <Navigation size={12} /> Autonomous
            </button>
          </div>
        </div>
      </div>

      {/* System Events — Topic 0x05 */}
      <div className="card">
        <div className="card-header">
          <h3><Zap size={14} /> System Events</h3>
          <span className="card-badge">0x05</span>
        </div>
        <div className="card-body">
          <div className="event-buttons">
            <button className="event-btn start" disabled={disabled} onClick={() => sendEvent(0x01)}>
              <Play size={12} /> Start
            </button>
            <button className="event-btn stop" disabled={disabled} onClick={() => sendEvent(0x02)}>
              <Square size={12} /> Stop
            </button>
            <button className="event-btn pause" disabled={disabled} onClick={() => sendEvent(0x03)}>
              <Pause size={12} /> Pause
            </button>
            <button className="event-btn resume" disabled={disabled} onClick={() => sendEvent(0x04)}>
              <RefreshCw size={12} /> Resume
            </button>
            <button className="event-btn reset" disabled={disabled} onClick={() => sendEvent(0x05)}>
              <RotateCcw size={12} /> Reset
            </button>
            <button className="event-btn rehome" disabled={disabled} onClick={() => sendEvent(0x06)}>
              <RotateCcw size={12} /> Home Arm
            </button>
          </div>
        </div>
      </div>

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
              <span className="slider-value">{linearX.toFixed(2)} m/s</span>
            </div>
            <input type="range" min={-2} max={2} step={0.01} value={linearX}
              onChange={e => setLinearX(Number(e.target.value))}
              disabled={disabled}
              onDoubleClick={() => setLinearX(0)} />
          </div>
          <div className="slider-group">
            <div className="slider-label">
              <span>Angular Z</span>
              <span className="slider-value">{angularZ.toFixed(2)} rad/s</span>
            </div>
            <input type="range" min={-3.14} max={3.14} step={0.01} value={angularZ}
              onChange={e => setAngularZ(Number(e.target.value))}
              disabled={disabled}
              onDoubleClick={() => setAngularZ(0)} />
          </div>
          <button className="btn btn-primary btn-sm" style={{ width: '100%' }} onClick={sendCmdVel} disabled={disabled}>
            <Send size={12} /> Send Velocity
          </button>
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
                <span className="slider-value">{joint.val.toFixed(2)}°</span>
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
}
