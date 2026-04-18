#include "kinematics.h"

void Kinematics_Direct(float vx, float az, float *v_wheels) {
    /* Pass-through linear to all, angular as differential */
    v_wheels[0] = vx - az;
    v_wheels[1] = vx - az;
    v_wheels[2] = vx + az;
    v_wheels[3] = vx + az;
}
