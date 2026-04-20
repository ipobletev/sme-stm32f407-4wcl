#include "kinematics.h"
#include "app_config.h"

void Kinematics_Mecanum(float vx, float vy, float az, float *v_wheels) {
    /* Standard Mecanum Kinematics */
    float l_plus_w = AppConfig->wheelbase_length + AppConfig->shaft_width;

    v_wheels[0] = vx - vy - l_plus_w * az;
    v_wheels[1] = vx + vy - l_plus_w * az;
    v_wheels[2] = vx + vy + l_plus_w * az;
    v_wheels[3] = vx - vy + l_plus_w * az;
}
