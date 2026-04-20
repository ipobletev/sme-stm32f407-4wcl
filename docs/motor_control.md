# Motor Control Operation

This document describes the implementation and operation of the motor control system for the SME-STM32F407-4WCL robot.

## 1. System Overview

The motor control system is responsible for managing four DC motors with encoders in a closed-loop configuration (PID) or open-loop for testing. It handles everything from raw pulse counting to high-level kinematics for various chassis types (Mecanum, Differential, etc.).

### Key Components

| Component | File | Responsiblity |
| :--- | :--- | :--- |
| **Feedback Task** | `task_sensors.c` | High-frequency acquisition of encoder counts and velocity estimation. |
| **Control Task** | `task_mobility.c` | FSM-based logic for processing movement commands. |
| **Motor Module** | `encoder_motor.c` | Logic for individual motor objects, including PID application. |
| **PID controller** | `pid.c` | Incremental PID algorithm implementation. |
| **Kinematics** | `kinematics_*.c` | Mathematical models for wheel velocity distribution. |

---

## 2. Feedback Loop (Sensor Acquisition)

The feedback loop resides in `task_sensors.c`, running at **100Hz** (default).

### Velocity Estimation
For each motor, the raw encoder count is fetched from the hardware timers. The velocity is estimated using the following process:
1. **Delta Calculation**: Change in pulses since the last update.
2. **Filtering**: A simple low-pass filter is applied to the Ticks-Per-Second (TPS):
   $$TPS_{filt} = TPS_{new} \cdot 0.9 + TPS_{old} \cdot 0.1$$
3. **Normalization**: Conversion from TPS to Rotations-Per-Second (RPS) using the configured `motor_ticks_per_circle`.

The measured RPS is then stored in the `RobotState` for telemetry and control.

---

## 3. Kinematics

When the robot is in the `MOVING` state (`state_moving.c`), target linear and angular velocities are converted into individual wheel speeds.

### Linear/Angular to Wheel Velocity
The system applies the selected kinematic model (e.g., Mecanum) to calculate the required linear velocity ($v_i$ in m/s) for each wheel.

### Linear to Angular Conversion
Wheel linear velocity is converted to target RPS using the wheel diameter:
$$RPS_{target} = \frac{v_i}{\pi \cdot D_{wheel}}$$

> [!IMPORTANT]
> The Right-side motors (Motors 2 and 3) are physically flipped in the 4WCL chassis. The control logic compensates for this by inverting the target RPS sign in `state_moving.c`.

---

## 4. Control Loop (PID Logic)

The core control logic is executed in `encoder_motor_control` at **50Hz**.

### Incremental PID Algorithm
The system uses an **Incremental PID** controller to calculate the change in PWM output ($\Delta u$):
$$\Delta u = K_p (e_k - e_{k-1}) + K_i (e_k \cdot T) + K_d \frac{e_k - 2e_{k-1} + e_{k-2}}{T}$$

Where:
- $e_k$: Current error (Target RPS - Measured RPS).
- $T$: Loop period (0.02s).

**Closed-Loop Mode**: $\text{PWM}_{new} = \text{PWM}_{old} + \Delta u$
**Open-Loop Mode**: Simple linear mapping of Target RPS to Max PWM.

### Safety and Stability
- **Clamping**: PWM output is strictly limited to $\pm \text{AppConfig->motor_pwm_max}$.
- **Deadband**: If the target speed is 0 and the calculated pulse is within the `motor_pulse_deadzone`, the output is forced to 0 to prevent motor whining.

---

## 5. RTOS Integration

The system leverages several tasks to ensure deterministic behavior:

| Task | Frequency | Priority | Role |
| :--- | :--- | :--- | :--- |
| **SensorsTask** | 100Hz | High | Encoder reading & Velocity estimation. |
| **MobilityTask** | 50Hz | High | FSM execution & PID control. |
| **TelemetryTask** | Varied | Medium | Reporting motor status and errors. |

---

## 6. Configuration Parameters

The behavior can be tuned via `app_config_table`. Key parameters include:

| Parameter | Macro Name | Description |
| :--- | :--- | :--- |
| **Wheel Diameter** | `WHEEL_DIAMETER` | Outer diameter of the wheels (m). |
| **Ticks Per Circle** | `MOTOR_TICKS_PER_CIRCLE` | Combined encoder PPR and gearbox ratio. |
| **Max PWM** | `MOTOR_PWM_MAX` | Hardware timer period (usually 10000 or 20000). |
| **PID Gains** | `MOTOR_K_P / I / D` | Coefficients for the incremental controller. |
| **RPS Limit** | `MOTOR_RPS_LIMIT` | Max physical speed of the motor. |

---

## 7. Operational States

The Mobility FSM (`mobility_fsm.c`) ensures the motors are in a safe state:
- **INIT**: Calibrating and resetting hardware.
- **IDLE**: Targets are zeroed, but PID might still be active to maintain position.
- **MOVING**: Active tracking of velocity targets.
- **FAULT/ABORT**: Immediate hard-brake and PWM shutdown.
