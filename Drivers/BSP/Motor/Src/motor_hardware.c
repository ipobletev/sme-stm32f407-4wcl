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

/* Channel Mappings */
#define MOTOR1_BI_CHANNEL TIM_CHANNEL_3
#define MOTOR1_FI_CHANNEL TIM_CHANNEL_4

#define MOTOR2_BI_CHANNEL TIM_CHANNEL_1
#define MOTOR2_FI_CHANNEL TIM_CHANNEL_2

#define MOTOR3_FI_CHANNEL TIM_CHANNEL_1
#define MOTOR3_BI_CHANNEL TIM_CHANNEL_2

#define MOTOR4_BI_CHANNEL TIM_CHANNEL_1 /* htim10 */
#define MOTOR4_FI_CHANNEL TIM_CHANNEL_1 /* htim11 */

void Motor_Hardware_SetParam(EncoderMotorObjectTypeDef *motor, int32_t tpc, float rps_limit, float kp, float ki, float kd)
{
    motor->ticks_per_circle = tpc;
    motor->rps_limit = rps_limit;
    motor->pid_controller.kp = kp;
    motor->pid_controller.ki = ki;
    motor->pid_controller.kd = kd;
}

void Motor_Hardware_SetType(EncoderMotorObjectTypeDef *motor, MotorTypeEnum type) 
{
    switch(type) {
        case MOTOR_TYPE_JGB520:
            Motor_Hardware_SetParam(motor, MOTOR_JGB520_TICKS_PER_CIRCLE, MOTOR_JGB520_RPS_LIMIT, MOTOR_JGB520_PID_KP, MOTOR_JGB520_PID_KI, MOTOR_JGB520_PID_KD);
            break;
        case MOTOR_TYPE_JGB37:
            Motor_Hardware_SetParam(motor, MOTOR_JGB37_TICKS_PER_CIRCLE, MOTOR_JGB37_RPS_LIMIT, MOTOR_JGB37_PID_KP, MOTOR_JGB37_PID_KI, MOTOR_JGB37_PID_KD);
            break;
        case MOTOR_TYPE_JGA27:
            Motor_Hardware_SetParam(motor, MOTOR_JGA27_TICKS_PER_CIRCLE, MOTOR_JGA27_RPS_LIMIT, MOTOR_JGA27_PID_KP, MOTOR_JGA27_PID_KI, MOTOR_JGA27_PID_KD);
            break;
        case MOTOR_TYPE_JGB528:
            Motor_Hardware_SetParam(motor, MOTOR_JGB528_TICKS_PER_CIRCLE, MOTOR_JGB528_RPS_LIMIT, MOTOR_JGB528_PID_KP, MOTOR_JGB528_PID_KI, MOTOR_JGB528_PID_KD);
            break;
        default:
            break;
    }
}

