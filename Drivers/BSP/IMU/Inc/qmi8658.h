#ifndef _QMI8658_H_
#define _QMI8658_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "qmi8658reg.h"

typedef struct {
    float roll;
    float pitch;
    float yaw;
} EulerAngles;

/* BSP Public Interface for QMI8658 */
unsigned char qmi8658_begin(void);
void          qmi8658_read_xyz(float acc[3], float gyro[3]);
void          qmi8658_get_euler(float *pitch, float *roll, float *yaw);

#endif
