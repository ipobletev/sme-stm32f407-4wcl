#include "encoder_motor.h"

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
void encoder_motor_control(EncoderMotorObjectTypeDef *self, float period) 
{
    float pulse = 0;
    
    /* Update incremental PID */
    pid_controller_update(&self->pid_controller, self->rps, period);
    
    /* Calculate new PWM pulse (current + increment) */
    pulse = self->current_pulse + self->pid_controller.output;
    
    /* Clamp output to timer range (-1000 to 1000) */
    if (pulse > 1000.0f) pulse = 1000.0f;
    if (pulse < -1000.0f) pulse = -1000.0f;
    
    /* Apply deadband if necessary (approx +/- 250 as per ref) */
    float output_pulse = pulse;
    if (output_pulse < 250.0f && output_pulse > -250.0f && self->pid_controller.set_point == 0) {
        output_pulse = 0;
    }
    
    self->set_pulse(self, (int)output_pulse);
    self->current_pulse = pulse;
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
    self->ticks_overflow = 0; 
    self->ticks_per_circle = 3960; /* Default for common motors */
    self->rps_limit = 1.0f;
    pid_controller_init(&self->pid_controller, 0, 0, 0);
}
