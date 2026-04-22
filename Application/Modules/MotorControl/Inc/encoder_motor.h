#ifndef __ENCODER_MOTOR_H_
#define __ENCODER_MOTOR_H_

#include <stdint.h>
#include <stdbool.h>
#include "pid.h"

typedef struct EncoderMotorObject EncoderMotorObjectTypeDef;

/**
 * @brief Encoder Motor Object Structure
 */
struct EncoderMotorObject {
    int64_t counter;        /**< @brief Total count (64-bit to prevent overflow) */
    int32_t ticks_overflow; /**< @brief Timer ARR value for overflow handling */
    int64_t overflow_num;   /**< @brief Count of timer overflows */
    
    float tps;              /**< @brief Ticks Per Second */
    float measured_speed;   /**< @brief Measured velocity in m/s */
    float target_speed;     /**< @brief Target velocity in m/s */
    
    float current_pulse;    /**< @brief Current PWM output (-65535 to 65535) */
    PID_ControllerTypeDef pid_controller; /**< @brief Velocity PID controller */

    /* Physical parameters */
    int32_t ticks_per_circle; /**< @brief Ticks per output shaft revolution */
    float speed_limit;        /**< @brief Max safe speed (m/s) */
    float deadzone;           /**< @brief PWM deadzone threshold */

    /** Hardware Interface **/
    uint8_t motor_id;        /**< @brief Motor identifier (1-4) */
    void (*set_pulse)(EncoderMotorObjectTypeDef *self, int pulse);
};

void encoder_motor_object_init(EncoderMotorObjectTypeDef *self);
void encoder_update(EncoderMotorObjectTypeDef *self, float period, int64_t new_counter);
void encoder_motor_control(EncoderMotorObjectTypeDef *self, float period);
void encoder_motor_pid_control(EncoderMotorObjectTypeDef *self, float period);
void encoder_motor_open_loop_control(EncoderMotorObjectTypeDef *self);
void encoder_motor_set_speed(EncoderMotorObjectTypeDef *self, float speed_ms);
void encoder_motor_brake(EncoderMotorObjectTypeDef *self);
void encoder_motor_apply_pulse(EncoderMotorObjectTypeDef *self, float pulse);
void encoder_motor_refresh_config(EncoderMotorObjectTypeDef *self);

/**
 * @brief High-level hardware initialization for the motor system
 * @param motors Array of 4 motor objects to initialize
 * @return true if initialization succeeded, false otherwise
 */
bool encoder_motor_init_hw_system(EncoderMotorObjectTypeDef *motors[4]);

/**
 * @brief Configure motor with a specific profile and ensure safety
 * @param self Motor object
 * @param id Motor identifier (1-4)
 * @param motor_type Motor type (casted from MotorTypeEnum to avoid circularity)
 * @return true if configuration succeeded, false otherwise
 */
bool encoder_motor_configure(EncoderMotorObjectTypeDef *self, uint8_t id, uint8_t motor_type);

#endif /* __ENCODER_MOTOR_H_ */
