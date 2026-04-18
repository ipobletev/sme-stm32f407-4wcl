#include "kinematics.h"
#include "config.h"

void Kinematics_Mecanum(float vx, float vy, float az, float *v_wheels) {
    /* Standard Mecanum Kinematics */
    float l_plus_w = ROBOT_WHEELBASE_LENGTH + ROBOT_SHAFT_WIDTH;

    v_wheels[0] = vx - vy - l_plus_w * az;
    v_wheels[1] = vx + vy - l_plus_w * az;
    v_wheels[2] = vx + vy + l_plus_w * az;
    v_wheels[3] = vx - vy + l_plus_w * az;
}
