# SME-STM32F407-4WCL Control Board

> [!NOTE]
> This project is currently based on the **Hiwonder V1.2 Ros Control** board. The long-term goal is to design a custom hardware version of this control board.

Professional firmware for a 4-wheel drive control board equipped with a robotic arm, built on STM32F407. This project implements a hierarchical control architecture with a central Supervisor and specialized subsystem slaves.

## 🚀 System Architecture

The project is organized around a **Supervisor-Subsystem** pattern, ensuring safe and predictable behavior across all operational modes.

### Core Components
- **[Supervisor](Application/MainLogic/Supervisor)**: High-level Finite State Machine (HFSM) managing global states (Manual, Auto, Fault, Paused).
- **[Subsystems](Application/MainLogic/Slaves)**: Modular FSMs for specialized hardware control (Mobility, Robotic Arm).
- **[Robot State](Application/Core/Inc/robot_state.h)**: Centralized thread-safe structure managing Telemetry (Tx) and ROS Commands (Rx).

## 📂 Project Structure

```text
Application/
├── Core/             # Global handlers and Robot State
├── MainLogic/
│   ├── Supervisor/   # Main Supervisor FSM & State Handlers
│   └── Slaves/       # Mobility and Arm Subsystem Logic
├── RTOSLogic/        # FreeRTOS tasks and timers
└── Modules/          # Hardware abstraction and utility modules
Docs/
└── state_machine.md  # Detailed FSM Transition & Safety Spec
```

## 🖥️ Ecosystem Integration (The Dashboard)

Web application (Vibe coding) that allows you to monitor and control the control board, motors and robotic arm. **[SerialROS Dashboard](tools/serial-ros-web-client/README.md)**. The dashboard is a tool for the development workflow:

- **On-the-fly Calibration**: Adjust critical parameters like wheel diameter, shaft width, and PID gains in real-time without needing to reflash the STM32.
- **Advanced PID Tuning**: Utilize the high-frequency telemetry link to perform step-response tests and visualize motor performance.
- **Universal Access**: Any mobile device on the same network can access the dashboard, providing a portable operator console and diagnostic screen.
- **Visual Verification**: Monitor the internal logic of the Supervisor and Slave FSMs to ensure the robot is operating within safe constraints.

## 🛡️ Safety & Supervision

The system implements a dual safety mechanism:
1.  **Top-Down Override**: The Supervisor can instantly disable or pause all subsystems.
2.  **Bottom-Up Guarding**: Subsystems report heartbeats and errors to the Supervisor, which triggers a global fault if a failure is detected.

For a deep dive into the state transitions and safety logic, refer to the **[State Machine Documentation](docs/state_machine.md)**.

---
*Developed for SME - Advanced Agentic Coding Project.*
