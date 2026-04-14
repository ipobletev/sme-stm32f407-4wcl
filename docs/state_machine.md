# System State Machine Documentation

This document describes the **Hierarchical Finite State Machine (HFSM)** architecture for the **sme-stm32f407-4wcl** control board. The system uses a **Supervisor-Subsystem** pattern to ensure centralized safety while allowing modular subsystem control.

## 1. Architectural Overview

The system is composed of one **Supervisor Controller** and multiple **Subsystem Slaves**.

- **Supervisor FSM**: Orchestrates global operational modes (Manual, Auto, Fault) and enforces top-level safety transitions.
- **Slave FSMs (Mobility/Arm)**: Manage specific hardware logic and kinematics. They run in dedicated RTOS tasks and react to the Supervisor FSM's state via a **Top-Down Override** mechanism.

---

## 2. Supervisor System FSM

| State | Description | Global Impact |
| :--- | :--- | :--- |
| **STATE_INIT** | Power-on / Hardware Init. | Forces all slaves to **DISABLED**. |
| **STATE_IDLE** | System ready. Safe standby. | Forces all slaves to **STOPPED** / **STDBY**. |
| **STATE_MANUAL** | Operator driving mode. | Slaves follow local control commands. |
| **STATE_AUTO** | ROS-driven autonomous mode. | Slaves follow ROS commands. |
| **STATE_PAUSED**| Temporary halt (Manual/Auto). | Forces slaves to **STOP** (Holding positions). |
| **STATE_FAULT** | Critical error detected. | Immediate **DISABLED** of all power systems. |

---

## 3. Subsystem Slaves

### 3.1 Mobility FSM
Responsible for base movement and powertrain safety.

| State | Description | Reacts to Supervisor |
| :--- | :--- | :--- |
| **MOB_DISABLED** | Motors disabled / Signals cut. | Supervisor in **INIT** / **FAULT**. |
| **MOB_STOPPED** | Velocity = 0. Encoders active. | Supervisor in **IDLE** / **PAUSED**. |
| **MOB_MOVING** | Moving according to target. | Supervisor in **MANUAL** / **AUTO**. |
| **MOB_FAULT** | Local hardware drive error. | Prevents movement. |

### 3.2 Robotic Arm FSM
Responsible for the 3-joint arm positioning.

| State | Description | Reacts to Supervisor |
| :--- | :--- | :--- |
| **ARM_DISABLED** | Servo power cut. | Supervisor in **INIT** / **FAULT**. |
| **ARM_HOMING** | Seeking zero-reference. | Triggered after Supervisor becomes Active. |
| **ARM_IDLE** | Position holding. | Supervisor in **IDLE** / **PAUSED**. |
| **ARM_MOVING** | Executing trajectory. | Supervisor in **MANUAL** / **AUTO**. |

---

## 4. Cross-Layer Logic (Safety Mechanisms)

The system enforces safety via two mechanisms: **Top-Down Override** and **Bottom-Up Supervision (Node Guarding)**.

### 4.1 Top-Down Override
Subsystems do not transition independently of the Supervisor Controller's safety context:
1.  **Fault Propagation**: If Supervisor enters `STATE_FAULT`, all slaves are immediately `DISABLED`.
2.  **Pause Dynamics**: If Supervisor enters `STATE_PAUSED`, Mobility goes to `STOPPED` (zero velocity) and Arm goes to `IDLE` (holding current position).
3.  **Homing Requirement**: The Arm subsystem cannot move (`MOVING`) until it successfully completes the `HOMING` routine, which is triggered when the Supervisor first enters an active mode (`MANUAL`/`AUTO`).

