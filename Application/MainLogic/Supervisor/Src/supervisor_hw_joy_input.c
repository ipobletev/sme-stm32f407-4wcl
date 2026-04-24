#include "supervisor_hw_joy_input.h"
#include "supervisor_fsm.h"
#include "usb_joystick.h"
#include "debug_module.h"
#include <stdbool.h>
#include "osal.h"

#define LOG_TAG "SUP_JOY"

/* Internal timer for the 4-button RESET combo */
static uint32_t reset_combo_start_time = 0;


void Supervisor_HandleJoystickInput(void) {
    USB_Joystick_State_t *js = USB_Joystick_GetState();
    SystemState_t current_state = Supervisor_GetCurrentState();

    // /* Diagnostic log for connection status */
    // static uint32_t last_joy_status_tick = 0;
    // if (osal_get_tick() - last_joy_status_tick > 2000) {
    //     LOG_INFO(LOG_TAG, "Joystick Status: %s, Buttons: 0x%04X, State: %s\r\n", 
    //             USB_Joystick_IsConnected() ? "CONNECTED" : "DISCONNECTED",
    //             js->buttons, Supervisor_StateToStr(current_state));
    //     last_joy_status_tick = osal_get_tick();
    // }

    if (!USB_Joystick_IsConnected()) {
        reset_combo_start_time = 0;
        return;
    }

    /* 1. EMERGENCY STOP (MODE Button) - Global */
    if (js->buttons & JOY_BTN_MODE) {
        if (current_state != STATE_SUPERVISOR_FAULT) {
            //LOG_ERROR(LOG_TAG, "Joystick EMERGENCY STOP (MODE) pressed!\r\n");
            Supervisor_SendEvent(EVENT_SUPERVISOR_ERROR, SRC_PHYSICAL);
        }
    }

    /* 2. STOP / IDLE (SELECT Button) - Operational states */
    if (js->buttons & JOY_BTN_SELECT) {
        if (current_state == STATE_SUPERVISOR_MANUAL || 
            current_state == STATE_SUPERVISOR_AUTO || 
            current_state == STATE_SUPERVISOR_PAUSED) {
            //LOG_INFO(LOG_TAG, "Joystick STOP (SELECT) pressed\r\n");
            Supervisor_SendEvent(EVENT_SUPERVISOR_STOP, SRC_PHYSICAL);
        }
    }

    /* 3. START (START Button) - Only in IDLE */
    if (js->buttons & JOY_BTN_START) {
        if (current_state == STATE_SUPERVISOR_IDLE) {
            //LOG_INFO(LOG_TAG, "Joystick START pressed -> Starting system\r\n");
            Supervisor_SendEvent(EVENT_SUPERVISOR_START, SRC_PHYSICAL);
        }
    }

    /* 4. RESET COMBO (L1 + R1 + L2 + R2 held for 2 seconds) */
    /* Consider L2/R2 pressed if > 100 */
    bool reset_pressed = (js->buttons & JOY_BTN_L1) && 
                         (js->buttons & JOY_BTN_R1) && 
                         (js->l2 > 100) && (js->r2 > 100);

    if (reset_pressed) {
        if (reset_combo_start_time == 0) {
            reset_combo_start_time = osal_get_tick();
            //LOG_INFO(LOG_TAG, "Reset combo detected... hold for 2s\r\n");
        } else if (osal_get_tick() - reset_combo_start_time > 2000) {
            if (current_state == STATE_SUPERVISOR_FAULT) {
                //LOG_INFO(LOG_TAG, "Joystick RESET combo triggered!\r\n");
                Supervisor_SendEvent(EVENT_SUPERVISOR_RESET, SRC_PHYSICAL);
            }
            reset_combo_start_time = 0; 
        }
    } else {
        reset_combo_start_time = 0;
    }
}
