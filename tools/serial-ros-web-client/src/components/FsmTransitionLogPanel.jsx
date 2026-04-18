import { useRef, useEffect } from 'react';
import { ListTree, Trash2 } from 'lucide-react';
import {
  getSupervisorStateName,
  getMobilityStateName,
  getArmStateName,
  formatErrorMask,
} from '../utils/fsmLabels';

function formatTime(ts) {
  const d = new Date(ts);
  const h = String(d.getHours()).padStart(2, '0');
  const m = String(d.getMinutes()).padStart(2, '0');
  const s = String(d.getSeconds()).padStart(2, '0');
  const ms = String(d.getMilliseconds()).padStart(3, '0');
  return `${h}:${m}:${s}.${ms}`;
}

function DeltaCell({ from, to, labelFn }) {
  if (from === to) {
    return <span className="fsm-tlog-same" title="No change">{labelFn(from)}</span>;
  }
  return (
    <span className="fsm-tlog-delta" title={`${from} → ${to}`}>
      <span className="fsm-tlog-from">{labelFn(from)}</span>
      <span className="fsm-tlog-arrow">→</span>
      <span className="fsm-tlog-to">{labelFn(to)}</span>
    </span>
  );
}

function ErrCell({ from, to }) {
  if (from === to) {
    return <span className="fsm-tlog-same mono">{formatErrorMask(to)}</span>;
  }
  return (
    <span className="fsm-tlog-err mono" title="Error mask">
      <span className="fsm-tlog-from">{formatErrorMask(from)}</span>
      <span className="fsm-tlog-arrow">→</span>
      <span className="fsm-tlog-to">{formatErrorMask(to)}</span>
    </span>
  );
}

export default function FsmTransitionLogPanel({ rows, onClear, connected, sysStatus }) {
  const bodyRef = useRef(null);

  useEffect(() => {
    if (!bodyRef.current) return;
    bodyRef.current.scrollTop = 0;
  }, [rows.length]);

  return (
    <div className="card fsm-tlog-card">
      <div className="card-header">
        <h3>
          <ListTree size={16} />
          FSM transition log
        </h3>
        <div className="fsm-tlog-actions">
          {sysStatus && (
            <span className="card-badge" title="Current state (telemetry)">
              SUP {getSupervisorStateName(sysStatus.state)} · MOB {getMobilityStateName(sysStatus.mobility_state)} · ARM{' '}
              {getArmStateName(sysStatus.arm_state)}
            </span>
          )}
          <button
            type="button"
            className="fsm-tlog-clear"
            onClick={onClear}
            disabled={rows.length === 0 && !sysStatus}
            title="Clear table"
          >
            <Trash2 size={14} />
            Clear
          </button>
        </div>
      </div>
      <div className="card-body fsm-tlog-body" ref={bodyRef}>
        {!connected && rows.length === 0 ? (
          <p className="fsm-tlog-empty">Connect the serial port to record transitions.</p>
        ) : rows.length === 0 ? (
          <p className="fsm-tlog-empty">
            {sysStatus
              ? 'No state changes yet. Rows appear when Supervisor, Mobility, Arm, or the error mask changes.'
              : 'Waiting for telemetry. An initial snapshot row appears when the first sys_status frame arrives.'}
          </p>
        ) : (
          <div className="fsm-tlog-table-wrap">
            <table className="fsm-tlog-table">
              <thead>
                <tr>
                  <th>Time</th>
                  <th>Supervisor</th>
                  <th>Mobility</th>
                  <th>Arm</th>
                  <th>Errors</th>
                </tr>
              </thead>
              <tbody>
                {rows.map((r) => (
                  <tr key={r.id}>
                    <td className="fsm-tlog-time" title={formatTime(r.ts)}>
                      {r.kind === 'initial' ? (
                        <span className="fsm-tlog-initial">Initial</span>
                      ) : (
                        formatTime(r.ts)
                      )}
                    </td>
                    <td>
                      <DeltaCell from={r.sup.from} to={r.sup.to} labelFn={getSupervisorStateName} />
                    </td>
                    <td>
                      <DeltaCell from={r.mob.from} to={r.mob.to} labelFn={getMobilityStateName} />
                    </td>
                    <td>
                      <DeltaCell from={r.arm.from} to={r.arm.to} labelFn={getArmStateName} />
                    </td>
                    <td>
                      <ErrCell from={r.errFrom} to={r.errTo} />
                    </td>
                  </tr>
                ))}
              </tbody>
            </table>
          </div>
        )}
      </div>
    </div>
  );
}
