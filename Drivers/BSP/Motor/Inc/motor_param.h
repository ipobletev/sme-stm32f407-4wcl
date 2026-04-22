#ifndef __MOTOR_PARAM_H_
#define __MOTOR_PARAM_H_

#include "encoder_motor.h"

/**
 * @brief Supported Motor Types
 */
typedef enum {
    MOTOR_TYPE_JGB520 = 0,
    MOTOR_TYPE_JGB37,
    MOTOR_TYPE_JGA27,
    MOTOR_TYPE_JGB528,
    MOTOR_TYPE_COUNT
} MotorTypeEnum;

/**
 * JGB520 Parameters (Default for Jetauto)
 */
#define MOTOR_JGB520_TICKS_PER_CIRCLE   3960.0f
#define MOTOR_JGB520_PID_KP             41000.0f
#define MOTOR_JGB520_PID_KI             1700.0f
#define MOTOR_JGB520_PID_KD             1500.0f
#define MOTOR_JGB520_SPEED_LIMIT          1.5f

/**
 * JGB37 Parameters
 */
#define MOTOR_JGB37_TICKS_PER_CIRCLE    1980.0f
#define MOTOR_JGB37_PID_KP              26000.0f
#define MOTOR_JGB37_PID_KI              1300.0f
#define MOTOR_JGB37_PID_KD              1300.0f
#define MOTOR_JGB37_SPEED_LIMIT           3.0f

/**
 * JGA27 Parameters
 */
#define MOTOR_JGA27_TICKS_PER_CIRCLE    1040.0f
#define MOTOR_JGA27_PID_KP              -23000.0f
#define MOTOR_JGA27_PID_KI              -650.0f
#define MOTOR_JGA27_PID_KD              -650.0f
#define MOTOR_JGA27_SPEED_LIMIT           6.0f 

/**
 * JGB528 Parameters
 */
#define MOTOR_JGB528_TICKS_PER_CIRCLE   5764.0f
#define MOTOR_JGB528_PID_KP             195000.0f
#define MOTOR_JGB528_PID_KI             1300.0f
#define MOTOR_JGB528_PID_KD             7800.0f
#define MOTOR_JGB528_SPEED_LIMIT          1.1f 

#endif /* __MOTOR_PARAM_H_ */
