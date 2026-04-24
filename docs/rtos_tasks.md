# RTOS Architecture and Tasks

This document describes the FreeRTOS tasks and timers implemented in the robot firmware, their priorities, and their responsibilities.

## Task and Timer Overview

| Name | Type | Priority | Period/Interval | Description |
| :--- | :--- | :--- | :--- | :--- |
| **UARTListenerTask** | Thread | **High** | Event-based | Listens for commands on UART1 and pushes events to the controller queue. |
| **ManagerTask** | Thread | Normal | Event-based | Executes the **Supervisor** state machine logic (Master FSM). |
| **HWInputTask** | Thread | Normal | Polling (100ms) | Polls physical K1/K2/SW3 for safety (E-Stop, Reset, Permissivity). |
| **MobilityTask** | Thread | Normal | Event-based | Executes the motor control state machine (Slave FSM). |
| **ArmTask** | Thread | Normal | Event-based | Executes the robotic arm control state machine (Slave FSM). |
| **SerialRosTask**| Thread | **High** | Event-based | Handles binary protocol communication for ROS. |
| **TelemetryTask**| Thread | Normal | **10ms** | Multi-rate telemetry publisher (IMU/Odom/Status). |
| **IMUTask**      | Thread | **High** | **10ms** | High-precision IMU sampling and orientation. |
| **defaultTask** | Thread | Normal | Yielding | Background task for low-priority system processing. |
| **Heartbeat** | Timer | N/A | 1000ms | Provides system heartbeat (LED blink) and status logging. |

---

## Detailed Task Responsibilities

### [UARTListenerTask](Application/RTOSLogic/Src/task_uart_listener.c)
The system's gateway for external commands (e.g., from ROS or a debug terminal). It handles character reception via DMA and IDLE line detection, parsing structured events like `AUTO`, `STOP`, or `MANUAL`.

### [ManagerTask](Application/MainLogic/Supervisor/Src/supervisor_fsm.c)
The "brain" of the robot. It manages high-level states (IDLE, MANUAL, AUTO, FAULT, PAUSED) and coordinates subsystems. It listens to the **centralized `stateMsgQueueHandle`**, where all validated events arrive.

### [HWInputTask](Application/RTOSLogic/Src/task_hw_input.c)
The hardware safety layer. It handles critical physical inputs (K1 E-Stop, K2 Reset) and manages the **Autonomous Permissivity Switch (SW3)**, which can physically isolate the robot from external UART commands.

### [MobilityTask / ArmTask](Application/MainLogic/Slaves/)
These tasks host the slave state machines. They execute subsystem-specific logic independently of the supervisor, allowing parallel execution of movement and arm poses while respecting global pause/stop commands.

### [SerialRosTask](Application/RTOSLogic/Src/task_serial_ros.c)
Decouples the timing-critical UART communication from the rest of the application. It manages the `rosTxQueue` and `rosRxQueue`, parsing incoming binary frames and dispatching them to the internal system state.

### [TelemetryTask](Application/RTOSLogic/Src/task_telemetry.c)
A consolidated reporting hub. Instead of each module sending its own UART data, this task snapshots the global `RobotState` at a fixed rate (10ms) and enqueues messages at different intervals:
- **IMU**: 100Hz (Every cycle)
- **Odometry**: 50Hz (Every 2 cycles)
- **System Status**: 2Hz (Every 50 cycles)

### [IMUTask](Application/RTOSLogic/Src/task_imu.c)
Responsible for high-frequency sampling of the on-board MPU6050/ICM20948. It performs sensor fusion (if enabled) and updates the global telemetry structure.

---

## Message Queues

The system uses these main queues for thread-safe communication:
1.  **`stateMsgQueueHandle`**: Transmits unified commands from all sources (Hardware, Gamepad, UART) to the Supervisor Manager.
3.  **`rosTxQueueHandle`**: Buffers binary packets to be transmitted to ROS.
4.  **`rosRxQueueHandle`**: Buffers received binary packets for processing.
