# SME-STM32F407-4WCL Control Board

> [!NOTE]
> This project is currently based on the **Hiwonder V1.2 Ros Control** board. The long-term goal is to design a custom hardware version of this control board.

Professional firmware for a 4-wheel drive control board equipped with a robotic arm, built on STM32F407. This project implements a hierarchical control architecture with a central Supervisor and specialized subsystem slaves.

## 🚀 System Architecture

The project is organized around a **Supervisor-Subsystem** pattern, ensuring safe and predictable behavior across all operational modes.

### Core Components
- **[Supervisor](Application/MainLogic/Supervisor)**: High-level Finite State Machine (HFSM) managing global states (Manual, Auto, Fault, Paused).
- **[USB Joystick](Application/MainLogic/Supervisor/Src/supervisor_hw_joy_input.c)**: Native support for generic HID Gamepads with safety combos.
- **[Subsystems](Application/MainLogic/Slaves)**: Modular FSMs for specialized hardware control (Mobility, Robotic Arm).
- **[Robot State](Application/Core/Inc/robot_state.h)**: Centralized thread-safe structure managing Telemetry (Tx) and Commands (Rx).

## 📖 Quick Documentation
- 🕹️ **[User Operation Guide](docs/user_operation.md)**: How to drive the robot and use the physical buttons.
- 🛡️ **[Safety & State Machine](docs/state_machine.md)**: Detailed technical spec of the FSM and authority roles.
- 🔌 **[I/O & Pinout Mapping](docs/io_mapping.md)**: Full hardware connection table.

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
├── state_machine.md  # Detailed FSM Transition & Safety Spec
└── user_operation.md # Practical Guide for Robot Operation
```

## 🖥️ Ecosystem Integration (The Dashboard)

Web application (Vibe coding) that allows you to monitor and control the control board, motors and robotic arm. **[SerialROS Dashboard](tools/serial-ros-web-client/README.md)**. The dashboard is a tool for the development workflow:

- **On-the-fly Calibration**: Adjust critical parameters like wheel diameter, shaft width, and PID gains in real-time without needing to reflash the STM32.
- **Advanced PID Tuning**: Utilize the high-frequency telemetry link to perform step-response tests and visualize motor performance.
- **Universal Access**: Any mobile device on the same network can access the dashboard, providing a portable operator console and diagnostic screen.
- **Visual Verification**: Monitor the internal logic of the Supervisor and Slave FSMs to ensure the robot is operating within safe constraints.

## 🛡️ Safety & Supervision

The system implements a triple safety architecture:
1.  **Top-Down Override**: The Supervisor can instantly disable or pause all subsystems.
2.  **Bottom-Up Guarding**: Subsystems report heartbeats and errors to the Supervisor.
3.  **Absolute Hardware Isolation (SW3)**: A physical switch that can hardware-isolate the robot from remote (UART3/ROS) commands, enforcing a "Manual Only" safety zone.

### Control Authority Roles
The Supervisor enforces an authority hierarchy to prevent command conflicts:
- **Role 0 (Internal System)**: Critical safety monitors and automated fail-safes.
- **Role 1 (Physical HW)**: On-board buttons (K1, K2). General-purpose physical interface, extensible for any direct-interaction logic.
- **Role 2 (Gamepad)**: Manual driving via USB (Disabled in Auto mode).
- **Role 3 (External Client)**: Remote ROS commands (Disabled in Manual mode or if SW3 is OFF).

For a deep dive into the state transitions and safety logic, refer to the **[State Machine Documentation](docs/state_machine.md)**.



## 🛠️ Upcoming Features

The project is currently under active development. The following features are pending implementation or validation:

- [ ] **Kinematics Validation**: Testing of kinematic movement models on real hardware.
- [ ] **Radio Control (SBUS)**: Integration of RC receivers using the SBUS protocol.
- [ ] **Robotic Arm**: Full integration and servo motor control for the robotic arm.
