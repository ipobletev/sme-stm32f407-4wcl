#include "usb_joystick.h"
#include "debug_module.h"
#include "usbh_hid.h"
#include <string.h>

#define LOG_TAG "Joystick"

static USB_Joystick_State_t joystick_state = {0};
static uint8_t joystick_report[64];

void USB_Joystick_Init(void) {
    memset(&joystick_state, 0, sizeof(joystick_state));
}

void USB_Joystick_Process(USBH_HandleTypeDef *phost) {
    HID_HandleTypeDef *HID_Handle = (HID_HandleTypeDef *) phost->pActiveClass->pData;
    
    if (HID_Handle == NULL) return;
    
    // /* Heartbeat every 2 seconds if device is connected but silent */
    // static uint32_t last_joy_heartbeat = 0;
    // if (HAL_GetTick() - last_joy_heartbeat > 2000) {
    //     if (USBH_HID_GetDeviceType(phost) == HID_UNKNOWN) {
    //         LOG_INFO(LOG_TAG, "Heartbeat - Connected: %d, Pipe: %d, Length: %d\r\n", 
    //                joystick_state.connected, HID_Handle->InPipe, HID_Handle->length);
    //     }
    //     last_joy_heartbeat = HAL_GetTick();
    // }

    /* Check if the device is actually a joystick. 
     * In many cases, if it's not a keyboard or mouse, it might be a generic HID joystick.
     */
    if (USBH_HID_GetDeviceType(phost) == HID_UNKNOWN) {
        /* If it's unknown to the standard driver, we can try to parse it as generic joystick */
        if (HID_Handle->length > 0 && HID_Handle->length <= sizeof(joystick_report)) {
            if (USBH_HID_FifoRead(&HID_Handle->fifo, joystick_report, HID_Handle->length) == HID_Handle->length) {
                /* Generic Mapping (Standard for many gamepads/joysticks):
                 * Byte 0: X axis
                 * Byte 1: Y axis
                 * Byte 2: Z axis or Rz
                 * Byte 3: Rz or buttons
                 * ... 
                 * This is a heuristic. In a real scenario, we'd use the HID parser.
                 */
                 
                /* Debug: Print raw report (Full length) */
                char hex_buffer[128];
                int pos = 0;
                pos += sprintf(hex_buffer + pos, "Joy (%d):", HID_Handle->length);
                for (int i = 0; i < HID_Handle->length && i < 32; i++) {
                    pos += sprintf(hex_buffer + pos, " %02X", joystick_report[i]);
                }
                LOG_DEBUG(LOG_TAG, "%s\r\n", hex_buffer);

                /* Heuristic parsing based on docs/usb_joystick_protocol.md:
                 * Byte 6-7:   Joy1 X (Horiz)
                 * Byte 8-9:   Joy1 Y (Vert)
                 * Byte 10-11: Joy2 X (Horiz)
                 * Byte 12-13: Joy2 Y (Vert)
                 */
                if (HID_Handle->length >= 14) {
                    /* Parse 16-bit axes and scale to 8-bit (-128 to 127) */
                    int16_t raw_lx, raw_ly, raw_rx, raw_ry;
                    memcpy(&raw_lx, &joystick_report[6], 2);
                    memcpy(&raw_ly, &joystick_report[8], 2);
                    memcpy(&raw_rx, &joystick_report[10], 2);
                    memcpy(&raw_ry, &joystick_report[12], 2);
                    
                    /* Scale from [-32768, 32767] to [-128, 127] */
                    joystick_state.lx = (int8_t)(raw_lx / 256);
                    joystick_state.ly = (int8_t)(raw_ly / 256);
                    joystick_state.rx = (int8_t)(raw_rx / 256);
                    joystick_state.ry = (int8_t)(raw_ry / 256);
                    
                    /* Parse Triggers */
                    joystick_state.l2 = joystick_report[4];
                    joystick_state.r2 = joystick_report[5];
                    
                    /* Pack buttons: Byte 2 (LSB) and Byte 3 (MSB) */
                    joystick_state.buttons = (uint16_t)joystick_report[2] | ((uint16_t)joystick_report[3] << 8);
                }
                
                if (!joystick_state.connected) {
                    LOG_DEBUG(LOG_TAG, "Joystick: Valid report received, now active.\r\n");
                }
                joystick_state.connected = 1;
            } else {
                /* Only print if we were connected, to avoid spam if FIFO is usually empty */
                if (joystick_state.connected) {
                    // printf("Joystick: FIFO empty or read failed.\r\n");
                }
            }
        } else {
            static uint32_t last_len_err = 0;
            if (HAL_GetTick() - last_len_err > 1000) {
                LOG_ERROR(LOG_TAG, "Joystick DEBUG: Invalid length %d (expected 1-64)\r\n", HID_Handle->length);
                last_len_err = HAL_GetTick();
            }
        }
    } else {
        if (joystick_state.connected) {
            LOG_ERROR(LOG_TAG, "Joystick: Device type changed to standard HID (Mouse/Kbd), disabling joystick mode.\r\n");
        }
        /* Standard Mouse/Keyboard - Not handled as joystick */
        joystick_state.connected = 0;
    }
}

USB_Joystick_State_t* USB_Joystick_GetState(void) {
    return &joystick_state;
}

uint8_t USB_Joystick_IsConnected(void) {
    return joystick_state.connected;
}
