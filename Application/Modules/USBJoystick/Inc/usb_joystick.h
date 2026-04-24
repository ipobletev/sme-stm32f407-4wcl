#ifndef __USB_JOYSTICK_H
#define __USB_JOYSTICK_H

#include <stdint.h>
#include "usbh_core.h"

/**
 * @brief Joystick state structure
 */
typedef struct {
    int8_t lx;          /* Joy1 X - Left/Right */
    int8_t ly;          /* Joy1 Y - Up/Down */
    int8_t rx;          /* Joy2 X - Left/Right */
    int8_t ry;          /* Joy2 Y - Up/Down */
    uint8_t l2;         /* L2 Trigger (0-255) */
    uint8_t r2;         /* R2 Trigger (0-255) */
    uint16_t buttons;   /* 16 buttons bitmask */
    uint8_t connected;  /* 1 if joystick is connected */
} USB_Joystick_State_t;

/* Joystick Button Masks (Based on Byte 2 and 3 packing) */
#define JOY_BTN_L1     0x0100  /* Byte 3, Bit 0 */
#define JOY_BTN_R1     0x0200  /* Byte 3, Bit 1 */
#define JOY_BTN_MODE   0x0400  /* Byte 3, Bit 2 */
#define JOY_BTN_START  0x0010  /* Byte 2, Bit 4 */
#define JOY_BTN_SELECT 0x0020  /* Byte 2, Bit 5 */

/**
 * @brief Initialize USB Joystick module
 */
void USB_Joystick_Init(void);

/**
 * @brief Process joystick data from HID report
 * @param phost USB Host handle
 */
void USB_Joystick_Process(USBH_HandleTypeDef *phost);

/**
 * @brief Get current joystick state
 * @return Pointer to joystick state
 */
USB_Joystick_State_t* USB_Joystick_GetState(void);

/**
 * @brief Check if joystick is connected
 * @return 1 if connected, 0 otherwise
 */
uint8_t USB_Joystick_IsConnected(void);

#endif /* __USB_JOYSTICK_H */
