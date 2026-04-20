#include "encoder_motor.h"
#include "app_config.h"
#include "debug_module.h"
#include "motor_hardware.h"
#include "motor_param.h"
#include "robot_state.h"
#include <stdbool.h>

/**
 * @brief Update motor velocity measurement
 * @param self Motor object
 * @param period Time since last update (s)
 * @param counter Current raw encoder count
 */
void encoder_update(EncoderMotorObjectTypeDef *self, float period, int64_t counter)
{
    /* Calculate delta considering overflow (handled by 64-bit extension in hardware abstraction) */
    int64_t delta_count = counter - self->counter;
    self->counter = counter;

    /* Low-pass filter for speed measurement (90% new, 10% old as per reference) */
    float measure_tps = (float)delta_count / period;
    self->tps = measure_tps * 0.9f + self->tps * 0.1f;
    
    /* Convert to RPS */
    self->rps = self->tps / (float)self->ticks_per_circle;
}

/**
 * @brief Motor velocity control PID loop
 * @param self Motor object
 * @param period Time step (s)
 */
void encoder_motor_control(uint8_t motor_id, EncoderMotorObjectTypeDef *self, float period) 
{
    float pulse = 0;
    
    if (RobotState_PIDIsEnabled()) {
        /* Update incremental PID */
        pid_controller_update(&self->pid_controller, self->rps, period);
        
        /* Calculate new PWM pulse (current + increment) */
        pulse = self->current_pulse + self->pid_controller.output;
    } else {
        /* Open Loop: Simple linear mapping for debugging/testing */
        pulse = (self->target_rps / self->rps_limit) * AppConfig->motor_pwm_max;
    }
    
    /* Clamp output to timer range (-MOTOR_PWM_MAX to MOTOR_PWM_MAX) */
    if (pulse > AppConfig->motor_pwm_max) pulse = AppConfig->motor_pwm_max;
    if (pulse < -AppConfig->motor_pwm_max) pulse = -AppConfig->motor_pwm_max;
    
    /* Apply individual deadband if necessary */
    float output_pulse = pulse;
    
    if (self->pid_controller.set_point == 0) {
        if (output_pulse < self->deadzone && output_pulse > -self->deadzone) {
            output_pulse = 0;
            pulse = 0; /* Reset incremental accumulator to stop the whine */
        }
    }
    self->set_pulse(self, (int)output_pulse);
    self->current_pulse = pulse;
    
    /* Update state for tuning telemetry */
    RobotState_SetPIDDebug(self->motor_id, self->target_rps, self->rps, output_pulse);
}

/**
 * @brief Set target velocity
 */
void encoder_motor_set_speed(EncoderMotorObjectTypeDef *self, float rps)
{
    /* Clamp target speed */
    if (rps > self->rps_limit) rps = self->rps_limit;
    if (rps < -self->rps_limit) rps = -self->rps_limit;
    
    self->target_rps = rps;
    self->pid_controller.set_point = rps;
}

/**
 * @brief Immediate hard stop (Brake)
 */
void encoder_motor_brake(EncoderMotorObjectTypeDef *self)
{
    self->target_rps = 0;
    self->pid_controller.set_point = 0;
    self->current_pulse = 0;
    if (self->set_pulse) {
        self->set_pulse(self, 0);
    }
}

/**
 * @brief Initialize motor object
 */
void encoder_motor_object_init(EncoderMotorObjectTypeDef *self)
{
    self->counter = 0;
    self->overflow_num = 0;
    self->tps = 0; 
    self->rps = 0;
    self->target_rps = 0;
    self->current_pulse = 0;
    self->motor_id = 0; /* Default, should be set during configure */
    self->ticks_overflow = 0; 
    self->ticks_per_circle = AppConfig->motor_ticks_per_circle; /* Default for common motors */
    self->rps_limit = AppConfig->motor_rps_limit;
    self->deadzone = 0;
    pid_controller_init(&self->pid_controller, 0, 0, 0);
}

void encoder_motor_refresh_config(EncoderMotorObjectTypeDef *self) {
    self->ticks_per_circle = AppConfig->motor_ticks_per_circle;
    self->rps_limit = AppConfig->motor_rps_limit;
    
    /* Update per-motor PID gains */
    float kp = DEFAULT_MOTOR_KP, ki = DEFAULT_MOTOR_KI, kd = DEFAULT_MOTOR_KD, deadzone = 0;
    
    switch(self->motor_id) {
        case 1: kp = AppConfig->motor1_kp; ki = AppConfig->motor1_ki; kd = AppConfig->motor1_kd; deadzone = AppConfig->motor1_deadzone; break;
        case 2: kp = AppConfig->motor2_kp; ki = AppConfig->motor2_ki; kd = AppConfig->motor2_kd; deadzone = AppConfig->motor2_deadzone; break;
        case 3: kp = AppConfig->motor3_kp; ki = AppConfig->motor3_ki; kd = AppConfig->motor3_kd; deadzone = AppConfig->motor3_deadzone; break;
        case 4: kp = AppConfig->motor4_kp; ki = AppConfig->motor4_ki; kd = AppConfig->motor4_kd; deadzone = AppConfig->motor4_deadzone; break;
    }
    
    self->deadzone = deadzone;
    pid_controller_init(&self->pid_controller, kp, ki, kd);
}


bool encoder_motor_init_hw_system(EncoderMotorObjectTypeDef *motors[4])
{
    return BSP_Motor_Hardware_Init(motors);
}

bool encoder_motor_configure(EncoderMotorObjectTypeDef *self, uint8_t id, uint8_t motor_type)
{
    /* 1. Initialize logic object */
    encoder_motor_object_init(self);
    self->motor_id = id;
    
    /* 2. Apply hardware profile (PID, TPC, etc) */
    BSP_Motor_Hardware_SetType(self, (MotorTypeEnum)motor_type);
    
    /* 3. Apply individual configuration (PID gains, ticks, etc) */
    encoder_motor_refresh_config(self);
    
    /* 4. Ensure hardware is in a safe state */
    encoder_motor_brake(self);

    /* Note: Currently BSP_Motor_Hardware_SetType doesn't fail, 
       but we return true for API consistency if object is set. */
    return (self->set_pulse != NULL);
}
