import React, { useState, useRef, useEffect, useCallback } from 'react';
import { Gamepad2, Zap, ShieldAlert, Sliders, Play, Square, AlertCircle, ChevronUp, ChevronDown, ChevronLeft, ChevronRight, ArrowUpLeft, ArrowUpRight, ArrowDownLeft, ArrowDownRight, RotateCcw, RotateCw } from 'lucide-react';
import { TOPIC_IDS, Encoders, buildPacket } from '../utils/protocol';
import { getSupervisorStateName, getSupervisorStateClass } from '../utils/fsmLabels';

export default function OperatorControl({ sendPacket, connected, sysStatus, appConfig }) {
  const systemMaxSpeed = appConfig?.motor_speed_limit || 2.0;
  const systemMaxAngular = appConfig?.motor_angular_speed_limit || 3.14;

  const [maxLinear, setMaxLinear] = useState(Math.min(0.5, systemMaxSpeed)); // m/s
  const [maxAngular, setMaxAngular] = useState(Math.min(1.5, systemMaxAngular)); // rad/s
  const [isDriving, setIsDriving] = useState(false);
  const [activeControlType, setActiveControlType] = useState(null); // 'joystick' or 'dpad'
  const [displayVel, setDisplayVel] = useState({ x: 0, z: 0 });
  
  const joystickRef = useRef(null);
  const knobRef = useRef(null);
  const velRef = useRef({ x: 0, z: 0 });
  const limitsRef = useRef({ linear: maxLinear, angular: maxAngular });
  const lastUpdateTimeRef = useRef(0);
  const wasDrivingRef = useRef(false);

  // Sync user selected maxLinear with system global maxSpeed if it decreases
  useEffect(() => {
    if (maxLinear > systemMaxSpeed) {
      setMaxLinear(systemMaxSpeed);
    }
    if (maxAngular > systemMaxAngular) {
      setMaxAngular(systemMaxAngular);
    }
  }, [systemMaxSpeed, systemMaxAngular, maxLinear, maxAngular]);

  // Sync limits to ref for the control loop
  useEffect(() => {
    limitsRef.current = { linear: maxLinear, angular: maxAngular };
  }, [maxLinear, maxAngular]);
  // Send velocity packet
  const transmitVelocity = useCallback((lx, az) => {
    if (!connected) return;
    const payload = Encoders.cmdVel(lx, az);
    const packet = buildPacket(TOPIC_IDS.RX.CMD_VEL, payload);
    sendPacket(packet);
  }, [connected, sendPacket]);

  // Stable 20Hz loop
  useEffect(() => {
    let interval;
    if (isDriving && connected) {
      wasDrivingRef.current = true;
      interval = setInterval(() => {
        const { x, z } = velRef.current;
        transmitVelocity(x, z);
        
        // Throttled UI update (approx 10Hz for display to save CPU)
        const now = Date.now();
        if (now - lastUpdateTimeRef.current > 100) {
          setDisplayVel({ x, z });
          lastUpdateTimeRef.current = now;
        }
      }, 50);
    } else {
      // Ensure we always stop when not driving, but only if we were driving before
      if (wasDrivingRef.current) {
        transmitVelocity(0, 0);
        wasDrivingRef.current = false;
      }
      setDisplayVel({ x: 0, z: 0 });
      velRef.current = { x: 0, z: 0 };
    }
    return () => {
      if (interval) {
        clearInterval(interval);
        // Safety: send one last stop when the component unmounts or loop breaks
        transmitVelocity(0, 0);
      }
    };
  }, [isDriving, connected, transmitVelocity]);

  const handlePointerDown = (e) => {
    if (e.cancelable) e.preventDefault();
    setIsDriving(true);
    setActiveControlType('joystick');
    handlePointerMove(e);
  };

  const handlePointerMove = (e) => {
    if (!isDriving || activeControlType !== 'joystick' || !joystickRef.current) return;

    const rect = joystickRef.current.getBoundingClientRect();
    const centerX = rect.left + rect.width / 2;
    const centerY = rect.top + rect.height / 2;
    
    // Support mouse and touch
    const clientX = e.clientX || (e.touches && e.touches[0].clientX);
    const clientY = e.clientY || (e.touches && e.touches[0].clientY);

    let dx = clientX - centerX;
    let dy = clientY - centerY;
    
    const distance = Math.sqrt(dx * dx + dy * dy);
    const maxRadius = rect.width / 2 - 20; // 20px padding
    
    if (distance > maxRadius) {
      const angle = Math.atan2(dy, dx);
      dx = Math.cos(angle) * maxRadius;
      dy = Math.sin(angle) * maxRadius;
    }

    // Update knob position
    if (knobRef.current) {
      knobRef.current.style.transform = `translate(${dx}px, ${dy}px)`;
    }

    // Map to velocities
    // Forward (linear_x) is negative dy (up), Left (angular_z) is negative dx (left)
    const lx = (-dy / maxRadius) * limitsRef.current.linear;
    const az = (-dx / maxRadius) * limitsRef.current.angular;
    
    velRef.current = { x: lx, z: az };
  };

  const handlePointerUp = () => {
    setIsDriving(false);
    setActiveControlType(null);
    
    // 1. Immediate stop transmission
    transmitVelocity(0, 0);
    
    // 2. Redundant check: reset values instantly
    velRef.current = { x: 0, z: 0 };
    setDisplayVel({ x: 0, z: 0 });
    
    // 3. UI Reset
    if (knobRef.current) {
      knobRef.current.style.transform = `translate(0px, 0px)`;
    }

    // 4. Send multiple stop packets to ensure delivery over relay/radio
    // (Serial protocols can sometimes drop packets)
    setTimeout(() => transmitVelocity(0, 0), 50);
    setTimeout(() => transmitVelocity(0, 0), 150);
  };

  const handleArrowPress = (direction, e) => {
    if (e && e.cancelable) e.preventDefault();
    setIsDriving(true);
    setActiveControlType('dpad');
    let lx = 0, az = 0;
    const lin = limitsRef.current.linear * 0.7;
    const ang = limitsRef.current.angular * 0.7;
    const diagLin = lin * 0.707; // 1/sqrt(2)
    const diagAng = ang * 0.707;

    switch(direction) {
      case 'up': lx = lin; break;
      case 'down': lx = -lin; break;
      case 'left': az = ang; break;
      case 'right': az = -ang; break;
      case 'rotate-left': az = ang; break;
      case 'rotate-right': az = -ang; break;
      case 'up-left': lx = diagLin; az = diagAng; break;
      case 'up-right': lx = diagLin; az = -diagAng; break;
      case 'down-left': lx = -diagLin; az = diagAng; break;
      case 'down-right': lx = -diagLin; az = -diagAng; break;
    }
    velRef.current = { x: lx, z: az };
  };

  const handleArrowRelease = () => {
    handlePointerUp();
  };

  useEffect(() => {
    if (isDriving) {
      window.addEventListener('mousemove', handlePointerMove);
      window.addEventListener('mouseup', handlePointerUp);
      window.addEventListener('touchmove', handlePointerMove, { passive: false });
      window.addEventListener('touchend', handlePointerUp);
      window.addEventListener('touchcancel', handlePointerUp);
    }
    return () => {
      window.removeEventListener('mousemove', handlePointerMove);
      window.removeEventListener('mouseup', handlePointerUp);
      window.removeEventListener('touchmove', handlePointerMove);
      window.removeEventListener('touchend', handlePointerUp);
      window.removeEventListener('touchcancel', handlePointerUp);
    };
  }, [isDriving]); // Removed maxLinear/maxAngular as they aren't needed for listeners

  const handleEStop = () => {
    transmitVelocity(0, 0);
    // Send standard RESET event to stop all controllers
    const packet = buildPacket(TOPIC_IDS.RX.SYS_EVENT, Encoders.sysEvent(0x02));
    sendPacket(packet);
  };

  const currentState = sysStatus?.state;
  // Strictly allow ONLY AUTO (3) state for Operator Control
  const isAuto = currentState === 3;
  const isOperational = isAuto;
  const stateName = getSupervisorStateName(currentState);

  const handleStart = () => {
    sendPacket(buildPacket(TOPIC_IDS.RX.SYS_EVENT, Encoders.sysEvent(0x01))); // START
  };

  const handleReset = () => {
    sendPacket(buildPacket(TOPIC_IDS.RX.SYS_EVENT, Encoders.sysEvent(0x05))); // RESET
  };

  const handleSetAuto = () => {
    sendPacket(buildPacket(TOPIC_IDS.RX.AUTONOMOUS, Encoders.autonomous(true)));
  };

  return (
    <div className="operator-control-panel">
      <div className="control-header">
        <div className="title-group">
          <Gamepad2 className="title-icon" />
          <div>
            <h1>Operator Console</h1>
            <p>Remote teleoperation via virtual joystick (AUTO mode ONLY)</p>
          </div>
        </div>
        
        <div className="quick-actions">
          <button className="action-btn start" onClick={handleStart} title="Start System">
            <Play size={18} />
            <span>START</span>
          </button>
          <button className="action-btn auto" onClick={handleSetAuto} title="Switch to AUTO mode">
            <Zap size={18} />
            <span>MODE: AUTO</span>
          </button>
          <button className="action-btn reset" onClick={handleReset} title="Reset System">
            <RotateCcw size={18} />
            <span>RESET</span>
          </button>
        </div>

        <div className="status-group">
          <div className={`status-badge ${isAuto ? 'active' : 'inactive'}`}>
            <Zap size={14} />
            {isAuto ? 'REMOTE CONTROL ACTIVE' : `LOCKED - SYSTEM IN ${stateName}`}
          </div>
        </div>
      </div>

      <div className="control-layout">
        {/* Left: Settings & Stats */}
        <div className="control-card settings-card">
          <div className="card-header">
            <h3><Sliders size={16} /> Velocity Limits</h3>
          </div>
          <div className="card-body">
            <div className="limit-group">
              <div className="limit-label">
                <span>Max Linear Speed</span>
                <span className="limit-value">
                  {maxLinear.toFixed(2)} m/s
                  <div style={{ fontSize: '0.65rem', opacity: 0.5, fontWeight: 'normal', marginTop: '2px' }}>
                    System Limit: {systemMaxSpeed.toFixed(2)} m/s
                  </div>
                </span>
              </div>
              <input 
                type="range" 
                min="0.1" 
                max={systemMaxSpeed} 
                step="0.01" 
                value={maxLinear} 
                onChange={(e) => setMaxLinear(parseFloat(e.target.value))} 
              />
            </div>
            
            <div className="limit-group">
              <div className="limit-label">
                <span>Max Angular Speed</span>
                <span className="limit-value">
                  {maxAngular.toFixed(2)} rad/s
                  <div style={{ fontSize: '0.65rem', opacity: 0.5, fontWeight: 'normal', marginTop: '2px' }}>
                    System Limit: {systemMaxAngular.toFixed(2)} rad/s
                  </div>
                </span>
              </div>
              <input 
                type="range" min="0.5" max={systemMaxAngular} step="0.1" 
                value={maxAngular} onChange={(e) => setMaxAngular(parseFloat(e.target.value))} 
              />
            </div>

            <div className="live-stats">
              <div className="stat-item">
                <span className="stat-label">Linear X</span>
                <span className="stat-value">{displayVel.x.toFixed(2)} <small>m/s</small></span>
              </div>
              <div className="stat-item">
                <span className="stat-label">Angular Z</span>
                <span className="stat-value">{displayVel.z.toFixed(2)} <small>rad/s</small></span>
              </div>
            </div>
          </div>
        </div>

        {/* Center: Joystick */}
        <div className="joystick-section">
          {!isAuto && (
            <div className="mode-warning">
              <AlertCircle size={20} />
              <p>
                <strong>Control Locked:</strong> Teleoperation is only available in <strong>AUTO</strong> state. 
                Current state: <em>{stateName}</em>.
              </p>
            </div>
          )}
          <div className="e-stop-container">
            <button className="btn-estop large" onClick={handleEStop}>
              <Square fill="white" size={18} />
              EMERGENCY STOP
            </button>
          </div>
          <div style={{ display: 'flex', alignItems: 'center', justifyContent: 'center', gap: '30px', margin: '20px 0' }}>
            {/* Rotate Right (CW) - INVERTED POSITION */}
            <button 
              className={`dpad-btn diag ${!isAuto || !connected ? 'disabled' : ''}`}
              style={{ width: '60px', height: '60px', borderRadius: '50%', background: 'rgba(255,255,255,0.03)', border: '1px solid rgba(255,255,255,0.1)' }}
              onMouseDown={(e) => handleArrowPress('rotate-right', e)} 
              onMouseUp={handleArrowRelease} 
              onTouchStart={(e) => handleArrowPress('rotate-right', e)} 
              onTouchEnd={handleArrowRelease} 
              onTouchCancel={handleArrowRelease}
              title="Rotate Clockwise"
            >
              <RotateCw size={20} />
            </button>

            <div 
              className={`joystick-container ${!isAuto || !connected ? 'disabled' : ''}`}
              ref={joystickRef}
              onMouseDown={handlePointerDown}
              onTouchStart={handlePointerDown}
              style={{ margin: 0 }}
            >
              <div className="joystick-base">
                <div className="joystick-guide-x"></div>
                <div className="joystick-guide-y"></div>
                <div className="joystick-center-dot"></div>
              </div>
              <div className={`joystick-knob ${isDriving ? 'active' : ''}`} ref={knobRef}>
                <div className="knob-inner"></div>
                {isDriving && <div className="knob-glow"></div>}
              </div>
            </div>

            {/* Rotate Left (CCW) - INVERTED POSITION */}
            <button 
              className={`dpad-btn diag ${!isAuto || !connected ? 'disabled' : ''}`}
              style={{ width: '60px', height: '60px', borderRadius: '50%', background: 'rgba(255,255,255,0.03)', border: '1px solid rgba(255,255,255,0.1)' }}
              onMouseDown={(e) => handleArrowPress('rotate-left', e)} 
              onMouseUp={handleArrowRelease} 
              onTouchStart={(e) => handleArrowPress('rotate-left', e)} 
              onTouchEnd={handleArrowRelease} 
              onTouchCancel={handleArrowRelease}
              title="Rotate Counter-Clockwise"
            >
              <RotateCcw size={20} />
            </button>
          </div>

          <div className={`dpad-container ${!isAuto || !connected ? 'disabled' : ''}`}>
             <div className="dpad-row">
               <button className="dpad-btn diag" onMouseDown={(e) => handleArrowPress('up-left', e)} onMouseUp={handleArrowRelease} onTouchStart={(e) => handleArrowPress('up-left', e)} onTouchEnd={handleArrowRelease} onTouchCancel={handleArrowRelease}>
                 <ArrowUpLeft size={20} />
               </button>
               <button className="dpad-btn" onMouseDown={(e) => handleArrowPress('up', e)} onMouseUp={handleArrowRelease} onTouchStart={(e) => handleArrowPress('up', e)} onTouchEnd={handleArrowRelease} onTouchCancel={handleArrowRelease}>
                 <ChevronUp size={24} />
               </button>
               <button className="dpad-btn diag" onMouseDown={(e) => handleArrowPress('up-right', e)} onMouseUp={handleArrowRelease} onTouchStart={(e) => handleArrowPress('up-right', e)} onTouchEnd={handleArrowRelease} onTouchCancel={handleArrowRelease}>
                 <ArrowUpRight size={20} />
               </button>
             </div>
             
             <div className="dpad-row">
               <button className="dpad-btn" onMouseDown={(e) => handleArrowPress('left', e)} onMouseUp={handleArrowRelease} onTouchStart={(e) => handleArrowPress('left', e)} onTouchEnd={handleArrowRelease} onTouchCancel={handleArrowRelease}>
                 <ChevronLeft size={24} />
               </button>
               <div className="dpad-center">
                 <Zap size={18} className={isDriving ? 'active' : ''} />
               </div>
               <button className="dpad-btn" onMouseDown={(e) => handleArrowPress('right', e)} onMouseUp={handleArrowRelease} onTouchStart={(e) => handleArrowPress('right', e)} onTouchEnd={handleArrowRelease} onTouchCancel={handleArrowRelease}>
                 <ChevronRight size={24} />
               </button>
             </div>

             <div className="dpad-row">
               <button className="dpad-btn diag" onMouseDown={(e) => handleArrowPress('down-left', e)} onMouseUp={handleArrowRelease} onTouchStart={(e) => handleArrowPress('down-left', e)} onTouchEnd={handleArrowRelease} onTouchCancel={handleArrowRelease}>
                 <ArrowDownLeft size={20} />
               </button>
               <button className="dpad-btn" onMouseDown={(e) => handleArrowPress('down', e)} onMouseUp={handleArrowRelease} onTouchStart={(e) => handleArrowPress('down', e)} onTouchEnd={handleArrowRelease} onTouchCancel={handleArrowRelease}>
                 <ChevronDown size={24} />
               </button>
               <button className="dpad-btn diag" onMouseDown={(e) => handleArrowPress('down-right', e)} onMouseUp={handleArrowRelease} onTouchStart={(e) => handleArrowPress('down-right', e)} onTouchEnd={handleArrowRelease} onTouchCancel={handleArrowRelease}>
                 <ArrowDownRight size={20} />
               </button>
             </div>
          </div>
        </div>

        {/* Right: State Info */}
        <div className="control-card info-card">
          <div className="card-header">
            <h3><ShieldAlert size={16} /> Security Status</h3>
          </div>
          <div className="card-body">
            <div className="security-item">
              <div className="security-label">System State</div>
              <span className={`state-badge ${getSupervisorStateClass(sysStatus?.state)}`}>
                {getSupervisorStateName(sysStatus?.state)}
              </span>
            </div>
            <div className="security-item">
              <div className="security-label">Link Status</div>
              <span className={`connection-badge ${connected ? 'connected' : 'disconnected'}`}>
                <div className="dot" />
                {connected ? 'LINK ESTABLISHED' : 'NO CONNECTION'}
              </span>
            </div>
            <p className="security-note">
              Teleoperation is strictly enforced for <strong>AUTO</strong> mode. 
              Manual, Testing, and other states will lock the controls automatically.
            </p>
          </div>
        </div>
      </div>
    </div>
  );
}
