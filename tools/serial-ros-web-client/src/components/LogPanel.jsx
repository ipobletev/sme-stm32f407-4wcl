import { Trash2, Terminal } from 'lucide-react';

function formatTime(ts) {
  const d = new Date(ts);
  return d.toLocaleTimeString('en-US', { hour12: false }) + '.' + String(d.getMilliseconds()).padStart(3, '0');
}

function topicLabel(id) {
  return '0x' + id.toString(16).padStart(2, '0');
}

export default function LogPanel({ log, onClear }) {
  return (
    <div className="card log-panel-full">
      <div className="card-header">
        <h3><Terminal size={14} /> Packet Log</h3>
        <button className="btn btn-ghost btn-sm btn-icon" onClick={onClear} title="Clear log">
          <Trash2 size={13} />
        </button>
      </div>
      <div className="card-body" style={{ padding: '8px 12px' }}>
        {log.length === 0 ? (
          <div className="empty-state" style={{ padding: '16px 0' }}>
            <p style={{ fontSize: '0.72rem' }}>No packets captured yet</p>
          </div>
        ) : (
          <div className="log-container log-container-horiz">
            {log.map((entry, i) => (
              <div className="log-entry" key={i}>
                <span className="log-time">{formatTime(entry.ts)}</span>
                <span className={`log-dir ${entry.dir.toLowerCase()}`}>{entry.dir}</span>
                <span className="log-topic">{topicLabel(entry.topicId)}</span>
                <span className="log-raw">{entry.raw}</span>
              </div>
            ))}
          </div>
        )}
      </div>
    </div>
  );
}
