#ifndef __BSP_IMU_H
#define __BSP_IMU_H

#include <stdint.h>
#include <stdbool.h>
#include "imu_types.h"

/* --- IMU TYPES --- */
typedef enum {
    IMU_TYPE_NONE = 0,
    IMU_TYPE_MPU6050,
    IMU_TYPE_QMI8658
} IMU_Type_t;

/* --- COMMON CONFIGURATION --- */
#define IMU_I2C_ADDR_QMI        0x6A << 1
#define IMU_I2C_ADDR_MPU        0x68 << 1

/* --- MPU6050 REGISTERS --- */
#define MPU_REG_SMPLRT_DIV      0x19
#define MPU_REG_CONFIG          0x1A
#define MPU_REG_GYRO_CONFIG     0x1B
#define MPU_REG_ACCEL_CONFIG    0x1C
#define MPU_REG_INT_ENABLE      0x38
#define MPU_REG_ACCEL_XOUT_H    0x3B
#define MPU_REG_PWR_MGMT_1      0x6B
#define MPU_REG_WHO_AM_I        0x75

/* --- QMI8658 REGISTERS --- */
#define QMI_REG_WHO_AM_I        0x00
#define QMI_REG_CTRL1           0x02
#define QMI_REG_CTRL2           0x03
#define QMI_REG_CTRL3           0x04
#define QMI_REG_CTRL4           0x05
#define QMI_REG_CTRL5           0x06
#define QMI_REG_CTRL7           0x08
#define QMI_REG_STATUS0         0x2D
#define QMI_REG_ACCEL_X_L       0x37
#define QMI_REG_GYRO_X_L        0x3D

/* --- DATA STRUCTURES --- */
typedef struct {
    int16_t ax, ay, az;
    int16_t gx, gy, gz;
    int16_t temp;
} IMU_RawData_t;

typedef enum {
    IMU_OK = 0,
    IMU_ERROR_I2C,
    IMU_ERROR_ID,
    IMU_ERROR_TIMEOUT
} IMU_Status_t;

/* Range Definitions (Used for scaling) */
#define MPU6050_GYRO_FSR_2000DPS  3
#define MPU6050_ACCEL_FSR_4G      1

/* BSP Interface */
IMU_Status_t BSP_IMU_Init(void);
IMU_Status_t BSP_IMU_ReadRaw(IMU_RawData_t *data);
IMU_Status_t BSP_IMU_ReadOrientation(float *pitch, float *roll, float *yaw); /* Radians */
IMU_Status_t BSP_IMU_ReadOrientationFull(Quaternion *q, EulerAngles *ea);
IMU_Status_t BSP_IMU_SetGyroFSR(uint8_t fsr, float *sf);
IMU_Status_t BSP_IMU_SetAccelFSR(uint8_t fsr, float *sf);
IMU_Status_t BSP_IMU_SetBias(float ax, float ay, float az, float gx, float gy, float gz);
IMU_Status_t BSP_IMU_SetRate(uint32_t rate_hz);
bool         BSP_IMU_IsDataReady(void);
IMU_Type_t   BSP_IMU_GetDetectedType(void);

#endif /* __BSP_IMU_H */
