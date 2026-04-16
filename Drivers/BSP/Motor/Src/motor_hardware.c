#include "motor_hardware.h"
#include "main.h"
#include <stdbool.h>

/* Timer handles from tim.c */
extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim4;
extern TIM_HandleTypeDef htim5;
extern TIM_HandleTypeDef htim9;
extern TIM_HandleTypeDef htim10;
extern TIM_HandleTypeDef htim11;

/* Pulse setters for each motor */
static void motor1_set_pulse(EncoderMotorObjectTypeDef *self, int speed);
static void motor2_set_pulse(EncoderMotorObjectTypeDef *self, int speed);
static void motor3_set_pulse(EncoderMotorObjectTypeDef *self, int speed);
static void motor4_set_pulse(EncoderMotorObjectTypeDef *self, int speed);

void Motor_Hardware_Init(EncoderMotorObjectTypeDef *motors[4])
{
    /* Initialize motor pointers and methods */
    motors[0]->set_pulse = motor1_set_pulse;
    motors[1]->set_pulse = motor2_set_pulse;
    motors[2]->set_pulse = motor3_set_pulse;
    motors[3]->set_pulse = motor4_set_pulse;

    /* Start PWM Timers */
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
    
    HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2);
    
    HAL_TIM_PWM_Start(&htim10, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim11, TIM_CHANNEL_1);

    /* Start Encoder Timers */
    HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);
    HAL_TIM_Encoder_Start(&htim3, TIM_CHANNEL_ALL);
}

static int64_t accumulated_counts[4] = {0, 0, 0, 0};
static uint32_t last_raw_values[4] = {0, 0, 0, 0};

int64_t Motor_Hardware_GetEncoderCount(uint8_t motor_idx)
{
    uint32_t now = 0;
    bool is_16_bit = false;
    
    switch(motor_idx) {
        case 0: now = __HAL_TIM_GET_COUNTER(&htim5); is_16_bit = false; break;
        case 1: now = __HAL_TIM_GET_COUNTER(&htim2); is_16_bit = false; break;
        case 2: now = __HAL_TIM_GET_COUNTER(&htim4); is_16_bit = true;  break;
        case 3: now = __HAL_TIM_GET_COUNTER(&htim3); is_16_bit = true;  break;
        default: return 0;
    }
    
    int32_t delta = 0;
    if (is_16_bit) {
        /* Handle 16-bit wrap-around by casting to int16_t before promotion */
        delta = (int16_t)(now - last_raw_values[motor_idx]);
    } else {
        /* Handle 32-bit wrap-around by casting to int32_t before promotion */
        delta = (int32_t)(now - last_raw_values[motor_idx]);
    }
    
    accumulated_counts[motor_idx] += delta;
    last_raw_values[motor_idx] = now;

    return accumulated_counts[motor_idx];
}

/* Hardcoded PWM setters based on board pinout */

static void motor1_set_pulse(EncoderMotorObjectTypeDef *self, int speed)
{
    if(speed > 0) {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, speed);
    } else if(speed < 0) {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, -speed);
    } else {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_3, 0);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, 0);
    }
}

static void motor2_set_pulse(EncoderMotorObjectTypeDef *self, int speed)
{
    if(speed > 0) {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, speed);
    } else if(speed < 0) {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, -speed);
    } else {
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, 0);
        __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_2, 0);
    }
}

static void motor3_set_pulse(EncoderMotorObjectTypeDef *self, int speed)
{
    if(speed > 0) {
        __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_2, 0);
        __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_1, speed);
    } else if(speed < 0) {
        __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_1, 0);
        __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_2, -speed);
    } else {
        __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_1, 0);
        __HAL_TIM_SET_COMPARE(&htim9, TIM_CHANNEL_2, 0);
    }
}

static void motor4_set_pulse(EncoderMotorObjectTypeDef *self, int speed)
{
    if(speed > 0) {
        __HAL_TIM_SET_COMPARE(&htim10, TIM_CHANNEL_1, 0);
        __HAL_TIM_SET_COMPARE(&htim11, TIM_CHANNEL_1, speed);
    } else if(speed < 0) {
        __HAL_TIM_SET_COMPARE(&htim11, TIM_CHANNEL_1, 0);
        __HAL_TIM_SET_COMPARE(&htim10, TIM_CHANNEL_1, -speed);
    } else {
        __HAL_TIM_SET_COMPARE(&htim10, TIM_CHANNEL_1, 0);
        __HAL_TIM_SET_COMPARE(&htim11, TIM_CHANNEL_1, 0);
    }
}
