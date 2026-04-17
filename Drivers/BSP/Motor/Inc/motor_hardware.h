#ifndef __MOTOR_HARDWARE_H_
#define __MOTOR_HARDWARE_H_

#include "encoder_motor.h"
#include "motor_param.h"
#include "tim.h"

/**
 * @brief Initialize all motor timers and encoder peripherals
 */
void Motor_Hardware_Init(EncoderMotorObjectTypeDef *motors[4]);

/**
 * @brief Set specific motor parameters
 */
void Motor_Hardware_SetParam(EncoderMotorObjectTypeDef *motor, int32_t tpc, float rps_limit, float kp, float ki, float kd);

/**
 * @brief Set motor parameters based on pre-defined type
 */
void Motor_Hardware_SetType(EncoderMotorObjectTypeDef *motor, MotorTypeEnum type);

/**
 * @brief Read raw encoder counts with overflow handling
 */
int64_t Motor_Hardware_GetEncoderCount(uint8_t motor_idx);

#endif /* __MOTOR_HARDWARE_H_ */
