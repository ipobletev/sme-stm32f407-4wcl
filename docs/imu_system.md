# IMU Processing System

This document provides a technical description of the Inertial Measurement Unit (IMU) processing sub-system within the robot firmware. The system focuses on multi-sensor data acquisition, attitude estimation using the Mahony filter, and dynamic bias compensation for long-term yaw stability.

## 1. Sensor Integration and Hardware Abstraction

The firmware employs a Unified IMU Interface (BSP) capable of identifying and communicating with multiple sensor types, primarily the **QMI8658** and **MPU60XX** families, via the I2C2 protocol.

### 1.1 Data Acquisition
Raw data acquisition is performed at a frequency of **100Hz**. The sensor driver performs the following operations:
- **Scaling**: Raw integer values from the ADCs are converted to SI units ($m/s^2$ for acceleration and $rad/s$ for angular velocity) using factory-defined scale factors ($S_{sf}$).
- **Axis Mapping**: Coordinate systems are aligned with the robot's physical chassis frame.
- **Low-Level Compensation**: Static bias offsets are subtracted directly within the driver's read routine to ensure all downstream filters operate on zero-centered data.

## 2. Attitude Estimation (Mahony Filter)

The attitude is estimated using a **Mahony Filter**, a computationally efficient complementary-style filter that combines gyroscope integration with accelerometer-derived gravity vector references.

### 2.1 Orientation Representation
Orientation is maintained Internally as a normalized quaternion ($q_0, q_1, q_2, q_3$) to avoid the gimbal lock singularity inherent in Euler angles.

### 2.2 Gravity Correction
The accelerometer provides a reference for the gravity vector. The filter calculates a rotational error by comparing the estimated gravity vector (from the current quaternion) with the normalized accelerometer reading:
$$e = a_{raw} \times v_{est}$$
This error is integrated and fed back into the gyroscope readings via Proportional-Integral (PI) gains ($K_p$ and $K_i$), correcting for pitch and roll drift.

### 2.3 Yaw Constraints
Since the accelerometer cannot provide a reference for rotation around the gravity vector (Yaw), the Filter relies on pure gyroscope integration for the vertical axis. Long-term stability is managed through the external ZUPT mechanism.

## 3. Drift Mitigation (Zero-Velocity Update - ZUPT)

To address the inherent integration drift in the gyroscope's vertical axis, the system implements a **Dynamic Zero-Velocity Update (ZUPT)** logic.

### 3.1 Stationary Detection
The system monitors wheel encoders to verify the robot's kinematic state. The "Stationary" state is triggered when:
- Linear velocity ($v_x$) is $< 10^{-3}\ m/s$.
- Angular velocity from encoders ($a_z$) is $< 10^{-3}\ rad/s$.
- The state is maintained for a duration $\geq 1.0\ s$.

### 3.2 Dynamic Bias Recalibration
While in the stationary state, any non-zero angular velocity reported by the IMU is identified as **thermal or stochastic bias**. The system calculates the average residual error and applies a corrective feedback to the driver's internal bias structure using an alpha-filter gain:
$$Bias_{new} = Bias_{old} + (Error_{avg} \cdot \alpha)$$
Additionally, angular velocity outputs are clamped to zero during the stationary period to prevent visual jitter in navigation frameworks.

## 4. System Architecture

The IMU data flow is managed by the `SensorsTask` in the FreeRTOS environment:
1. **Acquisition**: `BSP_IMU_ReadRaw` retrieves de-biased SI values.
2. **Estimation**: `BSP_IMU_ReadOrientation` executes the Mahony update and Euler conversion.
3. **Kinematics**: Encoder values are processed to determine the stationary state.
4. **Correction**: ZUPT logic updates the bias parameters if applicable.
5. **Telemetry**: Final state variables (Roll, Pitch, Yaw, Accel, Gyro) are synchronized with the `RobotState` global structure within a critical section.
