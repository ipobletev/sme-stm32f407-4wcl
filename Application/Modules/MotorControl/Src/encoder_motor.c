#include "encoder_motor.h"
#include "app_config.h"
#include "debug_module.h"
#include "motor_hardware.h"
#include "motor_param.h"
#include "robot_state.h"
#include <stdbool.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846f
#endif

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
    // LOG_INFO("DEBUG", "Motor %d TPS: %d\r\n", self->motor_id, (int)self->tps);
    /* Convert to m/s: speed = (TPS / TPC) * PI * Wheel_Diameter */
    float rps = self->tps / (float)self->ticks_per_circle;
    self->measured_speed = rps * (M_PI * AppConfig->wheel_diameter);
}

/**
 * @brief Public helper to finalize PWM output, handle deadzone and update telemetry.
 */
void encoder_motor_apply_pulse(EncoderMotorObjectTypeDef *self, float pulse)
{
    /* Clamp output to timer range (-MOTOR_PWM_MAX to MOTOR_PWM_MAX) */
    if (pulse > AppConfig->motor_pwm_max) pulse = AppConfig->motor_pwm_max;
    if (pulse < -AppConfig->motor_pwm_max) pulse = -AppConfig->motor_pwm_max;
    
    float output_pulse = 0.0f;
    float abs_pulse = (pulse < 0) ? -pulse : pulse;
    float deadzone = self->deadzone;
    float max_pwm = AppConfig->motor_pwm_max;
    
    /* 
     * Deadzone Compensation:
     * We map the requested effort [0, max_pwm] to the physical range [deadzone, max_pwm].
     * This ensures that even the smallest joystick movement (which produces a small pulse)
     * will immediately jump to the minimum PWM required to overcome static friction.
     */
    if (abs_pulse > 0.001f) {
        // Linear mapping: deadzone + (pulse_ratio * remaining_range)
        float compensated = deadzone + (abs_pulse / max_pwm) * (max_pwm - deadzone);
        output_pulse = (pulse > 0) ? compensated : -compensated;
    } else {
        output_pulse = 0.0f;
        
        /* Only reset incremental accumulator if the target is actually zero 
           to stop the 'whining' noise in stationary state. 
           If target is NON-ZERO (but pulse is 0), let the PID/accumulator build up. */
        if (self->target_speed == 0.0f) {
            pulse = 0.0f;
        }
    }
    
    self->set_pulse(self, (int)output_pulse);
    self->current_pulse = pulse;
    
    RobotState_SetMeasuredMotorDebug(self->motor_id, self->target_speed, self->measured_speed, output_pulse);
}

/**
 * @brief Motor velocity control using PID loop
 */
void encoder_motor_pid_control(EncoderMotorObjectTypeDef *self, float period)
{
    /* 
     * Active Braking logic:
     * If target is 0, we don't call brake() immediately. Instead, we let the PID 
     * run to provide counter-torque (active braking) as long as the motor is still moving.
     * We only engage the electronic brake once the speed is below a small threshold.
     */
    if (self->target_speed == 0.0f) {
        if (fabs(self->measured_speed) < 0.02f) {
            encoder_motor_brake(self);
            return;
        }
    }


    /* Update incremental PID */
    pid_controller_update(&self->pid_controller, self->measured_speed, period);
    
    /* 
     * Calculate new PWM pulse (current + increment).
     * current_pulse acts as the integrator/accumulator of the incremental PID.
     * It also carries the Feed-Forward baseline injected in encoder_motor_set_speed.
     */
    float pulse = self->current_pulse + self->pid_controller.output;
    
    encoder_motor_apply_pulse(self, pulse);
}


/**
 * @brief Motor control using Open Loop (Linear RPS-to-PWM mapping)
 */
void encoder_motor_open_loop_control(EncoderMotorObjectTypeDef *self)
{
    /* Simple linear mapping for debugging/testing */
    float pulse = (self->target_speed / self->speed_limit) * AppConfig->motor_pwm_max;
    
    encoder_motor_apply_pulse(self, pulse);
}

/**
 * @brief Motor velocity control dispatcher
 * @param self Motor object
 * @param period Time step (s)
 */
void encoder_motor_control(EncoderMotorObjectTypeDef *self, float period) 
{
    if (RobotState_PIDIsEnabled()) {
        encoder_motor_pid_control(self, period);
    } else {
        encoder_motor_open_loop_control(self);
    }
}

