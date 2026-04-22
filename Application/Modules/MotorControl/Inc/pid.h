#ifndef _PID_H
#define _PID_H

#include <stdint.h>

/**
  * @brief PID 控制器结构体
  * 
  * Note: This is an Incremental PID implementation.
  */
typedef struct {
    float set_point; /**< @brief 目标值 */
    float kp;        /**< @brief 比例增益 */
    float ki;        /**< @brief 积分增益 */
    float kd;        /**< @brief 微分增益 */
    
    float previous_0_err; /**< @brief 上次误差 e[k-1] */
    float previous_1_err; /**< @brief 上上次误差 e[k-2] */
    
    float output; /**< @brief PID输出 (增量 Δu) */
} PID_ControllerTypeDef;

void pid_controller_update(PID_ControllerTypeDef *self, float actual, float time_delta);
void pid_controller_reset(PID_ControllerTypeDef *self);
void pid_controller_init(PID_ControllerTypeDef *self, float kp, float ki, float kd);
void pid_controller_set_gains(PID_ControllerTypeDef *self, float kp, float ki, float kd);

#endif /* _PID_H */
