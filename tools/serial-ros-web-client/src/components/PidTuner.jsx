import React, { useState, useEffect, useMemo, useRef } from 'react';
import { 
  LineChart, Line, XAxis, YAxis, CartesianGrid, Tooltip, ResponsiveContainer, Legend 
} from 'recharts';
import { 
  Settings2, Play, CircleStop, Save, 
  Gauge, Zap, AlertCircle, Info, TrendingUp, Target,
  Activity, ShieldCheck, Send, Power, RotateCcw, Download, Navigation, Cpu,
  ChevronUp, ChevronDown, CheckCircle
} from 'lucide-react';
import { TOPIC_IDS, Encoders, buildPacket, SYS_EVENTS } from '../utils/protocol';
import SystemEventsControl from './SystemEventsControl';
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
export default function PidTuner({ 
  history, appConfig, sendPacket, connected, sysStatus, onClear,
  tuningHistory, setTuningHistory, 
  persistentData, setPersistentData,
  settings, setSettings
}) {
  const { speed: testSpeed, lead: testLead, duration: testDuration, tail: testTail, motor: selectedMotor } = settings;
  
  const updateSetting = (key, value) => {
    setSettings(prev => ({ ...prev, [key]: value }));
  };

  const setTestSpeed = (v) => updateSetting('speed', v);
  const setTestLead = (v) => updateSetting('lead', v);
  const setTestDuration = (v) => updateSetting('duration', v);
  const setTestTail = (v) => updateSetting('tail', v);
  const setSelectedMotor = (v) => updateSetting('motor', v);

  const [testTimer, setTestTimer] = useState(null);
  const [isCapturing, setIsCapturing] = useState(false);
  const [lastAutoEnable, setLastAutoEnable] = useState(0);
  const [isInternalTransitioning, setIsInternalTransitioning] = useState(false);
  const [pendingPidState, setPendingPidState] = useState(null); // null, 0, or 1
  const hasAutoEnabledRef = useRef(false);
  const captureStartTsRef = useRef(0);

  const isTesting = sysStatus?.state === 6; /* STATE_SUPERVISOR_TESTING */
  const canEnterTest = sysStatus?.state === 2 || sysStatus?.state === 3;
  const disabled = !connected;

  // Reactive cleanup: if we leave TESTING state (e.g. from external STOP), 
  // ensure PidTuner internal capture and timers are cleared.
  useEffect(() => {
    if (!isTesting) {
      if (isCapturing) {
        console.log('[PidTuner] System left TESTING state. Stopping capture.');
        setIsCapturing(false);
      }
      if (testTimer) {
        console.log('[PidTuner] System left TESTING state. Clearing test timer.');
        clearTimeout(testTimer);
        setTestTimer(null);
      }
    }
  }, [isTesting, isCapturing, testTimer]);

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
    const baseTs = captureStartTsRef.current || (history.length > 0 ? history[0].timestamp : Date.now());
    return history.map(p => {
      const target = p.pid_target?.[selectedMotor === 'all' ? 0 : selectedMotor] || 0;
      const measured = p.pid_measured?.[selectedMotor === 'all' ? 0 : selectedMotor] || 0;
      const relativeTs = ((p.timestamp - baseTs) / 1000).toFixed(3);
      return {
        time: p.timeLabel,
        relativeTime: `T + ${relativeTs}s`,
        ts: p.timestamp,
        target,
        measured,
        error: target - measured,
        pwm: p.pid_pwm?.[selectedMotor === 'all' ? 0 : selectedMotor] || 0,
        // Individual traces for 'ALL' mode
        t1: p.pid_target?.[0], m1: p.pid_measured?.[0], e1: (p.pid_target?.[0] || 0) - (p.pid_measured?.[0] || 0),
        t2: p.pid_target?.[1], m2: p.pid_measured?.[1], e2: (p.pid_target?.[1] || 0) - (p.pid_measured?.[1] || 0),
        t3: p.pid_target?.[2], m3: p.pid_measured?.[2], e3: (p.pid_target?.[2] || 0) - (p.pid_measured?.[2] || 0),
        t4: p.pid_target?.[3], m4: p.pid_measured?.[3], e4: (p.pid_target?.[3] || 0) - (p.pid_measured?.[3] || 0),
      };
    }).slice(-300);
  }, [history, selectedMotor]);

  useEffect(() => {
    if (isCapturing && currentTelemetryData.length > 0) {
      setPersistentData(prev => {
        const lastTs = prev.length > 0 ? prev[prev.length - 1].ts : 0;
        const minTs = Math.max(captureStartTsRef.current, lastTs);
        const newPoints = currentTelemetryData.filter(p => p.ts > minTs);
        if (newPoints.length === 0) return prev;
        
        // Cap persistent data to 1000 points (~20s at 50Hz) to prevent OOM
        const combined = [...prev, ...newPoints];
        return combined.length > 1000 ? combined.slice(-1000) : combined;
      });
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
    
    // Calculate Total Error (Integral of Absolute Error)
    // Formula: Sum(|target - measured|) * dt
    const rawTotalError = chartData.reduce((acc, p) => acc + Math.abs(p.target - p.measured), 0);
    const iae = rawTotalError * 0.02; // Assuming 50Hz

    let status = 'good';
    let suggestion = 'Locked & Loaded';
    let iconType = 'shield';

    const recommendations = [];

    // Rule 1: Rise Time (KP)
    if (riseTime === null || (riseTime * 20) > 150) {
        recommendations.push({ 
            param: 'KP', 
            action: 'INCREASE', 
            trend: 'up',
            reason: 'Response is sluggish. Increasing KP will improve rise time.' 
        });
        status = 'warn';
        suggestion = 'Slow Response';
        iconType = 'trending';
    }

    // Rule 2: Overshoot (KP/KD)
    if (overshoot > 12) {
        recommendations.push({ 
            param: 'KP', 
            action: 'DECREASE', 
            trend: 'down',
            reason: 'High overshoot detected. Reducing KP will soften the impact.' 
        });
        recommendations.push({ 
            param: 'KD', 
            action: 'INCREASE', 
            trend: 'up',
            reason: 'Increase KD to add damping and reduce the overshoot peak.' 
        });
        status = 'crit';
        suggestion = 'High Overshoot';
        iconType = 'alert';
    } else if (overshoot > 5) {
        recommendations.push({ 
            param: 'KD', 
            action: 'FINE-TUNE', 
            trend: 'up',
            reason: 'Slight overshoot. Minor increase in KD could dampen the response.' 
        });
    }

    // Rule 3: Steady State Error (KI)
    if (Math.abs(error) > (target * 0.04)) {
        recommendations.push({ 
            param: 'KI', 
            action: 'INCREASE', 
            trend: 'up',
            reason: 'System fails to reach target. Increase KI to eliminate steady-state error.' 
        });
        if (status !== 'crit') status = 'warn';
        suggestion = 'Steady State Error';
        iconType = 'activity';
    }

    // Rule 4: System Stability / Oscillations
    const noiseLevel = chartData.reduce((acc, p, i) => i > 0 ? acc + Math.abs(p.measured - chartData[i-1].measured) : acc, 0) / chartData.length;
    if (noiseLevel > 0.5) {
        recommendations.push({ 
            param: 'KD', 
            action: 'DECREASE', 
            trend: 'down',
            reason: 'High measurement noise. Reduce KD to prevent jittery effort.' 
        });
    }

    if (recommendations.length === 0) {
        recommendations.push({ 
            param: 'ALL', 
            action: 'OPTIMAL', 
            trend: 'check',
            reason: 'Parameters are well balanced for this operating point.' 
        });
    }

    return { 
        overshoot: Math.max(0, overshoot), 
        error, 
        status, 
        suggestion, 
        iconType,
        peak: maxVal,
        riseTime: riseTime ? `${riseTime * 20}ms` : 'N/A', // assuming 50Hz (20ms/sample)
        settling: Math.abs(error) < (target * 0.02) ? 'Stable' : 'Unstable',
        totalError: rawTotalError,
        iae: iae,
        recommendations
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
                iae: analysis.iae,
                status: analysis.status,
                recommendations: analysis.recommendations,
                data: [...persistentData] // Capture the raw data for export
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
        console.log(`[PidTuner] ${testTail}ms tail finished. Stopping capture.`);
        setIsCapturing(false);
    }, Math.max(0, testTail));
  };

  const yDomain = useMemo(() => {
    const limit = appConfig?.motor_speed_limit || 5.0;
    return [-limit, limit];
  }, [appConfig?.motor_speed_limit]);

  // Handle auto-clamping of test speed when limit changes
  useEffect(() => {
    const limit = appConfig?.motor_speed_limit || 5.0;
    if (testSpeed > limit) {
        console.log(`[PidTuner] Clamping test speed (${testSpeed}) to safety limit (${limit})`);
        setTestSpeed(limit);
    }
  }, [appConfig?.motor_speed_limit, testSpeed]);

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


  const stopStep = async (stopCapture = false) => {
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

    if (stopCapture) {
      console.log('[PidTuner] Manual stop requested. Terminating capture.');
      setIsCapturing(false);
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
    console.log('[PidTuner] Starting capture sequence...');
    captureStartTsRef.current = Date.now();
    setPersistentData([]); // Clear previous capture data
    setIsCapturing(true);

    // 1. Pre-test lead time for baseline
    if (testLead > 0) {
      await new Promise(r => setTimeout(r, testLead));
    }

    console.log('[PidTuner] Applying step now...');
    const val = parseFloat(testSpeed);
    const motors = selectedMotor === 'all' ? [0, 1, 2, 3] : [selectedMotor];

    // Start step for each selected motor
    for (const mIdx of motors) {
      sendPacket(buildPacket(TOPIC_IDS.RX.ACTUATOR_VEL, Encoders.actuatorVel(mIdx, val)));
      if (motors.length > 1) await new Promise(r => setTimeout(r, 15));
    }

    // Clear existing timer if any
    if (testTimer) clearTimeout(testTimer);

    // Auto-stop after duration
    const timer = setTimeout(handleTestCompletion, Math.max(100, testDuration));
    
    setTestTimer(timer);
  };

  const handleStartAutonomous = () => {
    sendPacket(buildPacket(TOPIC_IDS.RX.AUTONOMOUS, Array.from(Encoders.autonomous(true))));
  };

  const downloadCsv = (filename, csvContent) => {
    const blob = new Blob([csvContent], { type: 'text/csv;charset=utf-8;' });
    const link = document.createElement('a');
    if (link.download !== undefined) {
      const url = URL.createObjectURL(blob);
      link.setAttribute('href', url);
      link.setAttribute('download', filename);
      link.style.visibility = 'hidden';
      document.body.appendChild(link);
      link.click();
      document.body.removeChild(link);
    }
  };

  const exportSectionToCsv = (entry) => {
    if (!entry.data || entry.data.length === 0) return;
    
    let csv = `PID Test Session - ${entry.timestamp}\n`;
    csv += `Motor,${entry.motor}\n`;
    csv += `KP,${entry.kp}\n`;
    csv += `KI,${entry.ki}\n`;
    csv += `KD,${entry.kd}\n`;
    csv += `Overshoot,${entry.overshoot?.toFixed(2)}%\n`;
    csv += `Steady State Error,${entry.error?.toFixed(4)}\n`;
    csv += `Total Error (IAE),${((entry.data.reduce((acc, p) => acc + Math.abs(p.target - p.measured), 0)) * 0.02).toFixed(4)}\n\n`;
    
    csv += "N,Timestamp,Target,Measured,Error,PWM\n";
    entry.data.forEach((p, idx) => {
      csv += `${idx + 1},${p.ts},${p.target},${p.measured},${p.error?.toFixed(4) || 0},${p.pwm}\n`;
    });

    const filename = `pid_test_${entry.motor.replace(' ', '_')}_${entry.timestamp.replace(/[: ]/g, '-')}.csv`;
    downloadCsv(filename, csv);
  };

  const exportAllSectionsToCsv = () => {
    if (tuningHistory.length === 0) return;
    
    let csv = "Section,N,Timestamp,Motor,KP,KI,KD,Target,Measured,Error,PWM\n";
    tuningHistory.forEach((entry, idx) => {
      const sectionName = `Session_${tuningHistory.length - idx}`;
      entry.data.forEach((p, pIdx) => {
        csv += `${sectionName},${pIdx + 1},${p.ts},${entry.motor},${entry.kp},${entry.ki},${entry.kd},${p.target},${p.measured},${p.error?.toFixed(4) || 0},${p.pwm}\n`;
      });
    });

    const filename = `pid_tuning_full_history_${new Date().toISOString().slice(0, 10)}.csv`;
    downloadCsv(filename, csv);
  };

  const exitTesting = () => {
    console.log('[PidTuner] Manual exit test mode requested...');
    // We only send the event; the reactive useEffect above handles the cleanup
    sendPacket(buildPacket(TOPIC_IDS.RX.SYS_EVENT, [SYS_EVENTS.STOP]));

    // Backup safety stop for all motors
    for (let i = 0; i < 4; i++) {
        sendPacket(buildPacket(TOPIC_IDS.RX.ACTUATOR_VEL, Encoders.actuatorVel(i, 0)));
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

        <div className="safety-limit-dock" title="Maximum allowed velocity defined in system configuration">
           <ShieldCheck size={14} />
           <div className="limit-info">
             <span className="limit-label">SAFETY LIMIT</span>
             <span className="limit-value">{appConfig.motor_speed_limit?.toFixed(2) || '--'} <small>m/s</small></span>
           </div>
        </div>

        <div className="status-control-dock">
           <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
              <ShieldCheck size={14} color="var(--accent-emerald)" />
              <span style={{ fontSize: '0.7rem', color: 'var(--text-muted)', fontWeight: 'bold' }}>SYSTEM ACTIVE</span>
           </div>

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
            <ResponsiveContainer width="100%" height={280}>
              <LineChart 
                data={chartData} 
                syncId="pidTuningSync"
                margin={{ top: 5, right: 10, left: 10, bottom: 5 }}
              >
                <XAxis dataKey="ts" hide />
                <YAxis 
                  domain={yDomain} 
                  stroke="#444" 
                  fontSize={11} 
                  tickFormatter={(v) => v.toFixed(1)}
                  label={{ value: 'Velocity (m/s)', angle: -90, position: 'insideLeft', fill: '#666', fontSize: 10 }}
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
                   trigger="axis"
                   contentStyle={{ background: 'rgba(10,14,23,0.9)', border: '1px solid #333', borderRadius: '8px', fontSize: '12px' }}
                   cursor={{ stroke: 'rgba(255,255,255,0.2)', strokeWidth: 1, strokeDasharray: '4 4' }}
                   labelFormatter={(ts) => {
                     const p = chartData.find(d => d.ts === ts);
                     return p ? p.relativeTime : '—';
                   }}
                   formatter={(value, name) => {
                     if (name === 'Raw PWM') return [Math.round(value), name];
                     return [value.toFixed(4), name];
                   }}
                />
                <Legend verticalAlign="top" align="right" iconType="circle" wrapperStyle={{ fontSize: '11px', paddingBottom: '10px' }} />
                
                {selectedMotor === 'all' ? (
                  <>
                    <Line type="linear" dataKey="m1" name="M1" stroke="var(--accent-cyan)" strokeWidth={2} dot={false} isAnimationActive={false} activeDot={{ r: 6, strokeWidth: 0, fill: 'var(--accent-cyan)' }} />
                    <Line type="linear" dataKey="m2" name="M2" stroke="var(--accent-emerald)" strokeWidth={2} dot={false} isAnimationActive={false} activeDot={{ r: 6, strokeWidth: 0, fill: 'var(--accent-emerald)' }} />
                    <Line type="linear" dataKey="m3" name="M3" stroke="var(--accent-amber)" strokeWidth={2} dot={false} isAnimationActive={false} activeDot={{ r: 6, strokeWidth: 0, fill: 'var(--accent-amber)' }} />
                    <Line type="linear" dataKey="m4" name="M4" stroke="var(--accent-rose)" strokeWidth={2} dot={false} isAnimationActive={false} activeDot={{ r: 6, strokeWidth: 0, fill: 'var(--accent-rose)' }} />
                    <Line type="stepAfter" dataKey="t1" name="Target" stroke="#fff" strokeWidth={1} strokeDasharray="5 5" dot={false} opacity={0.3} isAnimationActive={false} activeDot={false} />
                  </>
                ) : (
                  <>
                    <Line type="stepAfter" dataKey="target" name="Setpoint" stroke="var(--accent-cyan)" strokeWidth={2} dot={false} isAnimationActive={false} activeDot={{ r: 6, strokeWidth: 0, fill: 'var(--accent-cyan)' }} />
                    <Line type="linear" dataKey="measured" name="Measured" stroke="var(--accent-emerald)" strokeWidth={2.5} dot={false} isAnimationActive={false} activeDot={{ r: 6, strokeWidth: 0, fill: 'var(--accent-emerald)' }} />
                    <Line yAxisId="pwm" type="linear" dataKey="pwm" name="Raw PWM" stroke="var(--accent-rose)" strokeWidth={1} strokeDasharray="4 4" dot={false} isAnimationActive={false} activeDot={{ r: 4, strokeWidth: 0, fill: 'var(--accent-rose)' }} />
                  </>
                )}
              </LineChart>
            </ResponsiveContainer>

            <div style={{ padding: '8px 0', borderTop: '1px dashed rgba(255,255,255,0.05)', marginTop: '8px' }}>
               <ResponsiveContainer width="100%" height={160}>
                <LineChart 
                  data={chartData} 
                  syncId="pidTuningSync"
                  margin={{ top: 5, right: 75, left: 10, bottom: 5 }}
                >
                  <CartesianGrid strokeDasharray="3 3" vertical={false} stroke="rgba(255,255,255,0.03)" />
                  <XAxis dataKey="ts" hide />
                  <YAxis 
                     stroke="#444" 
                     fontSize={11} 
                     tickFormatter={(v) => v.toFixed(2)}
                     label={{ value: 'Error', angle: -90, position: 'insideLeft', fill: '#666', fontSize: 10 }}
                  />
                  <Tooltip 
                     trigger="axis"
                     contentStyle={{ background: 'rgba(10,14,23,0.9)', border: '1px solid #333', borderRadius: '8px', fontSize: '12px' }}
                     cursor={{ stroke: 'rgba(255,255,255,0.2)', strokeWidth: 1, strokeDasharray: '4 4' }}
                     labelFormatter={(ts) => {
                       const p = chartData.find(d => d.ts === ts);
                       return p ? p.relativeTime : '—';
                     }}
                     formatter={(value, name) => [value.toFixed(4), name]}
                  />
                    {selectedMotor === 'all' ? (
                      <>
                        <Line type="linear" dataKey="e1" name="E1" stroke="var(--accent-cyan)" strokeWidth={1.5} dot={false} isAnimationActive={false} activeDot={{ r: 4, strokeWidth: 0, fill: 'var(--accent-cyan)' }} />
                        <Line type="linear" dataKey="e2" name="E2" stroke="var(--accent-emerald)" strokeWidth={1.5} dot={false} isAnimationActive={false} activeDot={{ r: 4, strokeWidth: 0, fill: 'var(--accent-emerald)' }} />
                        <Line type="linear" dataKey="e3" name="E3" stroke="var(--accent-amber)" strokeWidth={1.5} dot={false} isAnimationActive={false} activeDot={{ r: 4, strokeWidth: 0, fill: 'var(--accent-amber)' }} />
                        <Line type="linear" dataKey="e4" name="E4" stroke="var(--accent-rose)" strokeWidth={1.5} dot={false} isAnimationActive={false} activeDot={{ r: 4, strokeWidth: 0, fill: 'var(--accent-rose)' }} />
                      </>
                    ) : (
                      <Line type="linear" dataKey="error" name="Tracking Error" stroke="var(--accent-rose)" strokeWidth={2} dot={false} isAnimationActive={false} activeDot={{ r: 6, strokeWidth: 0, fill: 'var(--accent-rose)' }} />
                    )}
                  <Legend verticalAlign="top" align="right" iconType="circle" wrapperStyle={{ fontSize: '10px' }} />
                </LineChart>
              </ResponsiveContainer>
            </div>
          </div>

          {analysis && (
            <div className="analysis-board">
              <div className="status-hud-v2">
                <div className={`hud-status-badge-v2 ${analysis.status}`}>
                  {analysis.iconType === 'shield' && <ShieldCheck size={16} />}
                  {analysis.iconType === 'trending' && <TrendingUp size={16} />}
                  {analysis.iconType === 'alert' && <AlertCircle size={16} />}
                  {analysis.iconType === 'activity' && <Activity size={16} />}
                  <span>{analysis.suggestion}</span>
                </div>
              </div>

              <div className="advisor-section" style={{ 
                margin: '16px 0', 
                padding: '12px', 
                background: 'rgba(255,255,255,0.02)', 
                borderRadius: '8px', 
                border: '1px solid rgba(255,255,255,0.05)' 
              }}>
                <div style={{ fontSize: '0.65rem', fontWeight: '800', color: 'var(--text-muted)', marginBottom: '8px', letterSpacing: '0.05em' }}>
                  EXPERT ADVISOR RECOMMENDATIONS
                </div>
                <div className="advisor-list" style={{ display: 'flex', flexDirection: 'column', gap: '8px' }}>
                  {analysis.recommendations.map((rec, idx) => (
                    <div key={idx} className="advisor-item" style={{ 
                      display: 'flex', 
                      alignItems: 'flex-start', 
                      gap: '10px',
                      padding: '8px',
                      background: 'rgba(255,255,255,0.02)',
                      borderRadius: '6px'
                    }}>
                      <div className={`rec-badge ${rec.trend}`} style={{
                        padding: '4px 8px',
                        borderRadius: '4px',
                        fontSize: '0.7rem',
                        fontWeight: '700',
                        background: rec.trend === 'up' ? 'rgba(16, 185, 129, 0.1)' : rec.trend === 'down' ? 'rgba(239, 68, 68, 0.1)' : 'rgba(59, 130, 246, 0.1)',
                        color: rec.trend === 'up' ? 'var(--accent-emerald)' : rec.trend === 'down' ? 'var(--accent-rose)' : 'var(--accent-cyan)',
                        border: `1px solid ${rec.trend === 'up' ? 'rgba(16, 185, 129, 0.2)' : rec.trend === 'down' ? 'rgba(239, 68, 68, 0.2)' : 'rgba(59, 130, 246, 0.2)'}`,
                        minWidth: '70px',
                        textAlign: 'center',
                        display: 'flex',
                        alignItems: 'center',
                        justifyContent: 'center',
                        gap: '4px'
                      }}>
                        {rec.trend === 'up' && <ChevronUp size={10} />}
                        {rec.trend === 'down' && <ChevronDown size={10} />}
                        {rec.trend === 'check' && <CheckCircle size={10} />}
                        {rec.param}
                      </div>
                      <div style={{ flex: 1 }}>
                        <div style={{ fontSize: '0.75rem', fontWeight: '700', marginBottom: '2px' }}>{rec.action}</div>
                        <div style={{ fontSize: '0.7rem', color: 'var(--text-muted)', lineHeight: '1.4' }}>{rec.reason}</div>
                      </div>
                    </div>
                  ))}
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
                    <td>Total Error (IAE)</td>
                    <td className="value-cell">{analysis.iae?.toFixed(3)}</td>
                    <td>Score</td>
                  </tr>
                  <tr>
                    <td>Peak Measured</td>
                    <td className="value-cell">{analysis.peak?.toFixed(2)}</td>
                    <td>m/s</td>
                  </tr>
                </tbody>
              </table>
            </div>
          )}
        </div>

        {/* Controls Panel */}
        <div className="tuner-controls-column">
          <SystemEventsControl sendPacket={sendPacket} connected={connected} />

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
                    <span className="section-subtitle">STEP RESPONSE TEST (m/s)</span>
                    <TooltipWrapper content="Applies a sudden velocity change to analyze controller performance (Overshoot, Settling Time).">
                      <Info size={12} className="info-icon-dim" />
                    </TooltipWrapper>
                  </div>
                   <div className="step-input-row" style={{ marginTop: '8px', gap: '12px' }}>
                     <div className="input-with-label" style={{ flex: '1.2' }}>
                       <span>Setpoint (m/s)</span>
                       <input 
                          type="number" 
                          value={testSpeed} 
                          placeholder="m/s"
                          onChange={(e) => setTestSpeed(parseFloat(e.target.value))}
                       />
                     </div>
                     <div className="input-with-label">
                       <span>Stop Before (ms)</span>
                       <input 
                          type="number" 
                          value={testLead} 
                          placeholder="ms"
                          onChange={(e) => setTestLead(parseInt(e.target.value) || 0)}
                       />
                     </div>
                     <div className="input-with-label">
                       <span>Step Duration (ms)</span>
                       <input 
                          type="number" 
                          value={testDuration} 
                          placeholder="ms"
                          onChange={(e) => setTestDuration(parseInt(e.target.value) || 0)}
                       />
                     </div>
                     <div className="input-with-label">
                       <span>Stop After (ms)</span>
                       <input 
                          type="number" 
                          value={testTail} 
                          placeholder="ms"
                          onChange={(e) => setTestTail(parseInt(e.target.value) || 0)}
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
                     <button className="btn btn-ghost" onClick={() => stopStep(true)}><CircleStop size={14}/>STOP</button>
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
      </div>

      {/* Full-width History Log */}
      <div className="tuner-card history-card" style={{ marginTop: '24px' }}>
        <div className="tuner-card-header">
          <h3><Activity size={16} color="var(--accent-amber)" /> Tuning Session History</h3>
          <div style={{ display: 'flex', gap: '8px' }}>
            <button 
              className="btn btn-sm btn-ghost" 
              onClick={exportAllSectionsToCsv}
              disabled={tuningHistory.length === 0}
              title="Export all sessions to a single CSV"
            >
              <Download size={14} /> EXPORT ALL
            </button>
            <button 
              className="btn btn-sm btn-ghost" 
              onClick={() => setTuningHistory([])}
              disabled={tuningHistory.length === 0}
            >
              CLEAR HISTORY
            </button>
          </div>
        </div>
        <div className="history-table-wrapper">
          <table className="history-table">
            <thead>
              <tr>
                <th>TIME</th>
                <th>MOTOR</th>
                <th>PARAMETERS (P/I/D)</th>
                <th>OVERSHOOT</th>
                <th>TOTAL ERROR (IAE)</th>
                <th>ERROR</th>
                <th>STATUS</th>
                <th className="action-cell">ACTION</th>
              </tr>
            </thead>
            <tbody>
              {tuningHistory.length === 0 ? (
                <tr>
                  <td colSpan="8" style={{ textAlign: 'center', padding: '40px', color: 'var(--text-muted)' }}>
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
                    <td className="value">{entry.iae?.toFixed(3)}</td>
                    <td className="value">{entry.error?.toFixed(3)}</td>
                    <td>
                      <div className={`status-pill ${entry.status}`}>
                        {entry.status === 'good' ? 'OPTIMAL' : entry.status === 'warn' ? 'TUNING' : 'UNSTABLE'}
                      </div>
                    </td>
                    <td className="action-cell">
                      <button 
                         className="btn btn-sm btn-ghost" 
                         onClick={() => exportSectionToCsv(entry)}
                         title="Download CSV for this session"
                         style={{ padding: '4px 8px' }}
                      >
                        <Download size={14} />
                      </button>
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
