import { useState, useCallback } from 'react';
import { Settings2, Send, Power } from 'lucide-react';
import { buildPacket, Encoders, TOPIC_IDS } from '../utils/protocol';

/**
 * Isolated Sub-component to prevent remounting issues and allow smooth typing.
 * Now receives value and setter from parent to allow central reset.
 */
const MotorSlider = ({ id, label, disabled, onSend, value, setValue, config }) => {
  const [tempInput, setTempInput] = useState(value.toString());

  // Effect to sync when parent forces a value change (like STOP)
  if (tempInput !== value.toString() && document.activeElement !== document.getElementById(`input-${id}`)) {
    setTempInput(value.toString());
  }

  const handleInputChange = (e) => {
    const val = e.target.value;
    setTempInput(val);
    const num = parseFloat(val);
    if (!isNaN(num)) {
      setValue(num);
    }
  };

  const handleInputBlur = () => {
    setTempInput(value.toString());
  };

  const handleKeyDown = (e) => {
    if (e.key === 'Enter') {
      onSend(id, value);
    }
  };

  const handleSliderChange = (e) => {
    const val = parseFloat(e.target.value);
    setValue(val);
    setTempInput(val.toString());
  };

  return (
    <div className="slider-group" style={{ marginBottom: '16px' }}>
      <div className="slider-label" style={{ display: 'flex', justifyContent: 'space-between', marginBottom: '8px', alignItems: 'center' }}>
        <span style={{ fontSize: '0.8rem', fontWeight: '600' }}>{label}</span>
        <div style={{ display: 'flex', gap: '8px', alignItems: 'center' }}>
          <input 
            id={`input-${id}`}
            type="text" 
            className="value-input" 
            value={tempInput} 
            onChange={handleInputChange}
            onBlur={handleInputBlur}
            onKeyDown={handleKeyDown}
            style={{ 
              width: '80px', 
              background: 'rgba(255,255,255,0.05)', 
              border: '1px solid var(--border-color)',
              borderRadius: '4px',
              color: 'var(--text-primary)',
              padding: '2px 8px',
              fontSize: '0.8rem',
              fontFamily: 'monospace'
            }}
          />
          <button 
            className="btn btn-primary"
            style={{ padding: '4px 8px' }}
            onClick={() => onSend(id, value)}
            disabled={disabled}
            title="Send Command"
          >
            <Send size={12} />
          </button>
        </div>
      </div>
      <input 
        type="range" 
        min={config.min} 
        max={config.max} 
        step={config.step} 
        value={value}
        onChange={handleSliderChange}
        disabled={disabled}
        onDoubleClick={() => { setValue(0); onSend(id, 0); }}
        className="custom-range"
        style={{ width: '100%' }}
      />
      <div style={{ display: 'flex', justifyContent: 'space-between', fontSize: '0.6rem', marginTop: '2px', opacity: 0.4, fontFamily: 'monospace' }}>
        <span>{config.min}</span>
        <span>0</span>
        <span>{config.max}</span>
      </div>
    </div>
  );
};

