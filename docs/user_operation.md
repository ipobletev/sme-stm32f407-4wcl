# User Operation Guide

This guide describes how to operate the robot using the physical hardware interface, the USB Gamepad, and the autonomous control systems.

---

## 1. Physical Interface (On-board Controls)

The robot features three primary physical controls for immediate safety and mode management.

| Control | Action | Function |
| :--- | :--- | :--- |
| **Button K1** | Press | **Emergency Stop (E-STOP)**. Instantly halts all movement and enters FAULT state. |
| **Button K2** | Press | **System Reset**. Clears errors and returns the system to IDLE/INIT state. |
| **Switch SW3** | **OFF** | **Absolute Isolation**. Remote commands (ROS/UART3) are ignored. |
| **Switch SW3** | **ON** | **Autonomous Enabled**. Remote commands are permitted. |

---

## 2. Manual Operation (USB Gamepad)

To drive the robot manually, connect a compatible HID Gamepad (PS3/PS4 style) to the USB Type-A port.

### Basic Driving
- **START Button**: Press once while in IDLE to enter **MANUAL** mode.
- **Left Joystick (Vertical)**: Controls **Linear Velocity** (Forward/Backward).
- **Right Joystick (Horizontal)**: Controls **Angular Velocity** (Turning Left/Right).

### Safety & Stopping
- **SELECT Button**: **Soft Stop**. Returns the robot to IDLE mode.
- **MODE Button**: **Emergency Stop**. Forces a FAULT state.
- **RESET Combo**: Hold **L1 + R1 + L2 + R2** simultaneously for **2 seconds** to perform a system reset (only works while in FAULT state).

---

## 3. Autonomous Operation (ROS / Dashboard)

For autonomous or remote operation, the robot must be correctly configured:

1.  **Hardware Permission**: Ensure **SW3 is ON**. If SW3 is OFF, the robot will reject any remote command.
2.  **Connection**: Connect to the **UART3 (Type-C)** port or via the **SerialROS Dashboard**.
3.  **Activation**: Send the `AUTO` command via the dashboard or ROS node.
4.  **Override**: At any time, pressing **K1** on the board or **MODE** on the gamepad will override the autonomous mode and stop the robot.

---

## 4. Visual & Audible Feedback

- **User LED**: A steady blink indicates the system heartbeat is active.
- **Buzzer**: 
    - A short beep confirms valid state transitions.
    - A repetitive pattern indicates a **FAULT** state that requires a Reset.

---

> [!IMPORTANT]
> **Safety First**: Always keep the physical E-Stop (K1) or the Gamepad within reach when the robot is in Autonomous mode or undergoing testing.
