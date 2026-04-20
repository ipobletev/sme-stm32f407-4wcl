#include "bsp_internal_flash.h"
#include "stm32f4xx_hal.h"
#include <string.h>

void BSP_InternalFlash_Init(void) {
    /* HAL_Init usually initializes the flash interface, 
       but we can place any specific initialization here if needed. */
}

bool BSP_InternalFlash_EraseSector(uint32_t sector) {
    FLASH_EraseInitTypeDef erase_init;
    uint32_t sector_error = 0;
    HAL_StatusTypeDef status;

    HAL_FLASH_Unlock();

    erase_init.TypeErase    = FLASH_TYPEERASE_SECTORS;
    erase_init.Sector       = sector;
    erase_init.NbSectors    = 1;
    erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    status = HAL_FLASHEx_Erase(&erase_init, &sector_error);
    
    HAL_FLASH_Lock();

    return (status == HAL_OK);
}

bool BSP_InternalFlash_Write(uint32_t address, uint32_t* data, uint32_t length) {
    HAL_StatusTypeDef status = HAL_OK;

    HAL_FLASH_Unlock();

    /* Programmer in words (32-bit) */
    for (uint32_t i = 0; i < length; i += 4) {
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + i, *data);
        if (status != HAL_OK) {
            break;
        }
        data++;
    }

    HAL_FLASH_Lock();

    return (status == HAL_OK);
}

void BSP_InternalFlash_Read(uint32_t address, void* data, uint32_t length) {
    /* Internal flash is memory mapped, so we can just use memcpy */
    memcpy(data, (void*)address, length);
}