export default function ActuatorControl({ sendPacket, connected, sysStatus, appConfig }) {
  const [testMode, setTestMode] = useState('PWM'); /* 'PWM' or 'VEL' */
  const [m1, setM1] = useState(0);
  const [m2, setM2] = useState(0);
  const [m3, setM3] = useState(0);
  const [m4, setM4] = useState(0);

  const motorValues = [m1, m2, m3, m4];
  const motorSetters = [setM1, setM2, setM3, setM4];

  const disabled = !connected;
  const isTesting = sysStatus?.state === 6; /* STATE_SUPERVISOR_TESTING */
  
  /* Supervisor can enter TEST only from MANUAL(2) or AUTO(3) */
  const canEnterTest = sysStatus?.state === 2 || sysStatus?.state === 3;

  const startTesting = () => {
    if (disabled || !canEnterTest) return;
    const payload = Encoders.sysEvent(0x07); /* SYS_EVENT_TEST */
    sendPacket(buildPacket(TOPIC_IDS.RX.SYS_EVENT, Array.from(payload)));
  };

  const sendTest = (id, value) => {
    if (disabled || !isTesting || isNaN(value)) return;
    
    if (testMode === 'PWM') {
      console.log(`[Diagnostic] Sending Actuator PWM: ID=${id} Pulse=${value}`);
      const payload = Encoders.actuatorTest(id, value);
      sendPacket(buildPacket(TOPIC_IDS.RX.ACTUATOR_PWM, Array.from(payload)));
    } else {
      console.log(`[Diagnostic] Sending Actuator Velocity: ID=${id} m/s=${value}`);
      const payload = Encoders.actuatorVel(id, value);
      sendPacket(buildPacket(TOPIC_IDS.RX.ACTUATOR_VEL, Array.from(payload)));
    }
  };

  const stopMotors = async () => {
    // Reset internal UI state
    setM1(0); setM2(0); setM3(0); setM4(0);
    
    // Send 0 to all motors in current mode
    for (let id = 0; id < 4; id++) {
        if (testMode === 'PWM') {
            const payload = Encoders.actuatorTest(id, 0);
            sendPacket(buildPacket(TOPIC_IDS.RX.ACTUATOR_PWM, Array.from(payload)));
        } else {
            const payload = Encoders.actuatorVel(id, 0);
            sendPacket(buildPacket(TOPIC_IDS.RX.ACTUATOR_VEL, Array.from(payload)));
        }
        await new Promise(resolve => setTimeout(resolve, 20));
    }
  };

  const exitTesting = async () => {
    await stopMotors();
    // Send general stop event (0x02) to exit testing mode in mobility FSM
    const stopPayload = Encoders.sysEvent(0x02); // STOP/IDLE transition
    sendPacket(buildPacket(TOPIC_IDS.RX.SYS_EVENT, Array.from(stopPayload)));
  };

  const sendAll = async () => {
    if (disabled || !isTesting) return;
    for (let id = 0; id < 4; id++) {
      sendTest(id, motorValues[id]);
      // Small delay to prevent overwhelming the serial buffer/STM32 parser
      await new Promise(resolve => setTimeout(resolve, 20));
    }
  };

  const toggleMode = async () => {
    await stopMotors();
    setTestMode(prev => prev === 'PWM' ? 'VEL' : 'PWM');
  };

  const sliderConfig = testMode === 'PWM' ? {
    min: -(appConfig?.motor_pwm_max || 65535),
    max: (appConfig?.motor_pwm_max || 65535),
    step: 100,
    unit: 'Pulse'
  } : {
    min: -(appConfig?.motor_speed_limit || 2.0),
    max: (appConfig?.motor_speed_limit || 2.0),
    step: 0.05,
    unit: 'm/s'
  };

  const motorLabels = ['Front Left', 'Back Left', 'Front Right', 'Back Right'];

  return (
    <div className="card" style={{ maxWidth: '100%', animation: 'fadeIn 0.3s ease' }}>
      <div className="card-header" style={{ display: 'flex', justifyContent: 'space-between', alignItems: 'center' }}>
        <div style={{ display: 'flex', alignItems: 'center', gap: '8px' }}>
          <h3><Settings2 size={16} /> Actuator Diagnostic Tool</h3>
          <span className="card-badge" style={{ 
            background: !isTesting ? 'var(--accent-rose)' : (testMode === 'PWM' ? 'var(--accent-blue)' : 'var(--accent-purple)'),
            color: 'white',
            padding: '2px 6px',
            borderRadius: '4px',
            fontSize: '0.65rem'
          }}>
            {!isTesting ? 'LOCKED' : `${testMode} ACTIVE`}
          </span>
        </div>
        {isTesting ? (
            <button 
              className="btn btn-secondary" 
              onClick={toggleMode}
              disabled={disabled}
              style={{ fontSize: '0.7rem', padding: '4px 8px' }}
            >
              Switch to {testMode === 'PWM' ? 'Velocity' : 'PWM'}
            </button>
        ) : (
            <button 
              className={`btn ${canEnterTest ? 'btn-primary' : 'btn-disabled'}`} 
              onClick={startTesting}
              disabled={disabled || !canEnterTest}
              style={{ fontSize: '0.7rem', padding: '4px 12px' }}
            >
              ENABLE TESTING STATE
            </button>
        )}
      </div>
      <div className="card-body">
        {!isTesting ? (
            <div style={{ 
                padding: '24px', 
                textAlign: 'center', 
                background: 'rgba(0,0,0,0.2)', 
                borderRadius: '8px',
                border: '1px dashed var(--border-color)',
                color: 'var(--text-secondary)'
            }}>
                <Power size={32} style={{ marginBottom: '12px', opacity: 0.5 }} />
                <p style={{ fontSize: '0.9rem' }}>
                    Control is disabled. You must transition to <strong>TESTING</strong> mobility state to use this tool. Transition from Supervision Manual or Autonomous state
                </p>
                {!canEnterTest && (
                    <p style={{ fontSize: '0.75rem', color: 'var(--accent-rose)', marginTop: '8px' }}>
                        Supervisor must be in <strong>MANUAL</strong> or <strong>AUTO</strong> mode first.
                    </p>
                )}
            </div>
        ) : (
            <>
                <div style={{ 
                  marginBottom: '20px', 
                  padding: '12px', 
                  background: 'rgba(251, 191, 36, 0.05)', 
                  border: '1px solid rgba(251, 191, 36, 0.2)',
                  borderRadius: '8px',
                  fontSize: '0.75rem', 
                  color: 'var(--accent-amber)',
                  lineHeight: '1.4'
                }}>
                  <strong>CRITICAL WARNING:</strong> This tool sends raw commands directly to motors.
                  <br/><br/>
                  • Mode: {testMode} ({testMode === 'PWM' ? 'Raw Pulse' : 'Target m/s'}).
                  <br/>
                  • Click **Send** icon to dispatch command.
                </div>

                <button 
                  className="btn btn-primary" 
                  onClick={sendAll}
                  disabled={disabled || !isTesting}
                  style={{ width: '100%', marginBottom: '20px', justifyContent: 'center' }}
                >
                  <Send size={16} /> UPDATE ALL MOTORS
                </button>
                
                <div className="sliders-container">
                    {[0, 1, 2, 3].map(i => (
                        <MotorSlider 
                            key={i}
                            id={i}
                            label={`${motorLabels[i]} (M${i+1})`}
                            disabled={disabled || !isTesting}
                            onSend={sendTest}
                            value={motorValues[i]}
                            setValue={motorSetters[i]}
                            config={sliderConfig}
                        />
                    ))}
                </div>
            </>
        )}

        {isTesting && (
            <div className="btn-group" style={{ marginTop: '24px', display: 'grid', gridTemplateColumns: '1fr 1fr', gap: '12px' }}>
                <button 
                    className="btn btn-secondary" 
                    style={{ justifyContent: 'center', height: '48px', color: 'var(--accent-rose)', borderColor: 'var(--accent-rose)' }} 
                    onClick={stopMotors}
                    disabled={disabled}
                >
                    <Power size={18} /> STOP MOTORS
                </button>
                <button 
                    className="btn btn-estop" 
                    style={{ justifyContent: 'center', height: '48px' }} 
                    onClick={exitTesting}
                    disabled={disabled}
                >
                    <Power size={18} /> EXIT TEST MODE
                </button>
            </div>
        )}
      </div>
    </div>
  );
}
