#ifndef __COMMAND_DISPATCHER_H
#define __COMMAND_DISPATCHER_H

#include <stdint.h>
#include <stdbool.h>
#include "supervisor_fsm.h"
#include "mobility_fsm.h"
#include "arm_fsm.h"

/**
 * @brief Unified Command Dispatcher.
 * Centralizes the execution of robot commands from any source (UART, ROS, etc.).
 */

void CmdDispatcher_SetVelocity(float linear_x, float angular_z, uint8_t source);
void CmdDispatcher_TriggerEvent(SystemEvent_t event, uint8_t source);
void CmdDispatcher_SetMobilityConfig(uint8_t mode, uint8_t autonomous, uint8_t source);
void CmdDispatcher_SetArmGoal(float j1, float j2, float j3, uint8_t source);
void CmdDispatcher_UpdateConfig(uint16_t id, float value, uint8_t source);
void CmdDispatcher_SaveConfig(uint8_t source);
void CmdDispatcher_ActuatorTest(uint8_t id, float value, bool is_velocity, uint8_t source);

#endif /* __COMMAND_DISPATCHER_H */
