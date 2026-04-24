# I/O Mapping and Hardware Pinout

This document details the physical interaction components and the full hardware pinout for the **Hiwonder Board V1.2** based on the STM32F407VET6 MCU.

## 1. User Interaction (Buttons & LEDs)

The following components are directly accessible to the user for control and status monitoring.

| Component | Pin | Action Type | Triggered Event | Description |
| :--- | :--- | :--- | :--- | :--- |
| **K1 Button** | PE1 | Simple Press | `EVENT_ERROR` | **Emergency Stop (E-STOP)**. Immediately locks the system. |
| **K2 Button** | PE0 | Simple Press | `EVENT_RESET` | **System Reset**. Clears faults and returns to IDLE/INIT. |
| **SW3 Switch** | PD3 | ON/OFF | N/A | **Autonomous Permissivity**. If OFF, UART3 commands are ignored. |
| **User LED** | PE10 | Status Output | N/A | System Heartbeat. |
| **Buzzer** | PA8 | Audible Output | N/A | Warning signals and state transition feedback. |

---

## 2. Full Hardware Pinout

Below is the complete mapping of the STM32F407VET6 pins to the Hiwonder Board v1.2 headers and peripherals.

| Group | uC Pin | Header/Component | Code Name | Description | Integrated Component |
| :--- | :---: | :--- | :--- | :--- | :--- |
| **LED** | PE10 | LED1 | `USER_LED` | User led | - |
| **UART3** | PD9 | Micro/Type-C | `USART3_RX` | UART 3 (ROS Communication) | U3 (CH9102F) |
| **UART3** | PD8 | Micro/Type-C | `USART3_TX` | UART 3 (ROS Communication) | U3 (CH9102F) |
| **UART1** | PA10 | Micro/Type-C | `USART1_RX` | UART 1 (Debug/Console) | U13 (CH9102F) |
| **UART1** | PA9 | Micro/Type-C | `USART1_TX` | UART 1 (Debug/Console) | U13 (CH9102F) |
| **BUZZER** | PA8 | BUZ1 | `BUZZER` | System Buzzer | - |
| **SWITCH** | PD3 | SW3 | `USER_SW3` | User switch (Toggle) | - |
| **BUTTON** | PE1 | K1 | `USER_K1_BUTTON` | User button K1 | - |
| **BUTTON** | PE0 | K2 | `USER_K2_BUTTON` | User button K2 | - |
| **SERVO (UART)** | PC6 | P6/P7 | `SERVO_UART6_TX` | Serial Servo TX (5V) | SN74LVC2G125 |
| **SERVO (UART)** | PC7 | P6/P7 | `SERVO_UART6_RX` | Serial Servo RX (5V) | SN74LVC2G125 |
| **SERVO (CTRL)** | PE8 | - | `SERVO_RX_EN` | Control UART RX from Servo | SN74LVC2G125 |
| **SERVO (CTRL)** | PE7 | - | `SERVO_TX_EN` | Control UART TX to Servo | SN74LVC2G125 |
| **MOTORS** | PE13 | Header M1 | `MOTOR_1_BI` | Motor 1 Backwards (TIM1_CH3) | YX-4055AM |
| **MOTORS** | PE14 | Header M1 | `MOTOR_1_FI` | Motor 1 Forward (TIM1_CH4) | YX-4055AM |
| **MOTORS** | PE9 | Header M2 | `MOTOR_2_BI` | Motor 2 Backwards (TIM1_CH1) | YX-4055AM |
| **MOTORS** | PE11 | Header M2 | `MOTOR_2_FI` | Motor 2 Forward (TIM1_CH2) | YX-4055AM |
| **MOTORS** | PE6 | Header M3 | `MOTOR_3_BI` | Motor 3 Backwards (TIM9_CH2) | YX-4055AM |
| **MOTORS** | PE5 | Header M3 | `MOTOR_3_FI` | Motor 3 Forward (TIM9_CH1) | YX-4055AM |
| **MOTORS** | PB8 | Header M4 | `MOTOR_4_BI` | Motor 4 Backwards (TIM10_CH1)| YX-4055AM |
| **MOTORS** | PB9 | Header M4 | `MOTOR_4_FI` | Motor 4 Forward (TIM11_CH1) | YX-4055AM |
| **ENCODERS** | PA0 | Header M1 | `ENC1_A` | Encoder Motor 1 Phase A (TIM5_CH1) | - |
| **ENCODERS** | PA1 | Header M1 | `ENC1_B` | Encoder Motor 1 Phase B (TIM5_CH2) | - |
| **ENCODERS** | PA15 | Header M2 | `ENC2_A` | Encoder Motor 2 Phase A (TIM2_CH1) | - |
| **ENCODERS** | PB3 | Header M2 | `ENC2_B` | Encoder Motor 2 Phase B (TIM2_CH2) | - |
| **ENCODERS** | PB6 | Header M3 | `ENC3_A` | Encoder Motor 3 Phase A (TIM4_CH1) | - |
| **ENCODERS** | PB7 | Header M3 | `ENC3_B` | Encoder Motor 3 Phase B (TIM4_CH2) | - |
| **ENCODERS** | PB4 | Header M4 | `ENC4_A` | Encoder Motor 4 Phase A (TIM3_CH1) | - |
| **ENCODERS** | PB5 | Header M4 | `ENC4_B` | Encoder Motor 4 Phase B (TIM3_CH2) | - |
| **BATTERY** | PB0 | - | `V_BATT_SENSE` | ADC Battery Voltage Sensing | RT8289GSP |
| **IMU** | PB11 | - | `IMU_SDA` | I2C Data Line | MPU-6050 |
| **IMU** | PB10 | - | `IMU_SCL` | I2C Clock Line | MPU-6050 |
| **IMU** | PB12 | - | `IMU_INT` | IMU Interrupt Signal | MPU-6050 |
| **SBUS (RC)** | PD2 | J6 | `SBUS_RXD` | S.BUS Input for RC | - |
| **PWM SERVOS** | PA11 | J1 | `SERVO_PWM_1` | Output PWM Servo 1 | - |
| **PWM SERVOS** | PA12 | J2 | `SERVO_PWM_2` | Output PWM Servo 2 | - |
| **PWM SERVOS** | PC8 | J4 | `SERVO_PWM_3` | Output PWM Servo 3 | - |
| **PWM SERVOS** | PC9 | J5 | `SERVO_PWM_4` | Output PWM Servo 4 | - |
| **USB HOST** | PB15 | Type A | `USB_H_DP` | USB Data Plus (D+) | - |
| **USB HOST** | PB14 | Type A | `USB_H_DM` | USB Data Minus (D-) | - |

---

## 3. Implementation Details

- **Control Logic**: Handled by `StartHWInputTask` in [task_hw_input.c](../Application/RTOSLogic/Src/task_hw_input.c).
- **Sampling Rate**: Inputs are polled every 100ms for debouncing.
- **Safety Priority**: Button **K1** (E-STOP) and **SW3** (Permissivity) are the primary safety layers.
- **Hardware Config**: Pins are configured in the BSP layer using internal **Pull-Up** resistors (Active-LOW).

> [!NOTE]
> For digital inputs (Buttons/Switches), the system uses a 100ms debouncing logic in software to ensure stable state transitions.
