#ifndef __CHASSIS_CONFIG_H_
#define __CHASSIS_CONFIG_H_

/**
 * Common Motor Parameters (JGB520 default for Jetauto)
 */
#define MOTOR_TICKS_PER_CIRCLE 3960.0f
#define MOTOR_PID_KP           63.0f
#define MOTOR_PID_KI           2.6f
#define MOTOR_PID_KD           2.4f
#define MOTOR_RPS_LIMIT        1.5f

/**
 * Chassis Physical Parameters (Jetauto Mecanum)
 */
#define JETAUTO_WHEEL_DIAMETER   0.096f  /* 96mm */
#define JETAUTO_SHAFT_LENGTH     0.170f  /* 170mm half length or track width? 
                                            Reference code usually uses constants. */
#define JETAUTO_WHEELBASE        0.150f

#endif /* __CHASSIS_CONFIG_H_ */
