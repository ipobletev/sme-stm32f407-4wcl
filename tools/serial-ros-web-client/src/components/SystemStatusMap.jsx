import { useState } from 'react';
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
    { from: 0, to: 1, label: 'READY' },
    { from: 1, to: 2, label: 'START' },
    { from: 2, to: 1, label: 'STOP' },
    { from: 3, to: 1, label: 'STOP' },
    { from: 2, to: 3, label: 'MODE_AUTO' },
    { from: 3, to: 2, label: 'MODE_MAN' },
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
    { id: 1, label: 'INIT', x: 60, y: 160, color: 'var(--text-muted)' },
    { id: 2, label: 'IDLE', x: 220, y: 80, color: 'var(--accent-indigo)' },
    { id: 3, label: 'BREAK', x: 380, y: 220, color: 'var(--accent-amber)' },
    { id: 4, label: 'MOVING', x: 540, y: 80, color: 'var(--accent-emerald)' },
    { id: 5, label: 'TESTING', x: 500, y: 220, color: 'var(--accent-cyan)' },
    { id: 6, label: 'FAULT', x: 660, y: 220, color: 'var(--accent-rose)' },
  ],
  edges: [
    { from: 1, to: 2, label: 'Ready' },
    { from: 2, to: 4, label: 'Moving' },
    { from: 2, to: 3, label: 'Break' },
    { from: 2, to: 5, label: 'Test' },
    { from: 4, to: 2, label: 'Idle' },
    { from: 4, to: 3, label: 'Break' },
    { from: 3, to: 4, label: 'Moving' },
    { from: 3, to: 2, label: 'Idle' },
    { from: 5, to: 2, label: 'Idle' },
    { from: 5, to: 4, label: 'Moving' },
    { from: 2, to: 6, label: 'Error' },
    { from: 4, to: 6, label: 'Error' },
    { from: 6, to: 1, label: 'Reset' },
  ]
};

const ARM_FSM = {
  nodes: [
    { id: 1, label: 'INIT', x: 60, y: 160, color: 'var(--text-muted)' },
    { id: 2, label: 'HOMING', x: 180, y: 80, color: 'var(--accent-cyan)' },
    { id: 3, label: 'IDLE', x: 340, y: 160, color: 'var(--accent-indigo)' },
    { id: 4, label: 'MOVING', x: 500, y: 80, color: 'var(--accent-emerald)' },
    { id: 5, label: 'TESTING', x: 500, y: 220, color: 'var(--accent-cyan)' },
    { id: 6, label: 'FAULT', x: 640, y: 160, color: 'var(--accent-rose)' },
  ],
  edges: [
    { from: 1, to: 3, label: 'Idle' },
    { from: 3, to: 2, label: 'Homing' },
    { from: 3, to: 4, label: 'Moving' },
    { from: 3, to: 5, label: 'Test' },
    { from: 2, to: 3, label: 'Done' },
    { from: 2, to: 4, label: 'Moving' },
    { from: 2, to: 5, label: 'Test' },
    { from: 4, to: 3, label: 'Idle' },
    { from: 4, to: 2, label: 'Homing' },
    { from: 4, to: 5, label: 'Test' },
    { from: 5, to: 3, label: 'Idle' },
    { from: 5, to: 4, label: 'Moving' },
    { from: 2, to: 6, label: 'Error' },
    { from: 3, to: 6, label: 'Error' },
    { from: 4, to: 6, label: 'Error' },
    { from: 6, to: 1, label: 'Reset' },
  ]
};

/* --- SHARED COMPONENTS --- */

function NodeCircle({ node, isActive, isRelevant, onHover, onLeave }) {
  return (
    <motion.g 
      animate={{ scale: isActive ? 1.05 : (isRelevant ? 1.02 : 1) }}
      onMouseEnter={() => onHover(node.id)}
      onMouseLeave={() => onLeave()}
      style={{ cursor: 'pointer' }}
    >
      <circle 
        cx={node.x} cy={node.y} r="25"
        className={`fsm-node-circle ${isActive ? 'active' : ''}`}
        style={{ 
          stroke: isActive ? node.color : (isRelevant ? node.color : 'rgba(255,255,255,0.2)'),
          strokeWidth: isRelevant ? '2' : '1.5',
          fill: isActive ? 'rgba(255,255,255,0.08)' : (isRelevant ? 'rgba(255,255,255,0.05)' : 'rgba(0,0,0,0.4)'),
          filter: isActive ? `drop-shadow(0 0 8px ${node.color}cc)` : 'none'
        }}
      />
      <text x={node.x} y={node.y + 40} textAnchor="middle" className="fsm-node-text"
            style={{ 
              fill: (isActive || isRelevant) ? '#fff' : 'rgba(255,255,255,0.4)',
              fontWeight: (isActive || isRelevant) ? '800' : '500',
              fontSize: '10px'
            }}>
        {node.label}
      </text>
    </motion.g>
  );
}

function EdgeLine({ edge, nodes, isActive, isHovered }) {
  const from = nodes.find(n => n.id === edge.from);
  const to = nodes.find(n => n.id === edge.to);
  if (!from || !to) return null;

  return (
    <g style={{ opacity: isHovered || isActive ? 1 : 0.4 }}>
      <defs>
        <marker id={`arrowhead-${from.id}-${to.id}`} markerWidth="6" markerHeight="4" 
        refX="31" refY="2" orient="auto">
          <polygon points="0 0, 6 2, 0 4" fill={(isActive || isHovered) ? from.color : "rgba(255,255,255,0.2)"} />
        </marker>
      </defs>
      <line 
        x1={from.x} y1={from.y} x2={to.x} y2={to.y} 
        className={isActive ? 'fsm-edge-active' : 'fsm-edge-bg'}
        style={(isActive || isHovered) ? { stroke: from.color, strokeDasharray: isActive ? '6' : '4' } : {}}
        markerEnd={`url(#arrowhead-${from.id}-${to.id})`}
      />
    </g>
  );
}

