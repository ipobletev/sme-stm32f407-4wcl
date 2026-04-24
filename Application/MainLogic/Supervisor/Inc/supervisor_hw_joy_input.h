#ifndef __SUPERVISOR_HW_JOY_INPUT_H
#define __SUPERVISOR_HW_JOY_INPUT_H

/**
 * @brief Process all joystick-related inputs and trigger Supervisor events.
 * This should be called periodically from the Supervisor logic loop.
 */
void Supervisor_HandleJoystickInput(void);

#endif /* __SUPERVISOR_HW_JOY_INPUT_H */
