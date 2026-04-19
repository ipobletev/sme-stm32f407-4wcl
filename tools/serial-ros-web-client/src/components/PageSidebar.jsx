import { LayoutDashboard, Wrench, LineChart, Network, AlertTriangle, Table2, Gamepad2 } from 'lucide-react';
import { getActiveErrors } from '../utils/ErrorMapping';

export default function PageSidebar({ collapsed, activeTab, onTabChange, sysStatus }) {
  const navItems = [
    { id: 'dashboard', icon: LayoutDashboard, label: 'Dashboard' },
    { id: 'operator-control', icon: Gamepad2, label: 'Operator Control' },
    { id: 'actuator-tool', icon: Wrench, label: 'Actuator Tool' },
    { id: 'graphs', icon: LineChart, label: 'Real-time Graphs' },
    { id: 'fsm', icon: Network, label: 'Logic Map' },
    { id: 'fsm-log', icon: Table2, label: 'State log' },
  ];

  const activeErrors = getActiveErrors(sysStatus?.errors);

  return (
    <aside className={`page-sidebar ${collapsed ? 'collapsed' : 'expanded'}`}>
      <div className="sidebar-brand">
        <div className="logo-icon">S</div>
      </div>
      
      <nav className="sidebar-nav">
        {navItems.map((item) => (
          <button 
            key={item.id} 
            className={`nav-item ${activeTab === item.id ? 'active' : ''}`}
            title={item.label}
            onClick={() => onTabChange(item.id)}
          >
            <item.icon size={20} />
            <span className="nav-label">{item.label}</span>
          </button>
        ))}
      </nav>

      {/* Real-time Error List (Left Sidebar instance) */}
      {activeErrors.length > 0 && (
        <div className="sidebar-errors">
          {collapsed ? (
            <div 
              className={`error-compact-dot severity-${activeErrors[0].severity}`} 
              title={`${activeErrors.length} Active Errors`}
            />
          ) : (
            <>
              <div className="error-list-title">
                <AlertTriangle size={12} />
                <span>Active Errors</span>
              </div>
              <div className="error-items-container">
                {activeErrors.map((err, idx) => (
                  <div key={idx} className={`sidebar-error-item severity-${err.severity}`}>
                    <AlertTriangle size={14} className="error-icon" />
                    <span className="error-msg">{err.label}</span>
                  </div>
                ))}
              </div>
            </>
          )}
        </div>
      )}

      <div className="sidebar-footer">
        <div className="footer-status">
          <div className="status-dot online"></div>
          {!collapsed && <span className="status-text">System Online</span>}
        </div>
      </div>
    </aside>
  );
}
