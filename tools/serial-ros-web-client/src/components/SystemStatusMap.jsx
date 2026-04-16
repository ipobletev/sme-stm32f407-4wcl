import { motion } from 'framer-motion';
import { Shield, Cpu, Zap, Activity, AlertTriangle, Circle, ArrowRight } from 'lucide-react';

/* --- FSM DEFINITIONS (Based on docs/state_machine.md) --- */

const SUPERVISOR_FSM = {
  nodes: [
    { id: 0, label: 'INIT', x: 80, y: 120, color: '#a78bfa' },
    { id: 1, label: 'IDLE', x: 220, y: 120, color: '#94a3b8' },
    { id: 2, label: 'MANUAL', x: 400, y: 60, color: '#6366f1' },
    { id: 3, label: 'AUTO', x: 400, y: 180, color: '#10b981' },
    { id: 4, label: 'PAUSED', x: 580, y: 120, color: '#f59e0b' },
    { id: 5, label: 'FAULT', x: 320, y: 260, color: '#ef4444' },
  ],
  edges: [
    { from: 0, to: 1, label: 'START' },
    { from: 1, to: 2, label: 'MODE_MAN' },
    { from: 1, to: 3, label: 'MODE_AUTO' },
    { from: 2, to: 4, label: 'PAUSE' },
    { from: 3, to: 4, label: 'PAUSE' },
    { from: 4, to: 1, label: 'STOP' },
    { from: 4, to: 2, label: 'RESUME' },
    { from: 4, to: 3, label: 'RESUME' },
    { from: 1, to: 5, label: 'ERR' },
    { from: 2, to: 5, label: 'ERR' },
    { from: 3, to: 5, label: 'ERR' },
    { from: 4, to: 5, label: 'ERR' },
    { from: 5, to: 0, label: 'RESET' },
  ]
};

const MOBILITY_FSM = {
  nodes: [
    { id: 0, label: 'DISABLED', x: 100, y: 100, color: 'var(--text-muted)' },
    { id: 1, label: 'STOPPED', x: 280, y: 100, color: 'var(--accent-amber)' },
    { id: 2, label: 'MOVING', x: 460, y: 100, color: 'var(--accent-emerald)' },
    { id: 3, label: 'FAULT', x: 280, y: 200, color: 'var(--accent-rose)' },
  ],
  edges: [
    { from: 0, to: 1, label: 'Supervisor Active' },
    { from: 1, to: 2, label: 'Cmd > 0' },
    { from: 2, to: 1, label: 'Cmd = 0' },
    { from: 1, to: 0, label: 'Sup Init/Fault' },
    { from: 2, to: 1, label: 'Sup Idle/Pause' },
  ]
};

const ARM_FSM = {
  nodes: [
    { id: 0, label: 'DISABLED', x: 100, y: 100, color: 'var(--text-muted)' },
    { id: 1, label: 'HOMING', x: 230, y: 100, color: 'var(--accent-cyan)' },
    { id: 2, label: 'IDLE', x: 360, y: 100, color: 'var(--accent-indigo)' },
    { id: 3, label: 'MOVING', x: 490, y: 100, color: 'var(--accent-emerald)' },
    { id: 4, label: 'FAULT', x: 300, y: 200, color: 'var(--accent-rose)' },
  ],
  edges: [
    { from: 0, to: 1, label: 'Supervisor Active' },
    { from: 1, to: 2, label: 'Done' },
    { from: 2, to: 3, label: 'Target Set' },
    { from: 3, to: 2, label: 'Done' },
    { from: 2, to: 0, label: 'Sup Init/Fault' },
    { from: 3, to: 2, label: 'Sup Idle/Pause' },
  ]
};

/* --- SHARED COMPONENTS --- */

function NodeCircle({ node, isActive }) {
  return (
    <motion.g animate={{ scale: isActive ? 1.05 : 1 }}>
      <circle 
        cx={node.x} cy={node.y} r="25"
        className={`fsm-node-circle ${isActive ? 'active' : ''}`}
        style={{ 
          stroke: isActive ? node.color : 'rgba(255,255,255,0.2)',
          fill: isActive ? 'rgba(255,255,255,0.08)' : 'rgba(0,0,0,0.4)',
          filter: isActive ? `drop-shadow(0 0 8px ${node.color}cc)` : 'none'
        }}
      />
      <text x={node.x} y={node.y + 40} textAnchor="middle" className="fsm-node-text"
            style={{ 
              fill: isActive ? '#fff' : 'rgba(255,255,255,0.4)',
              fontWeight: isActive ? '800' : '500',
              fontSize: '10px'
            }}>
        {node.label}
      </text>
    </motion.g>
  );
}

