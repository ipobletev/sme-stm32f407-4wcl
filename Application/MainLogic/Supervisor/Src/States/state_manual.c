#include "States/state_handlers.h"
#include "robot_state.h"
#include "debug_module.h"
#include "usb_joystick.h"
#include "app_config.h"
#include "supervisor_fsm.h"

#define JOYSTICK_DEADZONE 10

void State_Manual_OnEnter(void) {
    LOG_INFO(LOG_TAG, "Entering MANUAL State (Operator Control)\r\n");
    RobotState_SetAutonomous(0);
}

void State_Manual_Run(void) {
    /* 1. Process Hardware USB Joystick */
    if (USB_Joystick_IsConnected()) {
        USB_Joystick_State_t *js = USB_Joystick_GetState();

        //Debug
        LOG_DEBUG(LOG_TAG, "Joy: LX=%d, LY=%d, RX=%d, RY=%d, BTN=0x%04X\r\n", 
                  js->lx, js->ly, js->rx, js->ry, js->buttons);
        
        float target_lx = 0;
        float target_az = 0;
        
        /* Mapping Joy 1 Y axis to Linear X (Forward/Backward) */
        if (js->ly > AppConfig->joy_linear_deadzone || js->ly < -AppConfig->joy_linear_deadzone) {
            target_lx = ((float)js->ly / 127.0f) * AppConfig->motor_speed_limit * AppConfig->joy_linear_gain;
        }
        
        /* Mapping Joy 2 X axis to Angular Z (Rotation) */
        if (js->rx > AppConfig->joy_angular_deadzone || js->rx < -AppConfig->joy_angular_deadzone) {
            target_az = ((float)js->rx / 127.0f) * AppConfig->motor_angular_speed_limit * AppConfig->joy_angular_gain;
        }
        
        /* Send commands to RobotState (MobilityTask will consume them) */
        RobotState_SetTargetVelocity(target_lx, target_az);
    }
}


void State_Manual_OnExit(void) {
    LOG_INFO(LOG_TAG, "Exiting MANUAL State\r\n");
    /* Cleanup before returning to active modes */
}