void Motor_Hardware_Init(EncoderMotorObjectTypeDef *motors[4])
{
    /* Initialize motor pointers and set overflow limits */
    for(int i = 0; i < 4; ++i) {
        motors[i]->ticks_overflow = 60000;
    }

    motors[0]->set_pulse = motor1_set_pulse;
    motors[1]->set_pulse = motor2_set_pulse;
    motors[2]->set_pulse = motor3_set_pulse;
    motors[3]->set_pulse = motor4_set_pulse;

    /* Motor 1 & 2 (TIM1) */
    __HAL_TIM_SET_COUNTER(&htim1, 0);
    __HAL_TIM_ENABLE(&htim1);
    __HAL_TIM_MOE_ENABLE(&htim1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_2);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);
    
    /* Motor 3 (TIM9) */
    __HAL_TIM_SET_COUNTER(&htim9, 0);
    __HAL_TIM_ENABLE(&htim9);
    __HAL_TIM_MOE_ENABLE(&htim9);
    HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim9, TIM_CHANNEL_2);
    
    /* Motor 4 (TIM10 & TIM11) */
    __HAL_TIM_SET_COUNTER(&htim10, 0);
    __HAL_TIM_SET_COUNTER(&htim11, 0);
    __HAL_TIM_ENABLE(&htim10);
    __HAL_TIM_ENABLE(&htim11);
    __HAL_TIM_MOE_ENABLE(&htim10);
    __HAL_TIM_MOE_ENABLE(&htim11);
    HAL_TIM_PWM_Start(&htim10, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(&htim11, TIM_CHANNEL_1);

    /* Encoder 1 (TIM5) */
    __HAL_TIM_SET_COUNTER(&htim5, 0);
    __HAL_TIM_CLEAR_IT(&htim5, TIM_IT_UPDATE);
    __HAL_TIM_ENABLE_IT(&htim5, TIM_IT_UPDATE);
    HAL_TIM_Encoder_Start(&htim5, TIM_CHANNEL_ALL);

    /* Encoder 2 (TIM2) */
    __HAL_TIM_SET_COUNTER(&htim2, 0);
    __HAL_TIM_CLEAR_IT(&htim2, TIM_IT_UPDATE);
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_UPDATE);
    HAL_TIM_Encoder_Start(&htim2, TIM_CHANNEL_ALL);

    /* Encoder 3 (TIM4) */
    __HAL_TIM_SET_COUNTER(&htim4, 0);
    __HAL_TIM_CLEAR_IT(&htim4, TIM_IT_UPDATE);
    __HAL_TIM_ENABLE_IT(&htim4, TIM_IT_UPDATE);
    HAL_TIM_Encoder_Start(&htim4, TIM_CHANNEL_ALL);

    /* Encoder 4 (TIM3) */
    __HAL_TIM_SET_COUNTER(&htim3, 0);
    __HAL_TIM_CLEAR_IT(&htim3, TIM_IT_UPDATE);
    __HAL_TIM_ENABLE_IT(&htim3, TIM_IT_UPDATE);
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
        __HAL_TIM_SET_COMPARE(&htim1, MOTOR1_BI_CHANNEL, 0);
        __HAL_TIM_SET_COMPARE(&htim1, MOTOR1_FI_CHANNEL, speed);
    } else if(speed < 0) {
        __HAL_TIM_SET_COMPARE(&htim1, MOTOR1_FI_CHANNEL, 0);
        __HAL_TIM_SET_COMPARE(&htim1, MOTOR1_BI_CHANNEL, -speed);
    } else {
        __HAL_TIM_SET_COMPARE(&htim1, MOTOR1_BI_CHANNEL, 0);
        __HAL_TIM_SET_COMPARE(&htim1, MOTOR1_FI_CHANNEL, 0);
    }
}

static void motor2_set_pulse(EncoderMotorObjectTypeDef *self, int speed)
{
    if(speed > 0) {
        __HAL_TIM_SET_COMPARE(&htim1, MOTOR2_BI_CHANNEL, 0);
        __HAL_TIM_SET_COMPARE(&htim1, MOTOR2_FI_CHANNEL, speed);
    } else if(speed < 0) {
        __HAL_TIM_SET_COMPARE(&htim1, MOTOR2_FI_CHANNEL, 0);
        __HAL_TIM_SET_COMPARE(&htim1, MOTOR2_BI_CHANNEL, -speed);
    } else {
        __HAL_TIM_SET_COMPARE(&htim1, MOTOR2_BI_CHANNEL, 0);
        __HAL_TIM_SET_COMPARE(&htim1, MOTOR2_FI_CHANNEL, 0);
    }
}

static void motor3_set_pulse(EncoderMotorObjectTypeDef *self, int speed)
{
    if(speed > 0) {
        __HAL_TIM_SET_COMPARE(&htim9, MOTOR3_BI_CHANNEL, 0);
        __HAL_TIM_SET_COMPARE(&htim9, MOTOR3_FI_CHANNEL, speed);
    } else if(speed < 0) {
        __HAL_TIM_SET_COMPARE(&htim9, MOTOR3_FI_CHANNEL, 0);
        __HAL_TIM_SET_COMPARE(&htim9, MOTOR3_BI_CHANNEL, -speed);
    } else {
        __HAL_TIM_SET_COMPARE(&htim9, MOTOR3_FI_CHANNEL, 0);
        __HAL_TIM_SET_COMPARE(&htim9, MOTOR3_BI_CHANNEL, 0);
    }
}

static void motor4_set_pulse(EncoderMotorObjectTypeDef *self, int speed)
{
    if(speed > 0) {
        __HAL_TIM_SET_COMPARE(&htim10, MOTOR4_BI_CHANNEL, 0);
        __HAL_TIM_SET_COMPARE(&htim11, MOTOR4_FI_CHANNEL, speed);
    } else if(speed < 0) {
        __HAL_TIM_SET_COMPARE(&htim11, MOTOR4_FI_CHANNEL, 0);
        __HAL_TIM_SET_COMPARE(&htim10, MOTOR4_BI_CHANNEL, -speed);
    } else {
        __HAL_TIM_SET_COMPARE(&htim10, MOTOR4_BI_CHANNEL, 0);
        __HAL_TIM_SET_COMPARE(&htim11, MOTOR4_FI_CHANNEL, 0);
    }
}