function SubsystemDiagram({ title, icon: Icon, fsm, currentState }) {
  const [hoveredNode, setHoveredNode] = useState(null);

  const potentialNextStates = hoveredNode !== null 
    ? fsm.edges.filter(e => e.from === hoveredNode).map(e => e.to)
    : [];

  return (
    <div className="sub-fsm-card glass-card">
      <div className="sub-fsm-header">
        <Icon size={16} />
        <h4>{title}</h4>
      </div>
      <svg viewBox="0 0 700 320" className="fsm-svg-small">
        {fsm.edges.map((e, i) => (
          <EdgeLine 
            key={i} 
            edge={e} 
            nodes={fsm.nodes} 
            isActive={e.from === currentState} 
            isHovered={e.from === hoveredNode}
          />
        ))}
        {fsm.nodes.map(n => (
          <NodeCircle 
            key={n.id} 
            node={n} 
            isActive={n.id === currentState} 
            isRelevant={n.id === hoveredNode || potentialNextStates.includes(n.id)}
            onHover={(id) => setHoveredNode(id)}
            onLeave={() => setHoveredNode(null)}
          />
        ))}
      </svg>
    </div>
  );
}

/* --- MAIN COMPONENT --- */

export default function SystemStatusMap({ sysStatus }) {
  const [hoveredNode, setHoveredNode] = useState(null);
  
  const currentSysState = sysStatus?.state ?? 1;
  const currentMobState = sysStatus?.mobility_state ?? 0;
  const currentArmState = sysStatus?.arm_state ?? 0;

  const potentialNextSysStates = hoveredNode !== null 
    ? SUPERVISOR_FSM.edges.filter(e => e.from === hoveredNode).map(e => e.to)
    : [];

  const getSysLabel = (id) => SUPERVISOR_FSM.nodes.find(n => n.id === id)?.label || 'UNKNOWN';
  const getMobLabel = (id) => MOBILITY_FSM.nodes.find(n => n.id === id)?.label || 'UNKNOWN';
  const getArmLabel = (id) => ARM_FSM.nodes.find(n => n.id === id)?.label || 'UNKNOWN';

  const getSysColor = (id) => SUPERVISOR_FSM.nodes.find(n => n.id === id)?.color || 'rgba(255,255,255,0.2)';
  const getMobColor = (id) => MOBILITY_FSM.nodes.find(n => n.id === id)?.color || 'rgba(255,255,255,0.2)';
  const getArmColor = (id) => ARM_FSM.nodes.find(n => n.id === id)?.color || 'rgba(255,255,255,0.2)';

  return (
    <div className="fsm-complex-view">
      
      <div className="quick-indicators-row">
        <div className="indicator-chip" data-active="true">
          <Cpu size={14} style={{ color: getSysColor(currentSysState) }} />
          <span className="label">SYSTEM:</span>
          <span className="value" style={{ color: getSysColor(currentSysState) }}>{getSysLabel(currentSysState)}</span>
        </div>
        <div className="indicator-chip" data-active="true">
          <Zap size={14} style={{ color: getMobColor(currentMobState) }} />
          <span className="label">MOBILITY:</span>
          <span className="value" style={{ color: getMobColor(currentMobState) }}>{getMobLabel(currentMobState)}</span>
        </div>
        <div className="indicator-chip" data-active="true">
          <Activity size={14} style={{ color: getArmColor(currentArmState) }} />
          <span className="label">ARM:</span>
          <span className="value" style={{ color: getArmColor(currentArmState) }}>{getArmLabel(currentArmState)}</span>
        </div>
      </div>

      <div className="fsm-diagrams-grid">
        
        <div className="main-fsm-card glass-card">
          <div className="sub-fsm-header">
            <Shield size={18} />
            <h3>Formal Supervisor Logic</h3>
          </div>
          <svg viewBox="0 0 800 320" className="fsm-svg-main">
            {SUPERVISOR_FSM.edges.map((e, i) => (
              <EdgeLine 
                key={i} 
                edge={e} 
                nodes={SUPERVISOR_FSM.nodes} 
                isActive={e.from === currentSysState} 
                isHovered={e.from === hoveredNode}
              />
            ))}
            {SUPERVISOR_FSM.nodes.map(n => (
              <NodeCircle 
                key={n.id} 
                node={n} 
                isActive={n.id === currentSysState} 
                isRelevant={n.id === hoveredNode || potentialNextSysStates.includes(n.id)}
                onHover={(id) => setHoveredNode(id)}
                onLeave={() => setHoveredNode(null)}
              />
            ))}
          </svg>
        </div>

        <div className="slaves-fsm-row">
          <SubsystemDiagram 
            title="Mobility" 
            icon={Zap} 
            fsm={MOBILITY_FSM} 
            currentState={currentMobState} 
          />
          <SubsystemDiagram 
            title="Arm" 
            icon={Activity} 
            fsm={ARM_FSM} 
            currentState={currentArmState} 
          />
        </div>

      </div>
    </div>
  );
}
