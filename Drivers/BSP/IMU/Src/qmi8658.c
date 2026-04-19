#include "qmi8658.h"
#include <math.h>
#include <string.h>
#include "i2c.h"
#include "osal.h"

#ifndef M_PI
#define M_PI			(3.14159265358979323846f)
#endif
#define ONE_G			(9.807f)

extern I2C_HandleTypeDef hi2c2;
static qmi8658_state g_imu;

/* Filter Constants */
#define Kp 10.0f               
#define Ki 0.008f            
#define halfT 0.005f        /* 100Hz = 10ms period -> halfT = 0.005s */

static void write_reg(uint8_t reg, uint8_t value) {
    HAL_I2C_Mem_Write(&hi2c2, QMI8658_SLAVE_ADDR_L << 1, reg, I2C_MEMADD_SIZE_8BIT, &value, 1, 100);
}

static uint8_t read_reg(uint8_t reg) {
    uint8_t ret = 0;
    HAL_I2C_Mem_Read(&hi2c2, QMI8658_SLAVE_ADDR_L << 1, reg, I2C_MEMADD_SIZE_8BIT, &ret, 1, 100);
    return ret;
}

static uint16_t readWord_reg(uint8_t reg) {
    uint8_t ret[2] = {0, 0};
    HAL_I2C_Mem_Read(&hi2c2, QMI8658_SLAVE_ADDR_L << 1, reg, I2C_MEMADD_SIZE_8BIT, ret, 2, 100);
    return ((uint16_t)ret[1] << 8) | ret[0]; // Little-Endian
}

static float invSqrt(float number) {
    volatile long i;
    volatile float x, y;
    volatile const float f = 1.5F;
    x = number * 0.5F;
    y = number;
    i = * (( long * ) &y);
    i = 0x5f375a86 - ( i >> 1 );
    y = * (( float * ) &i);
    y = y * ( f - ( x * y * y ) );
    return y;
}

static void get_orientation_internal(float gx, float gy, float gz, float ax, float ay, float az, Quaternion *q, EulerAngles *ea) {
    static float q0 = 1.0f, q1 = 0.0f, q2 = 0.0f, q3 = 0.0f;
    static float exInt = 0, eyInt = 0, ezInt = 0;
    float recipNorm;
    float vx, vy, vz;
    float ex, ey, ez;

    if (ax*ax + ay*ay + az*az == 0) return;

    recipNorm = invSqrt(ax*ax + ay*ay + az*az);
    ax *= recipNorm; ay *= recipNorm; az *= recipNorm;

    vx = 2*(q1*q3 - q0*q2);
    vy = 2*(q0*q1 + q2*q3);
    vz = q0*q0 - q1*q1 - q2*q2 + q3*q3;

    ex = ay*vz - az*vy;
    ey = az*vx - ax*vz;
    ez = ax*vy - ay*vx;

    exInt += ex * Ki;
    eyInt += ey * Ki;
    ezInt += ez * Ki;

    gx = gx + Kp*ex + exInt;
    gy = gy + Kp*ey + eyInt;
    gz = gz + Kp*ez + ezInt;

    q0 = q0 + (-q1*gx - q2*gy - q3*gz)*halfT;
    q1 = q1 + (q0*gx + q2*gz - q3*gy)*halfT;
    q2 = q2 + (q0*gy - q1*gz + q3*gx)*halfT;
    q3 = q3 + (q0*gz + q1*gy - q2*gx)*halfT;

    recipNorm = invSqrt(q0*q0 + q1*q1 + q2*q2 + q3*q3);
    q0 *= recipNorm; q1 *= recipNorm; q2 *= recipNorm; q3 *= recipNorm;

    if (q) {
        q->qw = q0; q->qx = q1; q->qy = q2; q->qz = q3;
    }
    if (ea) {
        ea->roll = atan2f(2*q2*q3 + 2*q0*q1, -2*q1*q1 - 2*q2*q2 + 1);
        ea->pitch = asinf(2*q1*q3 - 2*q0*q2);
        ea->yaw = -atan2f(2*q1*q2 + 2*q0*q3, -2*q2*q2 - 2*q3*q3 + 1);
    }
}