function EdgeLine({ edge, nodes, isActive }) {
  const from = nodes.find(n => n.id === edge.from);
  const to = nodes.find(n => n.id === edge.to);
  if (!from || !to) return null;

  return (
    <g>
      <defs>
        <marker id="arrowhead" markerWidth="6" markerHeight="4" 
        refX="31" refY="2" orient="auto">
          <polygon points="0 0, 6 2, 0 4" fill="rgba(255,255,255,0.2)" />
        </marker>
        <marker id="active-arrowhead" markerWidth="6" markerHeight="4" 
        refX="31" refY="2" orient="auto">
          <polygon points="0 0, 6 2, 0 4" fill={from.color} />
        </marker>
      </defs>
      <line 
        x1={from.x} y1={from.y} x2={to.x} y2={to.y} 
        className={isActive ? 'fsm-edge-active' : 'fsm-edge-bg'}
        style={isActive ? { stroke: from.color } : {}}
        markerEnd={isActive ? "url(#active-arrowhead)" : "url(#arrowhead)"}
      />
    </g>
  );
}

function SubsystemDiagram({ title, icon: Icon, fsm, currentState }) {
  return (
    <div className="sub-fsm-card glass-card">
      <div className="sub-fsm-header">
        <Icon size={16} />
        <h4>{title}</h4>
      </div>
      <svg viewBox="0 0 700 280" className="fsm-svg-small">
        {fsm.edges.map((e, i) => (
          <EdgeLine key={i} edge={e} nodes={fsm.nodes} isActive={e.from === currentState} />
        ))}
        {fsm.nodes.map(n => (
          <NodeCircle key={n.id} node={n} isActive={n.id === currentState} />
        ))}
      </svg>
    </div>
  );
}

/* --- MAIN COMPONENT --- */

export default function SystemStatusMap({ sysStatus }) {
  const currentSysState = sysStatus?.state ?? 1;
  const currentMobState = sysStatus?.mobility_state ?? 0;
  const currentArmState = sysStatus?.arm_state ?? 0;

  const getSysLabel = (id) => SUPERVISOR_FSM.nodes.find(n => n.id === id)?.label || 'UNKNOWN';
  const getMobLabel = (id) => MOBILITY_FSM.nodes.find(n => n.id === id)?.label || 'UNKNOWN';
  const getArmLabel = (id) => ARM_FSM.nodes.find(n => n.id === id)?.label || 'UNKNOWN';

  return (
    <div className="fsm-complex-view">
      
      {/* SECTION 1: QUICK INDICATORS (RESTORED) */}
      <div className="quick-indicators-row">
        <div className="indicator-chip" data-active="true">
          <Cpu size={14} style={{ color: SUPERVISOR_FSM.nodes[currentSysState]?.color }} />
          <span className="label">SYSTEM:</span>
          <span className="value" style={{ color: SUPERVISOR_FSM.nodes[currentSysState]?.color }}>{getSysLabel(currentSysState)}</span>
        </div>
        <div className="indicator-chip" data-active="true">
          <Zap size={14} style={{ color: MOBILITY_FSM.nodes[currentMobState]?.color }} />
          <span className="label">MOBILITY:</span>
          <span className="value" style={{ color: MOBILITY_FSM.nodes[currentMobState]?.color }}>{getMobLabel(currentMobState)}</span>
        </div>
        <div className="indicator-chip" data-active="true">
          <Activity size={14} style={{ color: ARM_FSM.nodes[currentArmState]?.color }} />
          <span className="label">ARM:</span>
          <span className="value" style={{ color: ARM_FSM.nodes[currentArmState]?.color }}>{getArmLabel(currentArmState)}</span>
        </div>
      </div>

      <div className="fsm-diagrams-grid">
        
        {/* SECTION 2: FORMAL SUPERVISOR DIAGRAM (TOP FULL WIDTH) */}
        <div className="main-fsm-card glass-card">
          <div className="sub-fsm-header">
            <Shield size={18} />
            <h3>Formal Supervisor Logic</h3>
          </div>
          <svg viewBox="0 0 800 320" className="fsm-svg-main">
            {SUPERVISOR_FSM.edges.map((e, i) => (
              <EdgeLine key={i} edge={e} nodes={SUPERVISOR_FSM.nodes} isActive={e.from === currentSysState} />
            ))}
            {SUPERVISOR_FSM.nodes.map(n => (
              <NodeCircle key={n.id} node={n} isActive={n.id === currentSysState} />
            ))}
          </svg>
        </div>

        {/* SECTION 3: SUBSYSTEM SLAVES (SIDE BY SIDE) */}
        <div className="slaves-fsm-row">
          <SubsystemDiagram 
            title="Powertrain (Mobility)" 
            icon={Zap} 
            fsm={MOBILITY_FSM} 
            currentState={currentMobState} 
          />
          <SubsystemDiagram 
            title="Kinematics (Arm)" 
            icon={Activity} 
            fsm={ARM_FSM} 
            currentState={currentArmState} 
          />
        </div>

      </div>
    </div>
  );
}
