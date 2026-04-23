# SerialROS Dashboard

Real-time diagnostic and control dashboard for the SME STM32 Robotics project. Built with React + Vite and optimized for both desktop and mobile devices.

## 🚀 Quick Start

### 1. Prerequisites
Ensure you have [Node.js](https://nodejs.org/) installed in your system.

### 2. Installation
Install dependencies from the root of this directory:
```bash
npm install
```

### 3. Running the Dashboard
Start the development server and the communication relay. It is recommended to use two separate terminal windows:

**Terminal A: Frontend**
```bash
npm run dev
```

**Terminal B: Network Relay (Bridge)**
```bash
npm run relay
```

## 🧩 Dashboard Features

The dashboard provides a suite of advanced tools for robot development and operation:

- **📊 Dashboard Overview**: Real-time telemetry monitoring, IMU orientation visualization, and critical system health alerts.
- **🎮 Operator Console**: Professional teleoperation suite featuring a 2D joystick (with deadzone), D-Pad controls, and real-time velocity limit scaling.
- **📉 PID Optimizer**: High-frequency motor control analysis tool. Features step-response capture, transient behavior charts, and an "Expert Advisor" for automated gain recommendations.
- **⚙️ Device Configuration**: Real-time synchronization of firmware parameters. Adjust PID gains, kinematic dimensions, and motor polarities without recompiling.
- **🗺️ FSM Map (State Logic)**: Live visualization of the hierarchical Finite State Machine. Monitor the Supervisor and Subsystem states in real-time.
- **📂 Real-time Logs**: Integrated system event feed and raw packet logs to simplify protocol debugging.

## 📱 Mobile & Network Access

The dashboard is fully responsive and can be accessed from any device on your local network (LAN).

### Steps to connect from a Phone:

1.  **Find your PC's IP Address**:
    Open a terminal and run `ipconfig`. Look for the "IPv4 Address" (e.g., `192.168.1.X`).
2.  **Access URL**:
    On your mobile browser, navigate to `http://YOUR_PC_IP:5173`.
3.  **Master Connection**:
    *   One tab **on the PC** must be connected physically to the robot via USB-Serial (click the "Connect" button in the header).
    *   Once the PC tab is connected, it will act as a **Master**, retransmitting telemetry to the Relay server.
    *   The phone will automatically pick up the data from the relay and show live updates.

## 🏗️ Architecture

*   **Vite Dev Server (Port 5173)**: Serves the React frontend. Configured with `--host` to be visible on the LAN.
*   **Node.js Relay (Port 3001)**: A WebSocket-based bridge that synchronizes telemetry between all open tabs (local and remote).
*   **Serial Master**: The browser tab that possesses the Serial Port handle (using Web Serial API).
*   **Vite Proxy**: Automatically routes `/ws-robot` requests from the frontend to the local Node.js relay.

## 🛠️ Tech Stack
- **Development Engine**: **Vite 5** — Powers the lightning-fast Hot Module Replacement (HMR) essential for iterative UI development and real-time data binding.
- **Framework**: React 19
- **3D Visualization**: React Three Fiber + Three.js (for IMU orientation)
- **Styling**: Vanilla CSS (Custom premium design system)
- **Icons**: Lucide React
- **Communications**: Web Serial API + WebSockets