/**
 * @brief Set target velocity
 */
void encoder_motor_set_speed(EncoderMotorObjectTypeDef *self, float speed_ms)
{
    float old_target = self->target_speed;

    /* Clamp target speed */
    if (speed_ms > self->speed_limit) speed_ms = self->speed_limit;
    if (speed_ms < -self->speed_limit) speed_ms = -self->speed_limit;
    
    /* If starting from stopped state, clear PID history to ensure clean jump */
    if (self->target_speed == 0.0f && speed_ms != 0.0f) {
        pid_controller_reset(&self->pid_controller);
    }

    /* 
     * Feed-Forward Injection:
     * We give the PID accumulator a 'head start' based on the theoretical PWM 
     * needed for the new target speed. This significantly improves response time
     * and prevents the PWM from dropping too low when Kp is the only active term.
     */
    if (RobotState_PIDIsEnabled() && speed_ms != old_target) {
        float ff_gain = AppConfig->motor_pwm_max / self->speed_limit;
        // We use a 90% FF factor to allow the PID to always have some control room
        float delta_ff = (speed_ms - old_target) * ff_gain * 0.90f;
        self->current_pulse += delta_ff;
        
        /* Clamp current_pulse to prevent initial jump from exceeding PWM limits before PID check */
        if (self->current_pulse > AppConfig->motor_pwm_max) self->current_pulse = AppConfig->motor_pwm_max;
        if (self->current_pulse < -AppConfig->motor_pwm_max) self->current_pulse = -AppConfig->motor_pwm_max;
    }
    
    self->target_speed = speed_ms;
    self->pid_controller.set_point = speed_ms;
}


/**
 * @brief Immediate hard stop (Brake)
 */
void encoder_motor_brake(EncoderMotorObjectTypeDef *self)
{
    self->target_speed = 0;
    self->pid_controller.set_point = 0;
    self->current_pulse = 0;
    pid_controller_reset(&self->pid_controller);
    
    if (self->set_pulse) {
        self->set_pulse(self, 0);
    }
    RobotState_SetMeasuredMotorDebug(self->motor_id, self->target_speed, self->measured_speed, 0.0f);
}

/**
 * @brief Initialize motor object
 */
void encoder_motor_object_init(EncoderMotorObjectTypeDef *self)
{
    self->counter = 0;
    self->overflow_num = 0;
    self->tps = 0; 
    self->measured_speed = 0;
    self->target_speed = 0;
    self->current_pulse = 0;
    self->motor_id = 0; /* Default, should be set during configure */
    self->ticks_overflow = 0; 
    self->ticks_per_circle = AppConfig->motor_ticks_per_circle; /* Default for common motors */
    self->speed_limit = AppConfig->motor_speed_limit;
    self->deadzone = 0;
    pid_controller_init(&self->pid_controller, 0, 0, 0);
}

void encoder_motor_refresh_config(EncoderMotorObjectTypeDef *self) {
    self->ticks_per_circle = AppConfig->motor_ticks_per_circle;
    self->speed_limit = AppConfig->motor_speed_limit;
    
    /* Update per-motor PID gains */
    float kp = DEFAULT_MOTOR_KP, ki = DEFAULT_MOTOR_KI, kd = DEFAULT_MOTOR_KD, deadzone = 0;
    
    switch(self->motor_id) {
        case 1: kp = AppConfig->motor1_kp; ki = AppConfig->motor1_ki; kd = AppConfig->motor1_kd; deadzone = AppConfig->motor1_deadzone; break;
        case 2: kp = AppConfig->motor2_kp; ki = AppConfig->motor2_ki; kd = AppConfig->motor2_kd; deadzone = AppConfig->motor2_deadzone; break;
        case 3: kp = AppConfig->motor3_kp; ki = AppConfig->motor3_ki; kd = AppConfig->motor3_kd; deadzone = AppConfig->motor3_deadzone; break;
        case 4: kp = AppConfig->motor4_kp; ki = AppConfig->motor4_ki; kd = AppConfig->motor4_kd; deadzone = AppConfig->motor4_deadzone; break;
    }
    
    self->deadzone = deadzone;
    pid_controller_set_gains(&self->pid_controller, kp, ki, kd);
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
