#ifndef __KINEMATICS_H
#define __KINEMATICS_H

#include <stdint.h>

/**
 * @brief Common interface for mobility kinematic models.
 * Calculates wheel velocities (m/s) based on target velocities.
 * 
 * @param vx Linear velocity in X (m/s)
 * @param vy Linear velocity in Y (m/s) - only for holonomic
 * @param az Angular velocity in Z (rad/s)
 * @param v_wheels Array to store resulting wheel velocities [v1, v2, v3, v4]
 */

void Kinematics_Mecanum(float vx, float vy, float az, float *v_wheels);
void Kinematics_Differential(float vx, float az, float *v_wheels);
void Kinematics_Ackermann(float vx, float az, float *v_wheels);
void Kinematics_Direct(float vx, float az, float *v_wheels);

#endif /* __KINEMATICS_H */
