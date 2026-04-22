#include "kinematics.h"
// #include "app_config.h" // Removed dependency

void Kinematics_Mecanum(float vx, float vy, float az, float wheelbase, float track_width, float *v_wheels, uint8_t num_wheels) {
    /* Standard Mecanum Kinematics (Requires at least 4 wheels) */
    if (num_wheels < 4) return;

    float l_plus_w = (wheelbase + track_width) / 2.0f;

    v_wheels[0] = vx - vy - l_plus_w * az; // Front-Left
    v_wheels[1] = vx + vy + l_plus_w * az; // Front-Right
    v_wheels[2] = vx + vy - l_plus_w * az; // Back-Left
    v_wheels[3] = vx - vy + l_plus_w * az; // Back-Right
    
    /* If more than 4, additional pairs follow the same logic as front/back pairs */
    for (int i = 4; i < num_wheels; i++) {
        v_wheels[i] = v_wheels[i % 4];
    }
}
