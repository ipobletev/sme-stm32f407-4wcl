import { LayoutDashboard, Terminal, Gauge, Settings, HelpCircle, User, LineChart } from 'lucide-react';

export default function PageSidebar({ collapsed, activeTab, onTabChange }) {
  const navItems = [
    { id: 'dashboard', icon: LayoutDashboard, label: 'Dashboard' },
    { id: 'graphs', icon: LineChart, label: 'Real-time Graphs' },
    // { id: 'telemetry', icon: Gauge, label: 'Telemetry' },
    // { id: 'terminal', icon: Terminal, label: 'Serial Log' },
  ];

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

      <div className="sidebar-footer">
        <div className="footer-status">
          <div className="status-dot online"></div>
          {!collapsed && <span className="status-text">System Online</span>}
        </div>
      </div>
    </aside>
  );
}
