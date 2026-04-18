#include "kinematics.h"
#include "config.h"

void Kinematics_Differential(float vx, float az, float *v_wheels) {
    /* Differential Drive (4WD mapping) */
    float track_width_half = ROBOT_SHAFT_WIDTH;

    v_wheels[0] = vx - track_width_half * az;
    v_wheels[1] = v_wheels[0];
    v_wheels[2] = vx + track_width_half * az;
    v_wheels[3] = v_wheels[2];
}
