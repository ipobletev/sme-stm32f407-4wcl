#include "kinematics.h"
// #include "app_config.h" // Removed dependency

void Kinematics_Direct(float vx, float az, float track_width, float *v_wheels, uint8_t num_wheels) {
    /* Standard differential-style mapping using half-track width */
    float track_width_half = track_width / 2.0f;
    float v_left = vx - track_width_half * az;
    float v_right = vx + track_width_half * az;

    if (num_wheels < 4) return;

    v_wheels[0] = v_left;  // Motor 1 (F-Left)
    v_wheels[1] = v_right; // Motor 2 (F-Right)
    v_wheels[2] = v_left;  // Motor 3 (B-Left)
    v_wheels[3] = v_right; // Motor 4 (B-Right)
}
