import { useState, useRef, useEffect } from 'react';
import { AlertTriangle } from 'lucide-react';

import { getActiveErrors } from '../utils/ErrorMapping';

function parseErrorFlags(flags) {
  return getActiveErrors(flags).map(err => err.label);
}

export default function ErrorLogPanel({ sysStatus }) {
  const [history, setHistory] = useState([]);
  const prevFlags = useRef(null);

  useEffect(() => {
    if (!sysStatus) return;
    const flags = sysStatus.errors;
    if (flags === null || flags === undefined) return;
    const bitmask = flags.toString();
    if (bitmask === prevFlags.current) return;
    prevFlags.current = bitmask;
    
    const bigFlags = BigInt(bitmask);
    const activeErrors = parseErrorFlags(bigFlags);
    const entry = {
      ts: Date.now(),
      flags: '0x' + bigFlags.toString(16).toUpperCase().padStart(16, '0'),
      errors: activeErrors,
    };
    setHistory(prev => [entry, ...prev].slice(0, 50));
  }, [sysStatus?.errors]);

  return (
    <div className="card log-panel-full">
      <div className="card-header">
        <h3><AlertTriangle size={14} /> Error Flag Log</h3>
        {sysStatus && (
          <span style={{ fontFamily: 'var(--font-mono)', fontSize: '0.62rem', color: 'var(--accent-rose)' }}>
            {(() => {
              const bitmask = sysStatus?.errors || '0';
              const bigFlags = BigInt(bitmask);
              return '0x' + bigFlags.toString(16).toUpperCase().padStart(16, '0');
            })()}
          </span>
        )}
      </div>
      <div className="card-body" style={{ padding: '8px 12px' }}>
        {history.length === 0 ? (
          <div className="empty-state" style={{ padding: '16px 0' }}>
            <p style={{ fontSize: '0.72rem', color: 'var(--accent-emerald)' }}>✓ No errors recorded</p>
          </div>
        ) : (
          <div className="log-container log-container-horiz">
            {history.map((entry, i) => (
              <div className="error-history-entry" key={i}>
                <span className="error-ts">
                  {new Date(entry.ts).toLocaleTimeString('en-US', { hour12: false })}
                </span>
                <span className="error-flags-val">{entry.flags}</span>
                <div className="error-tags">
                  {entry.errors.length === 0
                    ? <span className="error-tag" style={{ color: 'var(--accent-emerald)', borderColor: 'rgba(52,211,153,0.25)', background: 'rgba(52,211,153,0.08)' }}>OK</span>
                    : entry.errors.map(e => (
                        <span className="error-tag" key={e}>{e}</span>
                      ))
                  }
                </div>
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  );
}