### 4.2 Bottom-Up Supervision (Node Guarding)
To ensure the Supervisor is always aware of slave status without allowing slaves to directly command the Supervisor FSM:
1.  **Central Error Registry**: A shared 64-bit mask (`error_flags`) in the `RobotState` structure holds bit-flags for all hardware errors. Slaves set their corresponding bits if a local hardware failure is detected.
2.  **Heartbeats (Watchdogs)**: Each slave increments a heartbeat counter at 50Hz.
3.  **Cyclic Polling**: The Supervisor Manager task polls the error registry and heartbeat counters every 20ms. If a hardware error is set, or a slave heartbeat stalls for >500ms, the Supervisor transitions to `STATE_FAULT` (which then triggers the Top-Down shutdown).

### 4.3 Command Source & Authority Hierarchy

To prevent conflicting commands from different sources (e.g., a localized technician being overridden by a remote autonomous command), the system implements a **Hierarchical Authority Check**.

Each event sent to the Supervisor includes an `EventSource_t` identifier with an assigned priority level.

| Source | Level | Description | Authority |
| :--- | :---: | :--- | :--- |
| **SRC_INTERNAL_SUPERVISOR** | 4 | Internal safety monitors (watchdogs, temp). | **CRITICAL**: Highest priority. |
| **SRC_PHYSICAL** | 3 | On-board physical buttons (K1, K2). | **SAFETY**: Physical operator control. |
| **SRC_UART1_LOCAL** | 2 | Local Operator Console (UART1/USB). | **DEBUG**: Technician/Field control. |
| **SRC_UART3_ROS** | 1 | Remote Autonomous Control (UART3). | **AUTO**: Standard operation. |
| **SRC_UNKNOWN** | 0 | Unidentified sources. | Lowest priority. |

#### Resume Logic (Safety Guard)
When the system is in `STATE_PAUSED`, it records the authority level of the source that triggered the pause. A `RESUME` event will only be accepted if it comes from a source with **equal or higher authority**.

*Example: If a technician pauses the robot via the **Local Console (Level 2)**, a remote **ROS command (Level 1)** cannot resume operation. Only the Console (Level 2) or a Physical Button (Level 3) can do so.*

---

---

## 5. Transition Matrix (Supervisor)

| Current State | Event | Next State | Notes |
| :--- | :--- | :--- | :--- |
| **STATE_INIT** | `EVENT_START` | **STATE_IDLE** | Slaves remain disabled. |
| **STATE_IDLE** | `EVENT_START` | **STATE_MANUAL** | Slaves begin wakeup (Homing Arm). |
| **STATE_IDLE** | `EVENT_MODE_AUTO` | **STATE_AUTO** | Control authority to ROS. |
| **STATE_MANUAL** | `EVENT_PAUSE` | **STATE_PAUSED** | `STOPPED` (Mob) / `IDLE` (Arm). |
| **STATE_PAUSED** | `EVENT_RESUME` | *Prev Mode* | Resumes previous motion context. |
| **ANY** | `EVENT_ERROR` | **STATE_FAULT** | **CRITICAL**: Full system shutdown. |

---

## 6. Transition Matrix (Slaves)

### 6.1 Mobility Subsystem
| Current State | Condition / Event | Next State | Notes |
| :--- | :--- | :--- | :--- |
| **ANY** | Supervisor in `INIT` / `FAULT` | **MOB_DISABLED** | Safety: Motors powered off. |
| **MOB_MOVING** | Supervisor in `IDLE` / `PAUSED` | **MOB_STOPPED** | Immediate halt (Top-down). |
| **MOB_DISABLED** | Supervisor in `MANUAL` / `AUTO` | **MOB_STOPPED** | Wakeup sequence (Enabling drives). |
| **MOB_STOPPED** | Velocity Targets != 0 | **MOB_MOVING** | Start trajectory execution. |
| **MOB_MOVING** | Velocity Targets == 0 | **MOB_STOPPED** | Target reached / Stopped. |
| **ANY** | Driver/Link Error | **MOB_FAULT** | Local hardware fault detected. |

