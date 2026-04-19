import React, { useState, useRef, useEffect, useCallback } from 'react';
import { Gamepad2, Zap, ShieldAlert, Sliders, Play, Square, AlertCircle, ChevronUp, ChevronDown, ChevronLeft, ChevronRight, ArrowUpLeft, ArrowUpRight, ArrowDownLeft, ArrowDownRight } from 'lucide-react';
import { TOPIC_IDS, Encoders, buildPacket } from '../utils/protocol';
import { getSupervisorStateName, getSupervisorStateClass } from '../utils/fsmLabels';

export default function OperatorControl({ sendPacket, connected, sysStatus }) {
  const [maxLinear, setMaxLinear] = useState(0.5); // m/s
  const [maxAngular, setMaxAngular] = useState(1.5); // rad/s
  const [isDriving, setIsDriving] = useState(false);
  const [displayVel, setDisplayVel] = useState({ x: 0, z: 0 });
  
  const joystickRef = useRef(null);
  const knobRef = useRef(null);
  const velRef = useRef({ x: 0, z: 0 });
  const limitsRef = useRef({ linear: maxLinear, angular: maxAngular });
  const lastUpdateTimeRef = useRef(0);
  const wasDrivingRef = useRef(false);

  // Sync limits to ref
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
      if (interval) clearInterval(interval);
    };
  }, [isDriving, connected, transmitVelocity]);

  const handlePointerDown = (e) => {
    setIsDriving(true);
    handlePointerMove(e);
  };

  const handlePointerMove = (e) => {
    if (!isDriving || !joystickRef.current) return;

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
    velRef.current = { x: 0, z: 0 };
    if (knobRef.current) {
      knobRef.current.style.transform = `translate(0px, 0px)`;
    }
  };

  const handleArrowPress = (direction) => {
    setIsDriving(true);
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
      window.addEventListener('touchmove', handlePointerMove);
      window.addEventListener('touchend', handlePointerUp);
    }
    return () => {
      window.removeEventListener('mousemove', handlePointerMove);
      window.removeEventListener('mouseup', handlePointerUp);
      window.removeEventListener('touchmove', handlePointerMove);
      window.removeEventListener('touchend', handlePointerUp);
    };
  }, [isDriving, maxLinear, maxAngular]);

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
                <span className="limit-value">{maxLinear.toFixed(2)} m/s</span>
              </div>
              <input 
                type="range" min="0.1" max="2.0" step="0.1" 
                value={maxLinear} onChange={(e) => setMaxLinear(parseFloat(e.target.value))} 
              />
            </div>
            
            <div className="limit-group">
              <div className="limit-label">
                <span>Max Angular Speed</span>
                <span className="limit-value">{maxAngular.toFixed(2)} rad/s</span>
              </div>
              <input 
                type="range" min="0.5" max="5.0" step="0.1" 
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
          
          <div 
            className={`joystick-container ${!isAuto || !connected ? 'disabled' : ''}`}
            ref={joystickRef}
            onMouseDown={handlePointerDown}
            onTouchStart={handlePointerDown}
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

          <div className={`dpad-container ${!isAuto || !connected ? 'disabled' : ''}`}>
             <div className="dpad-row">
               <button className="dpad-btn diag" onMouseDown={() => handleArrowPress('up-left')} onMouseUp={handleArrowRelease} onTouchStart={() => handleArrowPress('up-left')} onTouchEnd={handleArrowRelease}>
                 <ArrowUpLeft size={20} />
               </button>
               <button className="dpad-btn" onMouseDown={() => handleArrowPress('up')} onMouseUp={handleArrowRelease} onTouchStart={() => handleArrowPress('up')} onTouchEnd={handleArrowRelease}>
                 <ChevronUp size={24} />
               </button>
               <button className="dpad-btn diag" onMouseDown={() => handleArrowPress('up-right')} onMouseUp={handleArrowRelease} onTouchStart={() => handleArrowPress('up-right')} onTouchEnd={handleArrowRelease}>
                 <ArrowUpRight size={20} />
               </button>
             </div>
             
             <div className="dpad-row">
               <button className="dpad-btn" onMouseDown={() => handleArrowPress('left')} onMouseUp={handleArrowRelease} onTouchStart={() => handleArrowPress('left')} onTouchEnd={handleArrowRelease}>
                 <ChevronLeft size={24} />
               </button>
               <div className="dpad-center">
                 <Zap size={18} className={isDriving ? 'active' : ''} />
               </div>
               <button className="dpad-btn" onMouseDown={() => handleArrowPress('right')} onMouseUp={handleArrowRelease} onTouchStart={() => handleArrowPress('right')} onTouchEnd={handleArrowRelease}>
                 <ChevronRight size={24} />
               </button>
             </div>

             <div className="dpad-row">
               <button className="dpad-btn diag" onMouseDown={() => handleArrowPress('down-left')} onMouseUp={handleArrowRelease} onTouchStart={() => handleArrowPress('down-left')} onTouchEnd={handleArrowRelease}>
                 <ArrowDownLeft size={20} />
               </button>
               <button className="dpad-btn" onMouseDown={() => handleArrowPress('down')} onMouseUp={handleArrowRelease} onTouchStart={() => handleArrowPress('down')} onTouchEnd={handleArrowRelease}>
                 <ChevronDown size={24} />
               </button>
               <button className="dpad-btn diag" onMouseDown={() => handleArrowPress('down-right')} onMouseUp={handleArrowRelease} onTouchStart={() => handleArrowPress('down-right')} onTouchEnd={handleArrowRelease}>
                 <ArrowDownRight size={20} />
               </button>
             </div>
          </div>
          
          <div className="e-stop-container">
            <button className="btn-estop large" onClick={handleEStop}>
              <Square fill="white" size={18} />
              EMERGENCY STOP
            </button>
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
