#include "pid.h"

/**
 * @brief Incremental PID calculation
 * Δu = Kp*(e[k]-e[k-1]) + Ki*e[k]*T + Kd*(e[k]-2e[k-1]+e[k-2])/T
 * 
 * NOTE: The test code implementation was slightly different in its interpretation 
 * of the P and D terms for the incremental output. I will stick to the reference 
 * code logic to ensure consistency with existing motor parameters.
 */
void pid_controller_update(PID_ControllerTypeDef *self, float actual, float time_delta) {
    float err = self->set_point - actual;
    
    /* 
     * Incremental PID components for Δu 
     * 
     * NOTE FOR VELOCITY CONTROL:
     * - The KP term here acts on (e[k] - e[k-1]). This provides a high-frequency "spike" 
     *   during setpoint changes but actually acts as a damper as the motor speeds up.
     *   This is mathematically equivalent to a standard P-controller u = Kp*e.
     * 
     * - The KI term here acts on e[k]. Because this is the INCREMENT (Δu), 
     *   this term accumulates effort as long as error exists.
     *   This is the term that will "rise" the PWM to reach the target velocity.
     */
    float integral = err * time_delta;
    float derivative = (err - 2.0f * self->previous_0_err + self->previous_1_err) / time_delta;
    
    /* Calculate the change in control effort (Δu) */
    self->output = (self->kp * (err - self->previous_0_err)) + (self->ki * integral) + (self->kd * derivative);
                   
    self->previous_1_err = self->previous_0_err;
    self->previous_0_err = err;
}


void pid_controller_reset(PID_ControllerTypeDef *self) {
    self->previous_0_err = 0;
    self->previous_1_err = 0;
    self->output = 0;
}

void pid_controller_init(PID_ControllerTypeDef *self, float kp, float ki, float kd) {
    self->set_point = 0;
    self->kp = kp;
    self->ki = ki;
    self->kd = kd;
    self->previous_0_err = 0;
    self->previous_1_err = 0;
    self->output = 0;
}
void pid_controller_set_gains(PID_ControllerTypeDef *self, float kp, float ki, float kd) {
    self->kp = kp;
    self->ki = ki;
    self->kd = kd;
}
