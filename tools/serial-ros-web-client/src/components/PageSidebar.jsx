import { LayoutDashboard, Terminal, Gauge, Settings, HelpCircle, User } from 'lucide-react';

export default function PageSidebar({ collapsed }) {
  const navItems = [
    { id: 'dashboard', icon: LayoutDashboard, label: 'Dashboard', active: true },
    { id: 'telemetry', icon: Gauge, label: 'Telemetry', active: false },
    { id: 'terminal', icon: Terminal, label: 'Serial Log', active: false },
    { id: 'settings', icon: Settings, label: 'Settings', active: false },
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
            className={`nav-item ${item.active ? 'active' : ''}`}
            title={item.label}
          >
            <item.icon size={20} />
            <span className="nav-label">{item.label}</span>
          </button>
        ))}
      </nav>

      <div className="sidebar-footer">
        <button className="nav-item" title="Documentation">
          <HelpCircle size={20} />
        </button>
        <button className="nav-item user-profile" title="User Profile">
          <User size={20} />
        </button>
      </div>
    </aside>
  );
}
