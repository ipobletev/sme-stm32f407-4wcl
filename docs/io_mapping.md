# I/O Mapping and User Interface

This document details the functionality of the physical interaction components (buttons and switches) implemented in the robot's firmware.

## User Function Table

| Component | Pin (STM32) | Action Type | Triggered Event | Description |
| :--- | :--- | :--- | :--- | :--- |
| **K1 Button** | PE1 | Simple Press | `EVENT_START` / `EVENT_RESUME` | Starts **MANUAL** mode or resumes the system if it is paused. |
| **K2 Button** | PE0 | Short Press (<1s) | `EVENT_PAUSE` | Pauses the robot's current execution. |
| **K2 Button** | PE0 | Long Press (>1s) | `EVENT_STOP` | **Emergency Stop (E-STOP)**. Stops all movement and locks the system. |
| **SW3 Switch** | PD3 | Press (Toggle) | `EVENT_ERROR` | **Error Simulation**: Forces the system into `FAULT` state (for testing). |
| **SW3 Switch** | PD3 | Press (Toggle) | `EVENT_RESET` | **Error Reset**: Exits from `FAULT` state and returns to `IDLE`. |

## Implementation Details

### Control Logic
The management of these components is centralized in the `StartControllerTask` located in [task_controller.c](../Application/RTOSLogic/Src/task_controller.c).

*   **Software Debounce:** The task processes inputs every 100ms, providing natural filtering to prevent false triggers.
*   **Safety Priority:** The **K2** button has safety priority. a long press will always send an absolute stop event, ignoring the robot's current state.
*   **Debugging:** The **SW3** switch works as a logical latch; the first press triggers an error, and the next one clears it.

## Hardware Reference

The pins are defined in the BSP layer and configured as inputs with internal **Pull-Up** resistors (active-low / GND activation).

- **BSP Source:** [bsp_button.c](../Drivers/BSP/Button/Src/bsp_button.c)
- **Definitions:** [main.h](../Core/Inc/main.h)