void qmi8658_get_orientation(Quaternion *q, EulerAngles *ea) {
    float acc[3], gyro[3];
    qmi8658_read_xyz(acc, gyro);
    get_orientation_internal(gyro[0], gyro[1], gyro[2], acc[0], acc[1], acc[2], q, ea);
}

static void qmi8658_on_demand_cali(void) {
    write_reg(Qmi8658Register_Reset, 0xb0);
    osal_delay(10);
    write_reg(Qmi8658Register_Ctrl9, (unsigned char)qmi8658_Ctrl9_Cmd_On_Demand_Cali);
    osal_delay(2200);
    write_reg(Qmi8658Register_Ctrl9, (unsigned char)qmi8658_Ctrl9_Cmd_NOP);
    osal_delay(100);
}

unsigned char qmi8658_begin(void) {
    if (read_reg(Qmi8658Register_WhoAmI) != 0x05) return 0;
    
    qmi8658_on_demand_cali();

    write_reg(Qmi8658Register_Ctrl1, 0x60); // Default interface settings
    
    // Accel: 8g, 1000Hz
    g_imu.ssvt_a = (1<<12);
    write_reg(Qmi8658Register_Ctrl2, Qmi8658AccRange_8g | Qmi8658AccOdr_1000Hz);
    
    // Gyro: 1024dps, 1000Hz
    g_imu.ssvt_g = 32;
    write_reg(Qmi8658Register_Ctrl3, Qmi8658GyrRange_2048dps | Qmi8658GyrOdr_1000Hz);
    g_imu.ssvt_g = 16; // 2048dps
    
    write_reg(Qmi8658Register_Ctrl5, 0x11); // LPF enabled
    write_reg(Qmi8658Register_Ctrl7, QMI8658_ACCGYR_ENABLE);

    return 1;
}

void qmi8658_read_xyz(float acc[3], float gyro[3]) {
    short raw_acc[3], raw_gyro[3];
    raw_acc[0] = (short)readWord_reg(Qmi8658Register_Ax_L);
    raw_acc[1] = (short)readWord_reg(Qmi8658Register_Ay_L);
    raw_acc[2] = (short)readWord_reg(Qmi8658Register_Az_L);
    raw_gyro[0] = (short)readWord_reg(Qmi8658Register_Gx_L);
    raw_gyro[1] = (short)readWord_reg(Qmi8658Register_Gy_L);
    raw_gyro[2] = (short)readWord_reg(Qmi8658Register_Gz_L);

    for (int i=0; i<3; i++) {
        acc[i] = ((float)(raw_acc[i] * ONE_G) / g_imu.ssvt_a) - g_imu.bias_a[i];
        gyro[i] = ((float)(raw_gyro[i] * M_PI) / (g_imu.ssvt_g * 180.0f)) - g_imu.bias_g[i];
        g_imu.imu[i] = acc[i];
        g_imu.imu[i+3] = gyro[i];
    }
}

void qmi8658_get_euler(float *pitch, float *roll, float *yaw) {
    EulerAngles ea;
    qmi8658_get_orientation(NULL, &ea);
    if (pitch) *pitch = ea.pitch;
    if (roll)  *roll  = ea.roll;
    if (yaw)   *yaw   = ea.yaw;
}

void qmi8658_set_bias(float ax, float ay, float az, float gx, float gy, float gz) {
    g_imu.bias_a[0] = ax;
    g_imu.bias_a[1] = ay;
    g_imu.bias_a[2] = az;
    g_imu.bias_g[0] = gx;
    g_imu.bias_g[1] = gy;
    g_imu.bias_g[2] = gz;
}