### 6.2 Robotic Arm Subsystem
| Current State | Condition / Event | Next State | Notes |
| :--- | :--- | :--- | :--- |
| **ANY** | Supervisor in `INIT` / `FAULT` | **ARM_DISABLED** | Safety: Servos unpowered. |
| **ARM_MOVING** | Supervisor in `IDLE` / `PAUSED` | **ARM_IDLE** | Holding current position (Top-down). |
| **ARM_DISABLED** | Supervisor in `MANUAL` / `AUTO` | **ARM_HOMING** | Auto-calibration on activation. |
| **ARM_HOMING** | Homing Finished | **ARM_IDLE** | Ready for trajectory commands. |
| **ARM_IDLE** | Joint Targets Set | **ARM_MOVING** | Start IK trajectory. |
| **ARM_MOVING** | Joint Targets Reached | **ARM_IDLE** | Position reached. |
| **ANY** | Servo Stall / Error | **ARM_FAULT** | Critical joint/servo failure. |

---

## 7. System Interaction Diagram

```mermaid
stateDiagram-v2
    state "SUPERVISOR CONTROLLER" as Supervisor {
        [*] --> STATE_INIT
        STATE_INIT --> STATE_IDLE : START
        STATE_IDLE --> STATE_MANUAL : START / MODE_MANUAL
        STATE_IDLE --> STATE_AUTO : MODE_AUTO
        
        STATE_MANUAL --> STATE_PAUSED : PAUSE
        STATE_AUTO --> STATE_PAUSED : PAUSE
        
        STATE_PAUSED --> STATE_IDLE : STOP
        STATE_PAUSED --> STATE_MANUAL : RESUME (if prev was MAN)
        STATE_PAUSED --> STATE_AUTO : RESUME (if prev was AUTO)

        STATE_IDLE --> STATE_FAULT : ERROR
        STATE_MANUAL --> STATE_FAULT : ERROR
        STATE_AUTO --> STATE_FAULT : ERROR
        STATE_PAUSED --> STATE_FAULT : ERROR
        
        STATE_FAULT --> STATE_INIT : RESET
    }

    state "MOBILITY SLAVE" as Mob {
        [*] --> MOB_DISABLED
        MOB_DISABLED --> MOB_STOPPED : Supervisor Active
        MOB_STOPPED --> MOB_MOVING : Command > 0
        MOB_MOVING --> MOB_STOPPED : Command = 0
        MOB_STOPPED --> MOB_DISABLED : Supervisor Init/Fault
        MOB_MOVING --> MOB_STOPPED : Supervisor Idle/Pause
    }

    state "ARM SLAVE" as Arm {
        [*] --> ARM_DISABLED
        ARM_DISABLED --> ARM_HOMING : Supervisor Active
        ARM_HOMING --> ARM_IDLE : Done
        ARM_IDLE --> ARM_MOVING : Target Set
        ARM_MOVING --> ARM_IDLE : Done
        ARM_IDLE --> ARM_DISABLED : Supervisor Init/Fault
        ARM_MOVING --> ARM_IDLE : Supervisor Idle/Pause
    }
```

---

## 8. Implementation Details

- **Supervisor FSM**: [supervisor_fsm.c](../Application/MainLogic/Supervisor/Src/supervisor_fsm.c)
- **State Handlers**: [state_handlers.h](../Application/MainLogic/Supervisor/Inc/States/state_handlers.h)
- **Robot State (Global)**: [robot_state.h](../Application/Core/Inc/robot_state.h)
- **Mobility Subsystem**: [mobility_fsm.c](../Application/MainLogic/Slaves/MobilityStateMachine/Src/mobility_fsm.c)
- **Arm Subsystem**: [arm_fsm.c](../Application/MainLogic/Slaves/ArmStateMachine/Src/arm_fsm.c)
- **RTOS Tasks**: Managed in `Application/RTOSLogic/Src/`.

> [!IMPORTANT]
> **Safety Priority**: The `K2` physical button and local error detections always trigger `EVENT_ERROR` in the Supervisor FSM, which cascadingly disables all hardware slaves within one RTOS tick (20ms).
