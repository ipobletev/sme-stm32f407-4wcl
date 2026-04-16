#ifndef __MOTOR_HARDWARE_H_
#define __MOTOR_HARDWARE_H_

#include "encoder_motor.h"
#include "tim.h"

/**
 * @brief Initialize all motor timers and encoder peripherals
 */
void Motor_Hardware_Init(EncoderMotorObjectTypeDef *motors[4]);

/**
 * @brief Read raw encoder counts with overflow handling
 * @param motor_idx 0-3
 * @return 64-bit integrated count
 */
int64_t Motor_Hardware_GetEncoderCount(uint8_t motor_idx);

#endif /* __MOTOR_HARDWARE_H_ */
