import { useState, useCallback } from 'react';
import { Settings2, Send, Power } from 'lucide-react';
import { buildPacket, Encoders, TOPIC_IDS } from '../utils/protocol';

/**
 * Isolated Sub-component to prevent remounting issues and allow smooth typing.
 * Now receives value and setter from parent to allow central reset.
 */
const MotorSlider = ({ id, label, disabled, onSend, value, setValue }) => {
  const [tempInput, setTempInput] = useState(value.toString());

  // Sync tempInput when value changes from parent (e.g., STOP button)
  const syncInput = useCallback((newVal) => {
    setTempInput(newVal.toString());
  }, []);

  const handleInputChange = (e) => {
    const val = e.target.value;
    setTempInput(val);
    const num = parseInt(val, 10);
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
    const val = parseInt(e.target.value, 10);
    setValue(val);
    setTempInput(val.toString());
  };

  // Effect to sync when parent forces a value change (like STOP)
  if (tempInput !== value.toString() && document.activeElement !== document.getElementById(`input-${id}`)) {
    setTempInput(value.toString());
  }

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
            title="Send Pulse"
          >
            <Send size={12} />
          </button>
        </div>
      </div>
      <input 
        type="range" 
        min={-30000} 
        max={30000} 
        step={500} 
        value={value}
        onChange={handleSliderChange}
        onMouseUp={() => onSend(id, value)}
        disabled={disabled}
        onDoubleClick={() => { setValue(0); onSend(id, 0); }}
        className="custom-range"
        style={{ width: '100%' }}
      />
    </div>
  );
};

export default function ActuatorControl({ sendPacket, connected }) {
  const [m1, setM1] = useState(0);
  const [m2, setM2] = useState(0);
  const [m3, setM3] = useState(0);
  const [m4, setM4] = useState(0);

  const motorValues = [m1, m2, m3, m4];
  const motorSetters = [setM1, setM2, setM3, setM4];

  const disabled = !connected;

  const sendTest = (id, pulse) => {
    if (disabled || isNaN(pulse)) return;
    console.log(`[Diagnostic] Sending Actuator PWM: ID=${id} Pulse=${pulse}`);
    const payload = Encoders.actuatorTest(id, pulse);
    sendPacket(buildPacket(TOPIC_IDS.RX.ACTUATOR_PWM, Array.from(payload)));
  };

  const stopAll = () => {
    // Reset internal UI state
    setM1(0); setM2(0); setM3(0); setM4(0);
    
    // Send 0 pulse to all motors
    [0, 1, 2, 3].forEach(id => {
        const payload = Encoders.actuatorTest(id, 0);
        sendPacket(buildPacket(TOPIC_IDS.RX.ACTUATOR_PWM, Array.from(payload)));
    });

    // Also send general stop event to exit testing mode in firmware
    const stopPayload = Encoders.sysEvent(0x02); // STOP
    sendPacket(buildPacket(TOPIC_IDS.RX.SYS_EVENT, Array.from(stopPayload)));
  };

  return (
    <div className="card" style={{ maxWidth: '100%', animation: 'fadeIn 0.3s ease' }}>
      <div className="card-header">
        <h3><Settings2 size={16} /> Actuator Diagnostic Tool</h3>
        <span className="card-badge">TOPIC 0x06</span>
      </div>
      <div className="card-body">
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
          <strong>CRITICAL WARNING:</strong> This tool sends raw PWM pulses directly to the motor drivers, bypassing all PID controllers and safety limits. Use with caution.
          <br/><br/>
          • Use the slider for rough adjustment.
          <br/>
          • Double-click slider to reset individual motor to 0.
          <br/>
          • Enter a number and press Enter or the button to send.
        </div>
        
        <MotorSlider id={0} label="Motor 1 (Front Left)" disabled={disabled} onSend={sendTest} value={m1} setValue={setM1} />
        <MotorSlider id={1} label="Motor 2 (Back Left)" disabled={disabled} onSend={sendTest} value={m2} setValue={setM2} />
        <MotorSlider id={2} label="Motor 3 (Front Right)" disabled={disabled} onSend={sendTest} value={m3} setValue={setM3} />
        <MotorSlider id={3} label="Motor 4 (Back Right)" disabled={disabled} onSend={sendTest} value={m4} setValue={setM4} />

        <div className="btn-group" style={{ marginTop: '24px' }}>
          <button 
            className="btn btn-estop" 
            style={{ width: '100%', justifyContent: 'center', height: '48px' }} 
            onClick={stopAll}
            disabled={disabled}
          >
            <Power size={18} /> STOP ALL & RELEASE CONTROL
          </button>
        </div>
      </div>
    </div>
  );
}
