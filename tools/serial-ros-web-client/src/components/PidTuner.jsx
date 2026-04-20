import React, { useState, useEffect, useMemo } from 'react';
import { 
  LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, Legend 
} from 'recharts';
import { 
  Settings2, Play, CircleStop, Save, 
  Gauge, Zap, AlertCircle, Info, TrendingUp, Target,
  Activity, ShieldCheck, Send
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
export default function PidTuner({ history, appConfig, sendPacket, connected }) {
  const [selectedMotor, setSelectedMotor] = useState(0); // 0-3
  const [testRps, setTestRps] = useState(1.0);
  const [testDuration, setTestDuration] = useState(2000);
  const [testTimer, setTestTimer] = useState(null);
  const [isCapturing, setIsCapturing] = useState(true);
  const [lastAutoEnable, setLastAutoEnable] = useState(0);

  // Auto-enable PID on mount if disabled
  useEffect(() => {
    if (appConfig && appConfig.pid_enabled === 0 && (Date.now() - lastAutoEnable > 5000)) {
        console.log('[PidTuner] Auto-enabling PID Controller...');
        sendPacket(buildPacket(TOPIC_IDS.RX.SET_CONFIG, Encoders.setConfig(0x10, 1.0)));
        setLastAutoEnable(Date.now());
    }
  }, [appConfig, sendPacket, lastAutoEnable]);

  // Analysis logic
  const chartData = useMemo(() => {
    if (!history || !isCapturing) return [];
    return history.map(p => ({
      time: p.timeLabel,
      // Current motor
      target: p.pid_target?.[selectedMotor === 'all' ? 0 : selectedMotor] || 0,
      measured: p.pid_measured?.[selectedMotor === 'all' ? 0 : selectedMotor] || 0,
      pwm: (p.pid_pwm?.[selectedMotor === 'all' ? 0 : selectedMotor] / 655.35) || 0,
      // Individual traces for 'ALL' mode
      t1: p.pid_target?.[0], m1: p.pid_measured?.[0],
      t2: p.pid_target?.[1], m2: p.pid_measured?.[1],
      t3: p.pid_target?.[2], m3: p.pid_measured?.[2],
      t4: p.pid_target?.[3], m4: p.pid_measured?.[3],
    })).slice(-100);
  }, [history, selectedMotor, isCapturing]);

  // Analysis logic
  const analysis = useMemo(() => {
    if (chartData.length < 20) return null;
    
    const lastPoint = chartData[chartData.length - 1];
    const setpoint = lastPoint.target;
    if (Math.abs(setpoint) < 0.1) return { status: 'idle', msg: 'Iddle' };

    const vals = chartData.map(d => d.measured);
    const maxVal = Math.max(...vals);
    const minVal = Math.min(...vals);
    
    let overshoot = 0;
    if (setpoint > 0) {
        overshoot = Math.max(0, ((maxVal - setpoint) / setpoint) * 100);
    } else {
        overshoot = Math.max(0, ((minVal - setpoint) / setpoint) * 100);
    }

    const recent = chartData.slice(-10);
    const avgMeasured = recent.reduce((sum, d) => sum + d.measured, 0) / recent.length;
    const error = Math.abs(setpoint - avgMeasured);

    let status = 'good';
    let suggestion = 'System stable.';
    let icon = <ShieldCheck size={14} />;

    if (overshoot > 20) {
        status = 'crit';
        suggestion = 'High overshoot. Lower Ki/Kp.';
        icon = <Zap size={14} />;
    } else if (overshoot > 10) {
        status = 'warn';
        suggestion = 'Overshoot detected. Increase Kd.';
        icon = <AlertCircle size={14} />;
    } else if (error > 0.1) {
        status = 'warn';
        suggestion = 'Steady state error. Increase Ki.';
        icon = <TrendingUp size={14} />;
    }

    return { overshoot, error, status, suggestion, icon };
  }, [chartData]);

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

  const runStep = async () => {
    // 1. Clear existing timer if any
    if (testTimer) clearTimeout(testTimer);
    
    // 2. Ensure robot is in TESTING mode (Topic 0x05, Event 0x07)
    sendPacket(buildPacket(TOPIC_IDS.RX.SYS_EVENT, Encoders.sysEvent(SYS_EVENTS.TEST)));
    
    // 3. Prepare motors to update
    const motorsToUpdate = selectedMotor === 'all' ? [0, 1, 2, 3] : [selectedMotor];
    
    // 4. Start step for each motor
    for (const mIdx of motorsToUpdate) {
      sendPacket(buildPacket(TOPIC_IDS.RX.ACTUATOR_VEL, Encoders.actuatorVel(mIdx, testRps)));
      if (motorsToUpdate.length > 1) await new Promise(r => setTimeout(r, 10)); // Prevent UART flooding
    }
    
    // 5. Set auto-stop if duration > 0
    if (testDuration > 0) {
        const timer = setTimeout(() => {
            stopStep();
            setTestTimer(null);
        }, testDuration);
        setTestTimer(timer);
    }
  };

  const stopStep = async () => {
    if (testTimer) {
        clearTimeout(testTimer);
        setTestTimer(null);
    }
    
    const motorsToUpdate = selectedMotor === 'all' ? [0, 1, 2, 3] : [selectedMotor];
    for (const mIdx of motorsToUpdate) {
      sendPacket(buildPacket(TOPIC_IDS.RX.ACTUATOR_VEL, Encoders.actuatorVel(mIdx, 0)));
      if (motorsToUpdate.length > 1) await new Promise(r => setTimeout(r, 10));
    }
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

        <div className="status-control-dock">
           <div className={`pid-status-indicator ${appConfig?.pid_enabled ? 'active' : ''}`}>
             <div className="status-dot"></div>
             <span>PID: {appConfig?.pid_enabled ? 'ENABLED' : 'DISABLED'}</span>
           </div>
           <button 
             className={`btn btn-sm ${appConfig?.pid_enabled ? 'btn-ghost' : 'btn-accent'}`}
             onClick={() => sendPacket(buildPacket(TOPIC_IDS.RX.SET_CONFIG, Encoders.setConfig(0x10, appConfig?.pid_enabled ? 0 : 1)))}
           >
             {appConfig?.pid_enabled ? 'DISABLE' : 'ENABLE'}
           </button>
        </div>
      </div>

      <div className="tuner-main-grid">
        {/* Chart View */}
        <div className="tuner-card">
          <div className="tuner-card-header">
            <h3><Activity size={16} color="var(--accent-cyan)" /> Real-time Response</h3>
            <button 
              className={`btn btn-sm ${isCapturing ? 'btn-danger' : 'btn-primary'}`}
              onClick={() => setIsCapturing(!isCapturing)}
            >
              {isCapturing ? <CircleStop size={14} /> : <Play size={14} />}
              {isCapturing ? 'PAUSE CAPTURE' : 'RESUME CAPTURE'}
            </button>
          </div>

          <div className="chart-container-box">
            <ResponsiveContainer width="100%" height={400}>
              <LineChart data={chartData}>
                <XAxis dataKey="time" hide />
                <YAxis 
                  domain={['auto', 'auto']} 
                  stroke="#444" 
                  fontSize={11} 
                  tickFormatter={(v) => v.toFixed(1)}
                  label={{ value: 'Velocity (RPS)', angle: -90, position: 'insideLeft', fill: '#666', fontSize: 10 }}
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
                    <Line type="monotone" dataKey="pwm" name="PWM %" stroke="var(--accent-rose)" strokeWidth={1} strokeDasharray="4 4" dot={false} isAnimationActive={false} />
                  </>
                )}
              </LineChart>
            </ResponsiveContainer>
          </div>

          {analysis && (
            <div className="status-hud">
              <div className="hud-line">
                <span className="hud-label">OVERSHOOT</span>
                <span className="hud-value" style={{ color: analysis.overshoot > 10 ? 'var(--accent-rose)' : 'var(--accent-emerald)' }}>
                  {analysis.overshoot?.toFixed(1)}%
                </span>
              </div>
              <div className="hud-line">
                <span className="hud-label">SS ERROR</span>
                <span className="hud-value">
                  {analysis.error?.toFixed(3)}
                </span>
              </div>
              <div className={`hud-status-badge ${analysis.status}`}>
                {analysis.icon}
                <span>{analysis.suggestion}</span>
              </div>
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
                    label="DEADZONE (counts)" 
                    value={appConfig?.[`motor${mIdx+1}_deadzone`] || 0} 
                    min={0} max={30000} step={100} 
                    onSend={(v) => handleSendParam('deadzone', v, mIdx)} 
                />
              </div>
            ))}

            <div className="tuner-actions-dock">
              <div className="step-test-section">
                <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
                  <span className="gain-label">STEP RESPONSE TEST (RPS)</span>
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
                <div className="step-input-row">
                   <button className="btn btn-primary" style={{ flex: 1 }} onClick={runStep}>
                     <Play size={14}/> {testTimer ? 'RESTART' : 'GO'}
                   </button>
                   <button className="btn btn-ghost" onClick={stopStep}><CircleStop size={14}/>STOP</button>
                </div>
              </div>
              
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
    </div>
  );
}
