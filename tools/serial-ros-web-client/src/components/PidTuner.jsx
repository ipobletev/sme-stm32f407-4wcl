import React, { useState, useEffect, useMemo, useRef } from 'react';
import { 
  LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, Legend 
} from 'recharts';
import { 
  Settings2, Play, CircleStop, Save, 
  Gauge, Zap, AlertCircle, Info, TrendingUp, Target,
  Activity, ShieldCheck, Send, Power, RotateCcw
} from 'lucide-react';
import { TOPIC_IDS, Encoders, buildPacket, SYS_EVENTS } from '../utils/protocol';
import './PidTuner.css';

const TooltipWrapper = ({ children, content }) => (
  <div className="tooltip-trigger" title={content}>
    {children}
  </div>
);

/**
 * Sub-component to manage local state of each PID gain
 */
const GainSlider = ({ label, value, min, max, step, onSend }) => {
  const [tempValue, setTempValue] = useState(value);
  const [isEditing, setIsEditing] = useState(false);

  // Sync temp value when remote value changes, IF NOT editing
  useEffect(() => {
    if (!isEditing) {
      setTempValue(value);
    }
  }, [value]);

  const handleInputChange = (e) => {
    setTempValue(e.target.value);
    setIsEditing(true);
  };

  const handleSend = () => {
    onSend(tempValue);
    // Don't set editing to false immediately, wait for acknowledgment or blur
  };

  const handleBlur = () => {
    // Small delay to allow the handleSend click to process first if that was the cause of blur
    setTimeout(() => {
      setIsEditing(false);
    }, 200);
  };

  const isDirty = Math.abs(parseFloat(tempValue) - value) > 0.0001;

  return (
    <div className="gain-slider-item">
      <div className="gain-info">
        <span className="gain-label">{label}</span>
        <div className="gain-value-input-row">
          <input 
            type="number"
            className="gain-num-input"
            value={tempValue}
            step={step}
            onChange={handleInputChange}
            onFocus={() => setIsEditing(true)}
            onBlur={handleBlur}
            onKeyDown={(e) => e.key === 'Enter' && handleSend()}
          />
          <button 
            className="btn-send-param" 
            onClick={handleSend} 
            disabled={!isDirty}
            title="Send to Robot RAM"
          >
            <Send size={14} />
          </button>
        </div>
      </div>
    </div>
  );
};

/**
 * PID Optimizer & Visual Tuner Component (Redesigned)
 */
