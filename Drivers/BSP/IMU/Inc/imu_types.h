#ifndef __IMU_TYPES_H
#define __IMU_TYPES_H

typedef struct {
    float qx;
    float qy;
    float qz;
    float qw;
} Quaternion;

typedef struct {
    float roll;     /* Radians */
    float pitch;    /* Radians */
    float yaw;      /* Radians */
} EulerAngles;

#endif /* __IMU_TYPES_H */
