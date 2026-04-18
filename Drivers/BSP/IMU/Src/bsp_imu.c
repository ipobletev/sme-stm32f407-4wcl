#include "bsp_imu.h"
#include "i2c.h"
#include "qmi8658.h"
#include <string.h>
#include <stdio.h>
#include "debug_module.h"

#define LOG_TAG "BSP_IMU"

extern I2C_HandleTypeDef hi2c2;

static uint8_t detected_addr = 0;
static IMU_Type_t imu_type = IMU_TYPE_NONE;

/* Internal prototypes for MPU driver (kept as fallback) */
static IMU_Status_t MPU6050_Init(void);
static IMU_Status_t MPU6050_ReadRaw(IMU_RawData_t *data);

/**
 * @brief Manual I2C Bus Recovery (Clock Toggling).
 */
static void I2C_BusRecover(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_I2C2_CLK_DISABLE();
    GPIO_InitStruct.Pin = IMU_SCL_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    for(int i = 0; i < 9; i++) {
        HAL_GPIO_WritePin(GPIOB, IMU_SCL_Pin, GPIO_PIN_RESET);
        HAL_Delay(5);
        HAL_GPIO_WritePin(GPIOB, IMU_SCL_Pin, GPIO_PIN_SET);
        HAL_Delay(5);
    }
    __HAL_RCC_I2C2_CLK_ENABLE();
    MX_I2C2_Init();
}

/**
 * @brief Unified IMU Initialization with Scanner.
 */
IMU_Status_t BSP_IMU_Init(void) {
    uint8_t check;
    
    if (hi2c2.Instance->SR2 & I2C_SR2_BUSY) {
        I2C_BusRecover();
    }

    detected_addr = 0;
    imu_type = IMU_TYPE_NONE;

    LOG_INFO(LOG_TAG, "Scanning I2C2 Bus...\r\n");
    for (uint8_t addr = 0x68; addr <= 0x6B; addr++) {
        uint8_t target = addr << 1;
        if (HAL_I2C_IsDeviceReady(&hi2c2, target, 2, 5) == HAL_OK) {
            /* Try QMI Identify (User's board confirmed QMI at 0x6A) */
            if (HAL_I2C_Mem_Read(&hi2c2, target, 0x00, 1, &check, 1, 50) == HAL_OK) {
                if (check == 0x05) {
                    detected_addr = target;
                    imu_type = IMU_TYPE_QMI8658;
                    LOG_INFO(LOG_TAG, "QMI8658 detected at 0x%02X\r\n", addr);
                    if (qmi8658_begin()) return IMU_OK;
                    return IMU_ERROR_I2C;
                }
            }
            /* Try MPU Identify Fallback */
            if (HAL_I2C_Mem_Read(&hi2c2, target, 0x75, 1, &check, 1, 50) == HAL_OK) {
                if (check == 0x68 || (check >= 0x70 && check <= 0x73)) {
                    detected_addr = target;
                    imu_type = IMU_TYPE_MPU6050;
                    LOG_INFO(LOG_TAG, "MPU Family detected at 0x%02X (ID: 0x%02X)\r\n", addr, check);
                    return MPU6050_Init();
                }
            }
        }
    }

    LOG_INFO(LOG_TAG, "No known IMU sensor found.\r\n");
    return IMU_ERROR_ID;
}

IMU_Status_t BSP_IMU_ReadRaw(IMU_RawData_t *data) {
    if (imu_type == IMU_TYPE_QMI8658) {
        float acc[3], gyro[3];
        qmi8658_read_xyz(acc, gyro);
        // We pack floats back to raw to reuse calibration logic if needed, 
        // but task_imu can also take them directly. 
        // For now, let's keep consistency.
        data->ax = (int16_t)(acc[0] * 8192.0f / 9.807f); // Rough conversion back
        data->ay = (int16_t)(acc[1] * 8192.0f / 9.807f);
        data->az = (int16_t)(acc[2] * 8192.0f / 9.807f);
        data->gx = (int16_t)(gyro[0] * 16.4f / 0.01745f);
        data->gy = (int16_t)(gyro[1] * 16.4f / 0.01745f);
        data->gz = (int16_t)(gyro[2] * 16.4f / 0.01745f);
        return IMU_OK;
    }
    if (imu_type == IMU_TYPE_MPU6050) return MPU6050_ReadRaw(data);
    return IMU_ERROR_ID;
}

IMU_Status_t BSP_IMU_ReadOrientation(float *pitch, float *roll, float *yaw) {
    if (imu_type == IMU_TYPE_QMI8658) {
        qmi8658_get_euler(pitch, roll, yaw);
        return IMU_OK;
    }
    return IMU_ERROR_ID;
}

/* Fallback MPU6050 Driver */
static IMU_Status_t MPU6050_Init(void) {
    uint8_t data = 0x80;
    HAL_I2C_Mem_Write(&hi2c2, detected_addr, 0x6B, 1, &data, 1, 100);
    HAL_Delay(100);
    data = 0x00;
    HAL_I2C_Mem_Write(&hi2c2, detected_addr, 0x6B, 1, &data, 1, 100);
    return IMU_OK;
}

static IMU_Status_t MPU6050_ReadRaw(IMU_RawData_t *data) {
    uint8_t buf[14];
    if (HAL_I2C_Mem_Read(&hi2c2, detected_addr, 0x3B, 1, buf, 14, 100) != HAL_OK) return IMU_ERROR_I2C;
    data->ax = (int16_t)((buf[0] << 8) | buf[1]);
    data->ay = (int16_t)((buf[2] << 8) | buf[3]);
    data->az = (int16_t)((buf[4] << 8) | buf[5]);
    data->gx = (int16_t)((buf[8] << 8) | buf[9]);
    data->gy = (int16_t)((buf[10] << 8) | buf[11]);
    data->gz = (int16_t)((buf[12] << 8) | buf[13]);
    return IMU_OK;
}

IMU_Status_t BSP_IMU_SetGyroFSR(uint8_t fsr, float *sf) {
    if (imu_type == IMU_TYPE_QMI8658) { *sf = 16.0f; return IMU_OK; }
    *sf = 16.4f; return IMU_OK;
}

IMU_Status_t BSP_IMU_SetAccelFSR(uint8_t fsr, float *sf) {
    if (imu_type == IMU_TYPE_QMI8658) { *sf = 8192.0f; return IMU_OK; }
    *sf = 8192.0f; return IMU_OK;
}

IMU_Status_t BSP_IMU_SetRate(uint32_t rate_hz) { return IMU_OK; }
bool         BSP_IMU_IsDataReady(void) { return true; }
IMU_Type_t   BSP_IMU_GetDetectedType(void) { return imu_type; }
