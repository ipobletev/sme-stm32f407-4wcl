# RTOS Architecture and Tasks

This document describes the FreeRTOS tasks and timers implemented in the robot firmware, their priorities, and their responsibilities.

## Task and Timer Overview

| Name | Type | Priority | Period/Interval | Description |
| :--- | :--- | :--- | :--- | :--- |
| **UARTListenerTask** | Thread | **High** | Event-based | Listens for commands on UART1 and pushes events to the controller queue. |
| **ManagerTask** | Thread | Normal | Event-based | Executes the **Supervisor** state machine logic (Master FSM). |
| **ControllerTask** | Thread | Normal | Polling (100ms) | Polls physical K1/K2/SW3 buttons and publishes events to the system. |
| **MobilityTask** | Thread | Normal | Event-based | Executes the motor control state machine (Slave FSM). |
| **ArmTask** | Thread | Normal | Event-based | Executes the robotic arm control state machine (Slave FSM). |
| **defaultTask** | Thread | Normal | Yielding | Background task for low-priority system processing. |
| **SystemSensors** | Timer | N/A | 500ms | Samples Battery, MCU Temperature, and Vref. |
| **Heartbeat** | Timer | N/A | 1000ms | Provides system heartbeat (LED blink) and status logging. |

---

## Detailed Task Responsibilities

### [UARTListenerTask](file:///c:/GIT/sme/sme-stm32f407-4wcl/Application/RTOSLogic/Src/task_uart_listener.c)
The system's gateway for external commands (e.g., from ROS or a debug terminal). It handles character reception via DMA and IDLE line detection, parsing structured events like `AUTO`, `STOP`, or `MANUAL`.

### [ManagerTask](file:///c:/GIT/sme/sme-stm32f407-4wcl/Application/MainLogic/Supervisor/Src/app_supervisor.c)
The "brain" of the robot. It manages high-level states (IDLE, MANUAL, AUTO, FAULT, PAUSED) and coordinates the start/stop sequences for the slave subsystems. It listens to the `stateMsgQueue`.

### [ControllerTask](file:///c:/GIT/sme/sme-stm32f407-4wcl/Application/RTOSLogic/Src/task_controller.c)
Centralizes event inputs. It combines physical button presses (K1, K2, SW3) with events received via UART, ensuring a single unified stream of commands reaching the Supervisor.

### [MobilityTask / ArmTask](file:///c:/GIT/sme/sme-stm32f407-4wcl/Application/MainLogic/Slaves/)
These tasks host the slave state machines. They execute subsystem-specific logic independently of the supervisor, allowing parallel execution of movement and arm poses while respecting global pause/stop commands.

### [SystemSensors Timer](file:///c:/GIT/sme/sme-stm32f407-4wcl/Application/RTOSLogic/Src/timer_system_sensors.c)
A periodic callback that ensures hardware health data is always fresh in the `RobotState` structure. It uses ADC DMA to perform non-blocking reads of voltages and temperatures.

---

## Message Queues

The system uses two main queues for thread-safe communication:
1.  **`uartEventQueueHandle`**: Buffers events from the UART interrupt/task to the Controller.
2.  **`stateMsgQueueHandle`**: Transmits unified commands from the Controller to the Supervisor Manager.
