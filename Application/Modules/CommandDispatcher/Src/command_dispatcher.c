#include "command_dispatcher.h"
#include "robot_state.h"
#include "supervisor_fsm.h"
#include "app_config.h"
#include "debug_module.h"

#define LOG_TAG "CMD_DISP"

void CmdDispatcher_SetVelocity(float linear_x, float angular_z, uint8_t source) {
    SystemState_t current_sup = Supervisor_GetCurrentState();
    
    /* Commands only accepted in AUTO or MANUAL */
    if (current_sup == STATE_SUPERVISOR_AUTO || current_sup == STATE_SUPERVISOR_MANUAL) {
        RobotState_SetTargetVelocity(linear_x, angular_z);
        LOG_DEBUG(LOG_TAG, "Vel Cmd: x=%.2f, z=%.2f (Src:%d)\r\n", linear_x, angular_z, source);
    } else {
        LOG_WARNING(LOG_TAG, "Vel Cmd REJECTED: State is %s\r\n", Supervisor_StateToStr(current_sup));
    }
}

void CmdDispatcher_TriggerEvent(SystemEvent_t event, uint8_t source) {
    LOG_INFO(LOG_TAG, "Event Cmd: %d (Src:%d)\r\n", event, source);
    Supervisor_SendEvent(event, source);
}

void CmdDispatcher_SetMobilityConfig(uint8_t mode, uint8_t autonomous, uint8_t source) {
    LOG_INFO(LOG_TAG, "Mode Cmd: Mode=%d, Auto=%d (Src:%d)\r\n", mode, autonomous, source);
    
    RobotState_SetTargetMobilityMode((MobilityMode_t)mode);
    
    uint8_t current_auto = RobotState_IsAutonomous();
    if (autonomous != current_auto) {
        RobotState_SetAutonomous(autonomous);
        if (autonomous) {
            Supervisor_SendEvent(EVENT_SUPERVISOR_MODE_AUTO, source);
        } else {
            Supervisor_SendEvent(EVENT_SUPERVISOR_MODE_MANUAL, source);
        }
    }
}

void CmdDispatcher_SetArmGoal(float j1, float j2, float j3, uint8_t source) {
    SystemState_t current_sup = Supervisor_GetCurrentState();
    if (current_sup == STATE_SUPERVISOR_AUTO || current_sup == STATE_SUPERVISOR_MANUAL) {
        RobotState_SetTargetArmPose(j1, j2, j3);
        LOG_DEBUG(LOG_TAG, "Arm Cmd: %.1f, %.1f, %.1f (Src:%d)\r\n", j1, j2, j3, source);
    } else {
        LOG_WARNING(LOG_TAG, "Arm Cmd REJECTED: State is %s\r\n", Supervisor_StateToStr(current_sup));
    }
}

void CmdDispatcher_UpdateConfig(uint16_t id, float value, uint8_t source) {
    LOG_INFO(LOG_TAG, "Config Cmd: ID=%d, Val=%.4f (Src:%d)\r\n", id, value, source);
    AppConfig_UpdateParam(id, value);
}

void CmdDispatcher_SaveConfig(uint8_t source) {
    LOG_INFO(LOG_TAG, "Save Cmd (Src:%d)\r\n", source);
    AppConfig_Save();
}

void CmdDispatcher_ActuatorTest(uint8_t id, float value, bool is_velocity, uint8_t source) {
    LOG_INFO(LOG_TAG, "Actuator Cmd: ID=%d, Val=%.2f, Type=%s (Src:%d)\r\n", 
             id, value, is_velocity ? "VEL" : "PWM", source);
             
    if (id == 0xFF) {
        for (uint8_t i = 0; i < 4; i++) {
            RobotState_SetMotorTestCommand(i, value, is_velocity ? 1 : 0);
        }
    } else if (id < 4) {
        RobotState_SetMotorTestCommand(id, value, is_velocity ? 1 : 0);
    }
}
