#ifndef BSP_INTERNAL_FLASH_H
#define BSP_INTERNAL_FLASH_H

#include <stdint.h>
#include <stdbool.h>

/**
 * @brief STM32F407 Internal Flash Sector Definitions
 */
#define FLASH_SECTOR_CONFIG     7       /* Last sector of 512KB device */
#define FLASH_ADDRESS_CONFIG    0x08060000

/**
 * @brief Initialize Flash Interface
 */
void BSP_InternalFlash_Init(void);

/**
 * @brief Erase a specific flash sector
 * @param sector Sector number (e.g., FLASH_SECTOR_7)
 * @return true if successful
 */
bool BSP_InternalFlash_EraseSector(uint32_t sector);

/**
 * @brief Write data to flash (word by word)
 * @param address Destination flash address
 * @param data Source data buffer
 * @param length Length in bytes (should be multiple of 4)
 * @return true if successful
 */
bool BSP_InternalFlash_Write(uint32_t address, uint32_t* data, uint32_t length);

/**
 * @brief Read data from flash (simple memory copy for internal flash)
 * @param address Source flash address
 * @param data Destination buffer
 * @param length Length in bytes
 */
void BSP_InternalFlash_Read(uint32_t address, void* data, uint32_t length);

#endif /* BSP_INTERNAL_FLASH_H */