export default function PidTuner({ history, appConfig, sendPacket, connected, sysStatus, onClear }) {
  const [selectedMotor, setSelectedMotor] = useState(0); // 0-3
  const [testRps, setTestRps] = useState(1.0);
  const [testDuration, setTestDuration] = useState(2000);
  const [testTimer, setTestTimer] = useState(null);
  const [isCapturing, setIsCapturing] = useState(false);
  const [persistentData, setPersistentData] = useState([]);
  const [tuningHistory, setTuningHistory] = useState([]);
  const [lastAutoEnable, setLastAutoEnable] = useState(0);
  const [isInternalTransitioning, setIsInternalTransitioning] = useState(false);
  const [pendingPidState, setPendingPidState] = useState(null); // null, 0, or 1
  const hasAutoEnabledRef = useRef(false);

  const isTesting = sysStatus?.state === 6; /* STATE_SUPERVISOR_TESTING */
  const canEnterTest = sysStatus?.state === 2 || sysStatus?.state === 3;
  const disabled = !connected;

  useEffect(() => {
    if (appConfig) {
      // If telemetry matches our pending state, clear the pending state
      if (pendingPidState !== null && appConfig.pid_enabled === pendingPidState) {
        setPendingPidState(null);
      }
    }
  }, [appConfig, pendingPidState]);

  // Auto-enable PID ONLY ONCE on mount if disabled
  useEffect(() => {
    if (appConfig && appConfig.pid_enabled === 0 && !hasAutoEnabledRef.current) {
        console.log('[PidTuner] Auto-enabling PID Controller for initial session...');
        hasAutoEnabledRef.current = true;
        sendPacket(buildPacket(TOPIC_IDS.RX.SET_CONFIG, Encoders.setConfig(0x10, 1.0)));
        setLastAutoEnable(Date.now());
    } else if (appConfig && appConfig.pid_enabled === 1) {
        // Mark as already enabled if it was already on
        hasAutoEnabledRef.current = true;
    }
  }, [appConfig, sendPacket]);

  // Mapping history to chart-ready format
  const currentTelemetryData = useMemo(() => {
    if (!history) return [];
    return history.map(p => ({
      time: p.timeLabel,
      // Current motor
      target: p.pid_target?.[selectedMotor === 'all' ? 0 : selectedMotor] || 0,
      measured: p.pid_measured?.[selectedMotor === 'all' ? 0 : selectedMotor] || 0,
      pwm: p.pid_pwm?.[selectedMotor === 'all' ? 0 : selectedMotor] || 0,
      // Individual traces for 'ALL' mode
      t1: p.pid_target?.[0], m1: p.pid_measured?.[0],
      t2: p.pid_target?.[1], m2: p.pid_measured?.[1],
      t3: p.pid_target?.[2], m3: p.pid_measured?.[2],
      t4: p.pid_target?.[3], m4: p.pid_measured?.[3],
    })).slice(-300);
  }, [history, selectedMotor]);

  // Update persistent data only when capturing
  useEffect(() => {
    if (isCapturing && currentTelemetryData.length > 0) {
      setPersistentData(currentTelemetryData);
    }
  }, [currentTelemetryData, isCapturing]);

  // The chart displays persistent data (frozen if not capturing)
  const chartData = persistentData;

  // Analysis logic
  // Analysis logic
  const analysis = useMemo(() => {
    if (!chartData || chartData.length < 10) return null;

    // Find the actual target used during the step (ignoring leading zeros)
    const targets = chartData.map(p => Math.abs(p.target));
    const target = Math.max(...targets);
    
    if (target < 0.1) return null;

    const measuredValues = chartData.map(p => p.measured);
    const maxVal = Math.max(...measuredValues);
    const minVal = Math.min(...measuredValues);
    const overshoot = ((maxVal - target) / target) * 100;
    const finalVal = measuredValues[measuredValues.length - 1];
    const error = target - finalVal;
    
    // Calculate Rise Time (10% to 90%)
    const t10 = target * 0.1;
    const t90 = target * 0.9;
    let idx10 = -1, idx90 = -1;
    for (let i = 0; i < measuredValues.length; i++) {
        if (idx10 === -1 && measuredValues[i] >= t10) idx10 = i;
        if (idx90 === -1 && measuredValues[i] >= t90) idx90 = i;
    }
    const riseTime = (idx10 !== -1 && idx90 !== -1) ? (idx90 - idx10) : null;

    let status = 'good';
    let suggestion = 'Locked & Loaded';
    let icon = <ShieldCheck size={16} />;

    if (overshoot > 15) {
      status = 'crit';
      suggestion = 'High Overshoot - Reduce KP';
      icon = <AlertCircle size={16} />;
    } else if (overshoot > 8) {
      status = 'warn';
      suggestion = 'Moderate Overshoot - Fine-tune KP/KD';
      icon = <Info size={16} />;
    } else if (Math.abs(error) > (target * 0.05)) {
      status = 'warn';
      suggestion = 'Steady State Error - Increase KI';
      icon = <Activity size={16} />;
    }

    return { 
        overshoot: Math.max(0, overshoot), 
        error, 
        status, 
        suggestion, 
        icon,
        peak: maxVal,
        riseTime: riseTime ? `${riseTime * 20}ms` : 'N/A', // assuming 50Hz (20ms/sample)
        settling: Math.abs(error) < (target * 0.02) ? 'Stable' : 'Unstable'
    };
  }, [chartData]);

  // Auto-log to history when capture finishes
  useEffect(() => {
     if (isCapturing) return; // Only process when capture STOPS
     
     console.log('[PidTuner] Capture stop detected. State:', {
        dataLength: persistentData.length,
        hasAnalysis: !!analysis,
        target: analysis?.peak
     });

     if (persistentData.length > 20 && analysis) {
        console.log('[PidTuner] Recording test result to history...', analysis);
        setTuningHistory(prev => {
            // Prevent duplicate logs for the same timestamp/data
            const timestamp = new Date().toLocaleTimeString();
            if (prev.length > 0 && prev[0].timestamp === timestamp && prev[0].overshoot === analysis.overshoot) {
                console.log('[PidTuner] Skipping duplicate log entry');
                return prev;
            }

            const activeMotorIdx = selectedMotor === 'all' ? 0 : selectedMotor;
            const entry = {
                id: Date.now(),
                timestamp,
                motor: selectedMotor === 'all' ? 'ALL' : `M${selectedMotor + 1}`,
                kp: appConfig?.[`motor${activeMotorIdx + 1}_kp`],
                ki: appConfig?.[`motor${activeMotorIdx + 1}_ki`],
                kd: appConfig?.[`motor${activeMotorIdx + 1}_kd`],
                overshoot: analysis.overshoot,
                error: analysis.error,
                status: analysis.status
            };
            
            console.log('[PidTuner] Entry created:', entry);
            return [entry, ...prev].slice(0, 10);
        });
     }
  }, [isCapturing, analysis, appConfig]);

  // Settling tail logic in runStep completion
  const handleTestCompletion = () => {
    console.log('[PidTuner] Test duration reached. Stopping motors, waiting 1s tail...');
    stopStep();
    setTestTimer(null);
    setTimeout(() => {
        console.log('[PidTuner] 1s tail finished. Stopping capture.');
        setIsCapturing(false);
    }, 1000);
  };

  const yDomain = useMemo(() => {
    const limit = appConfig?.motor_rps_limit || 5.0;
    return [-limit, limit];
  }, [appConfig?.motor_rps_limit]);

  const handleSendParam = async (p, v, forcedMotorIdx = null) => {
    // If forcedMotorIdx is provided, we only update that motor.
    // Otherwise, if in 'all' mode we broadcast, if not we update selected.
    let motorsToUpdate = [];
    if (forcedMotorIdx !== null) {
        motorsToUpdate = [forcedMotorIdx];
    } else {
        motorsToUpdate = selectedMotor === 'all' ? [0, 1, 2, 3] : [selectedMotor];
    }
    
    for (const mIdx of motorsToUpdate) {
        const m = mIdx + 1;
        const key = `motor${m}_${p}`;
        const map = {
            'motor1_kp': 0x40, 'motor1_ki': 0x41, 'motor1_kd': 0x42, 'motor1_deadzone': 0x43,
            'motor2_kp': 0x44, 'motor2_ki': 0x45, 'motor2_kd': 0x46, 'motor2_deadzone': 0x47,
            'motor3_kp': 0x48, 'motor3_ki': 0x49, 'motor3_kd': 0x4A, 'motor3_deadzone': 0x4B,
            'motor4_kp': 0x4C, 'motor4_ki': 0x4D, 'motor4_kd': 0x4E, 'motor4_deadzone': 0x4F,
        };
        const id = map[key];
        if (id) {
            sendPacket(buildPacket(TOPIC_IDS.RX.SET_CONFIG, Encoders.setConfig(id, parseFloat(v))));
            // Tiny delay to avoid flooding the UART queue too fast if multiple motors are updated
            if (motorsToUpdate.length > 1) await new Promise(r => setTimeout(r, 20));
        }
    }
  };

  const startTesting = () => {
    if (disabled || !canEnterTest) return;
    const payload = Encoders.sysEvent(0x07); /* SYS_EVENT_TEST */
    sendPacket(buildPacket(TOPIC_IDS.RX.SYS_EVENT, Array.from(payload)));
  };

  const stopStep = async () => {
    if (testTimer) {
      clearTimeout(testTimer);
      setTestTimer(null);
    }
    
    // Stop all motors
    const motors = selectedMotor === 'all' ? [0, 1, 2, 3] : [selectedMotor];
    for (const mIdx of motors) {
      sendPacket(buildPacket(TOPIC_IDS.RX.ACTUATOR_VEL, Encoders.actuatorVel(mIdx, 0)));
      if (motors.length > 1) await new Promise(r => setTimeout(r, 15));
    }
  };

   const runStep = async () => {
    const isPidOn = (pendingPidState !== null ? pendingPidState : appConfig?.pid_enabled);
    console.log('[PidTuner] runStep clicked. isTesting:', isTesting, 'isPidOn:', isPidOn);
    
    if (!isTesting) {
        console.warn('[PidTuner] Cannot run step: Not in TESTING mode.');
        return;
    }
    if (!isPidOn) {
        console.warn('[PidTuner] Cannot run step: PID is DISABLED.');
        return;
    }
    
    // Auto-resume capture
    console.log('[PidTuner] Starting capture and applying step...');
    setIsCapturing(true);

    const val = parseFloat(testRps);
    const motors = selectedMotor === 'all' ? [0, 1, 2, 3] : [selectedMotor];

    // Start step for each selected motor
    for (const mIdx of motors) {
      sendPacket(buildPacket(TOPIC_IDS.RX.ACTUATOR_VEL, Encoders.actuatorVel(mIdx, val)));
      if (motors.length > 1) await new Promise(r => setTimeout(r, 15));
    }

    // Clear existing timer if any
    if (testTimer) clearTimeout(testTimer);

    // Auto-stop after duration + small tail to see settling
    const timer = setTimeout(handleTestCompletion, Math.max(100, testDuration));
    
    setTestTimer(timer);
  };

  const exitTesting = () => {
    console.log('[PidTuner] Exiting test mode and stopping capture...');
    
    // 1. Immediate UI stop
    setIsCapturing(false);

    // 2. Clear any active test timers
    if (testTimer) {
        clearTimeout(testTimer);
        setTestTimer(null);
    }

    // 3. Send general STOP event to transition FSM out of testing
    sendPacket(buildPacket(TOPIC_IDS.RX.SYS_EVENT, [SYS_EVENTS.STOP]));

    // 4. Force zero velocities as a safety backup
    sendPacket(buildPacket(TOPIC_IDS.RX.ACTUATOR_VEL, Encoders.actuatorVel(0, 0)));
    sendPacket(buildPacket(TOPIC_IDS.RX.ACTUATOR_VEL, Encoders.actuatorVel(1, 0)));
    sendPacket(buildPacket(TOPIC_IDS.RX.ACTUATOR_VEL, Encoders.actuatorVel(2, 0)));
    sendPacket(buildPacket(TOPIC_IDS.RX.ACTUATOR_VEL, Encoders.actuatorVel(3, 0)));
  };

  if (!appConfig) {
    return (
      <div className="pid-tuner-container">
        <div className="tuner-header centered" style={{ flexDirection: 'column', gap: '20px', padding: '40px' }}>
          <Info size={48} color="var(--accent-amber)" />
          <div style={{ textAlign: 'center' }}>
            <h2>Configuration Required</h2>
            <p className="description">Please fetch the robot configuration to enable PID tuning.</p>
          </div>
          <button className="btn btn-primary" onClick={() => sendPacket(buildPacket(TOPIC_IDS.RX.GET_CONFIG))}>
            Fetch Configuration
          </button>
        </div>
      </div>
    );
  }

  return (
    <div className="pid-tuner-container">
      <div className="tuner-header">
        <div className="tuner-title-section">
          <div className="icon-box">
             <Target size={24} className="icon-pulse" />
          </div>
          <div>
            <h1>PID OPTIMIZER</h1>
            <p className="description" style={{ fontSize: '0.75rem', opacity: 0.6 }}>High-frequency transient response analysis</p>
          </div>
        </div>

        <div className="motor-selector-dock">
          <button 
            className={`motor-dock-btn ${selectedMotor === 'all' ? 'active' : ''}`}
            onClick={() => setSelectedMotor('all')}
          >
            ALL MOTORS
          </button>
          {[0, 1, 2, 3].map(i => (
            <button 
              key={i} 
              className={`motor-dock-btn ${selectedMotor === i ? 'active' : ''}`}
              onClick={() => setSelectedMotor(i)}
            >
              M{i + 1}
            </button>
          ))}
        </div>

        <div className="safety-limit-dock" title="Maximum allowed velocity defined in system configuration">
           <ShieldCheck size={14} />
           <div className="limit-info">
             <span className="limit-label">SAFETY LIMIT</span>
             <span className="limit-value">{appConfig.motor_rps_limit?.toFixed(1) || '--'} <small>RPS</small></span>
           </div>
        </div>

        <div className="status-control-dock">
           {isTesting ? (
             <button 
                className="btn btn-sm btn-ghost" 
                onClick={exitTesting}
                style={{ color: 'var(--accent-rose)', borderColor: 'var(--accent-rose)' }}
             >
                <Power size={14} /> EXIT TESTING
             </button>
           ) : (
             <button 
                className={`btn btn-sm ${canEnterTest ? 'btn-primary' : 'btn-disabled'}`}
                onClick={startTesting}
                disabled={disabled || !canEnterTest}
                title={!canEnterTest ? "System must be in MANUAL or AUTO mode to enable testing" : "Enable Testing Mode"}
             >
                <Power size={14} /> ENABLE TESTING
             </button>
           )}

           <div style={{ width: '1px', height: '24px', background: 'var(--border-color)', margin: '0 8px' }}></div>

           <button 
             className={`pid-status-toggle-btn ${(pendingPidState !== null ? pendingPidState : appConfig?.pid_enabled) ? 'active' : 'inactive'}`}
             onClick={() => {
                const newState = (pendingPidState !== null ? pendingPidState : appConfig?.pid_enabled) ? 0 : 1;
                setPendingPidState(newState);
                sendPacket(buildPacket(TOPIC_IDS.RX.SET_CONFIG, Encoders.setConfig(0x10, newState)));
                // Clear pending state after 2 seconds as a fallback
                setTimeout(() => setPendingPidState(null), 2000);
             }}
             title={appConfig?.pid_enabled ? 'Click to Disable PID' : 'Click to Enable PID'}
           >
             <div className="status-dot"></div>
             <span>PID: {(pendingPidState !== null ? pendingPidState : appConfig?.pid_enabled) ? 'ENABLED' : 'DISABLED'}</span>
           </button>
        </div>
      </div>

      <div className="tuner-main-grid">
        {/* Chart View */}
        <div className="tuner-card">
          <div className="tuner-card-header">
            <h3><Activity size={16} color="var(--accent-cyan)" /> Real-time Response</h3>
            <div style={{ display: 'flex', gap: '8px', alignItems: 'center' }}>
              <button 
                className="btn btn-sm btn-ghost" 
                onClick={() => {
                  if (onClear) onClear();
                  setPersistentData([]);
                }}
                title="Clear charts"
              >
                <RotateCcw size={14} /> CLEAR
              </button>
              
              <div className={`status-badge ${isCapturing ? 'active' : ''}`} style={{
                fontSize: '0.65rem',
                fontWeight: '800',
                padding: '4px 8px',
                borderRadius: '4px',
                background: isCapturing ? 'rgba(16, 185, 129, 0.1)' : 'rgba(255, 255, 255, 0.05)',
                color: isCapturing ? 'var(--accent-emerald)' : 'var(--text-muted)',
                border: `1px solid ${isCapturing ? 'rgba(16, 185, 129, 0.2)' : 'rgba(255, 255, 255, 0.1)'}`,
                display: 'flex',
                alignItems: 'center',
                gap: '6px'
              }}>
                <div className={`status-dot ${isCapturing ? 'pulse' : ''}`} style={{
                  width: '6px',
                  height: '6px',
                  borderRadius: '50%',
                  background: isCapturing ? 'var(--accent-emerald)' : 'var(--text-muted)',
                  boxShadow: isCapturing ? '0 0 8px var(--accent-emerald)' : 'none'
                }}></div>
                {isCapturing ? 'CAPTURING DATA' : 'CAPTURE PAUSED'}
              </div>
            </div>
          </div>

          <div className="chart-container-box">
            <ResponsiveContainer width="100%" height={400}>
              <LineChart data={chartData}>
                <XAxis dataKey="time" hide />
                <YAxis 
                  domain={yDomain} 
                  stroke="#444" 
                  fontSize={11} 
                  tickFormatter={(v) => v.toFixed(1)}
                  label={{ value: 'Velocity (RPS)', angle: -90, position: 'insideLeft', fill: '#666', fontSize: 10 }}
                />
                <YAxis 
                  yAxisId="pwm"
                  orientation="right"
                  domain={[-(appConfig?.motor_pwm_max || 1000), appConfig?.motor_pwm_max || 1000]}
                  stroke="var(--accent-rose)"
                  fontSize={11}
                  tickFormatter={(v) => Math.round(v)}
                  label={{ value: 'Effort (Raw PWM)', angle: 90, position: 'insideRight', fill: 'var(--accent-rose)', fontSize: 10, opacity: 0.8 }}
                />
                <Tooltip 
                   contentStyle={{ background: 'rgba(10,14,23,0.9)', border: '1px solid #333', borderRadius: '8px', fontSize: '12px' }}
                />
                <Legend verticalAlign="top" align="right" iconType="circle" wrapperStyle={{ fontSize: '11px', paddingBottom: '10px' }} />
                
                {selectedMotor === 'all' ? (
                  <>
                    <Line type="monotone" dataKey="m1" name="M1" stroke="var(--accent-cyan)" strokeWidth={2} dot={false} isAnimationActive={false} />
                    <Line type="monotone" dataKey="m2" name="M2" stroke="var(--accent-emerald)" strokeWidth={2} dot={false} isAnimationActive={false} />
                    <Line type="monotone" dataKey="m3" name="M3" stroke="var(--accent-amber)" strokeWidth={2} dot={false} isAnimationActive={false} />
                    <Line type="monotone" dataKey="m4" name="M4" stroke="var(--accent-rose)" strokeWidth={2} dot={false} isAnimationActive={false} />
                    <Line type="stepAfter" dataKey="t1" name="Target" stroke="#fff" strokeWidth={1} strokeDasharray="5 5" dot={false} opacity={0.3} isAnimationActive={false} />
                  </>
                ) : (
                  <>
                    <Line type="stepAfter" dataKey="target" name="Setpoint" stroke="var(--accent-cyan)" strokeWidth={2} dot={false} isAnimationActive={false} />
                    <Line type="monotone" dataKey="measured" name="Measured" stroke="var(--accent-emerald)" strokeWidth={2.5} dot={false} isAnimationActive={false} />
                    <Line yAxisId="pwm" type="monotone" dataKey="pwm" name="Raw PWM" stroke="var(--accent-rose)" strokeWidth={1} strokeDasharray="4 4" dot={false} isAnimationActive={false} />
                  </>
                )}
              </LineChart>
            </ResponsiveContainer>
          </div>

          {analysis && (
            <div className="analysis-board">
              <div className="status-hud-v2">
                <div className={`hud-status-badge-v2 ${analysis.status}`}>
                  {analysis.icon}
                  <span>{analysis.suggestion}</span>
                </div>
              </div>

              <table className="analysis-table">
                <thead>
                  <tr>
                    <th>PARAMETER</th>
                    <th>VALUE</th>
                    <th>STABILITY</th>
                  </tr>
                </thead>
                <tbody>
                  <tr>
                    <td>Overshoot</td>
                    <td className="value-cell">{analysis.overshoot?.toFixed(1)}%</td>
                    <td>{analysis.overshoot < 10 ? '✅ Optimal' : '⚠️ High'}</td>
                  </tr>
                  <tr>
                    <td>Rise Time</td>
                    <td className="value-cell">{analysis.riseTime}</td>
                    <td>{analysis.riseTime !== 'N/A' ? '⏱️ Measured' : '--'}</td>
                  </tr>
                  <tr>
                    <td>Steady State Error</td>
                    <td className="value-cell">{analysis.error?.toFixed(3)}</td>
                    <td>{Math.abs(analysis.error) < 0.05 ? '✅ Good' : '❌ Offset'}</td>
                  </tr>
                  <tr>
                    <td>Peak Measured</td>
                    <td className="value-cell">{analysis.peak?.toFixed(2)}</td>
                    <td>RPS</td>
                  </tr>
                </tbody>
              </table>
            </div>
          )}
        </div>

        {/* Controls Panel */}
        <div className="tuner-card">
          <div className="tuner-card-header">
            <h3><Gauge size={16} color="var(--accent-violet)" /> Parameters</h3>
          </div>
          <div className="control-panel">
            {(selectedMotor === 'all' ? [0, 1, 2, 3] : [selectedMotor]).map(mIdx => (
              <div className="motor-param-group" key={mIdx}>
                <div className="motor-group-tag">MOTOR {mIdx + 1} CONFIGURATION</div>
                <GainSlider 
                    label="KP" 
                    value={appConfig?.[`motor${mIdx+1}_kp`] || 0} 
                    min={0} max={40} step={0.01} 
                    onSend={(v) => handleSendParam('kp', v, mIdx)} 
                />
                <GainSlider 
                    label="KI" 
                    value={appConfig?.[`motor${mIdx+1}_ki`] || 0} 
                    min={0} max={40} step={0.01} 
                    onSend={(v) => handleSendParam('ki', v, mIdx)} 
                />
                <GainSlider 
                    label="KD" 
                    value={appConfig?.[`motor${mIdx+1}_kd`] || 0} 
                    min={0} max={20} step={0.01} 
                    onSend={(v) => handleSendParam('kd', v, mIdx)} 
                />
                <GainSlider 
                    label="DZ" 
                    value={appConfig?.[`motor${mIdx+1}_deadzone`] || 0} 
                    min={0} max={30000} step={100} 
                    onSend={(v) => handleSendParam('deadzone', v, mIdx)} 
                />
              </div>
            ))}

            <div className="tuner-actions-dock">
              {!isTesting ? (
                <div className="testing-required-notice">
                   <Power size={24} style={{ marginBottom: '8px', opacity: 0.5 }} />
                   <p>Optimizer locked. You must enable <strong>TESTING</strong> mode to run tests.</p>
                   {!canEnterTest ? (
                     <p className="notice-sub" style={{ color: 'var(--accent-rose)', fontWeight: 'bold' }}>
                       ⚠️ System must be in MANUAL or AUTO state first.
                     </p>
                   ) : (
                     <p className="notice-sub">Click 'ENABLE TESTING' in the header to proceed.</p>
                   )}
                </div>
              ) : (
                <div className="step-test-section">
                  <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
                    <span className="section-subtitle">STEP RESPONSE TEST (RPS)</span>
                    <TooltipWrapper content="Applies a sudden velocity change to analyze controller performance (Overshoot, Settling Time).">
                      <Info size={12} className="info-icon-dim" />
                    </TooltipWrapper>
                  </div>
                   <div className="step-input-row" style={{ marginTop: '8px' }}>
                     <div className="input-with-label">
                       <span>Amplitude (RPS)</span>
                       <input 
                          type="number" 
                          value={testRps} 
                          placeholder="RPS"
                          onChange={(e) => setTestRps(parseFloat(e.target.value))}
                       />
                     </div>
                     <div className="input-with-label">
                       <span>Duration (ms)</span>
                       <input 
                          type="number" 
                          value={testDuration} 
                          placeholder="ms"
                          onChange={(e) => setTestDuration(parseInt(e.target.value) || 0)}
                       />
                     </div>
                  </div>
                  <div className="step-input-row" style={{ marginTop: '8px' }}>
                     <button 
                        className={`btn ${!(pendingPidState !== null ? pendingPidState : appConfig?.pid_enabled) ? 'btn-disabled' : 'btn-primary'}`} 
                        style={{ flex: 1 }} 
                        onClick={runStep}
                        disabled={!(pendingPidState !== null ? pendingPidState : appConfig?.pid_enabled)}
                     >
                       <Play size={14}/> {!(pendingPidState !== null ? pendingPidState : appConfig?.pid_enabled) ? 'ENABLE PID FIRST' : (testTimer ? 'RESTART' : 'GO')}
                     </button>
                     <button className="btn btn-ghost" onClick={stopStep}><CircleStop size={14}/>STOP</button>
                  </div>
                </div>
              )}
              
              <button className="btn btn-accent full-width" style={{ marginTop: '12px' }} onClick={() => sendPacket(buildPacket(TOPIC_IDS.RX.SAVE_CONFIG))}>
                 <Save size={14} /> PERSIST TO FLASH
              </button>

              <div className="floating-hint">
                <Info size={12} />
                <span>Save PID values to flash memory</span>
              </div>
            </div>
          </div>
        </div>
      </div>

      {/* Full-width History Log */}
      <div className="tuner-card history-card" style={{ marginTop: '24px' }}>
        <div className="tuner-card-header">
          <h3><Activity size={16} color="var(--accent-amber)" /> Tuning Session History</h3>
          <button 
            className="btn btn-sm btn-ghost" 
            onClick={() => setTuningHistory([])}
            disabled={tuningHistory.length === 0}
          >
            CLEAR HISTORY
          </button>
        </div>
        <div className="history-table-wrapper">
          <table className="history-table">
            <thead>
              <tr>
                <th>TIME</th>
                <th>MOTOR</th>
                <th>PARAMETERS (P/I/D)</th>
                <th>OVERSHOOT</th>
                <th>ERROR</th>
                <th>STATUS</th>
              </tr>
            </thead>
            <tbody>
              {tuningHistory.length === 0 ? (
                <tr>
                  <td colSpan="6" style={{ textAlign: 'center', padding: '40px', color: 'var(--text-muted)' }}>
                    No tests recorded in this session. Click 'GO' to start capturing results.
                  </td>
                </tr>
              ) : (
                tuningHistory.map(entry => (
                  <tr key={entry.id}>
                    <td className="time">{entry.timestamp}</td>
                    <td><span className="motor-chip">{entry.motor}</span></td>
                    <td className="params">
                      <span>{entry.kp?.toFixed(3)}</span> / 
                      <span>{entry.ki?.toFixed(3)}</span> / 
                      <span>{entry.kd?.toFixed(3)}</span>
                    </td>
                    <td className={`value ${entry.overshoot > 10 ? 'bad' : 'good'}`}>
                      {entry.overshoot?.toFixed(1)}%
                    </td>
                    <td className="value">{entry.error?.toFixed(3)}</td>
                    <td>
                      <div className={`status-pill ${entry.status}`}>
                        {entry.status === 'good' ? 'OPTIMAL' : entry.status === 'warn' ? 'TUNING' : 'UNSTABLE'}
                      </div>
                    </td>
                  </tr>
                ))
              )}
            </tbody>
          </table>
          </div>
        </div>
      </div>
    );
}
