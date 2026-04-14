cmake_minimum_required(VERSION 3.22)
project(sme-stm32f407-4wcl C CXX ASM)

# Add all include directories
include_directories(
    Core/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc
    Drivers/STM32F4xx_HAL_Driver/Inc/Legacy
    Middlewares/Third_Party/FreeRTOS/Source/include
    Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2
    Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F
    Drivers/CMSIS/Device/ST/STM32F4xx/Include
    Drivers/CMSIS/Include
    Application/RTOSLogic/Inc
    Application/Core/Inc
    Application/MainLogic/Supervisor/Inc
    Application/MainLogic/Supervisor/Inc/States
    Application/MainLogic/Slaves/MobilityStateMachine/Inc
    Application/MainLogic/Slaves/ArmStateMachine/Inc
    Application/Modules/OSAL/Inc
    Drivers/BSP/Inc
    Drivers/BSP/Led/Inc
    Drivers/BSP/Button/Inc
    Drivers/BSP/Buzzer/Inc
    Drivers/BSP/Console/Inc
    Drivers/BSP/SystemSensors/Inc
    Drivers/BSP/SerialRos/Inc
    Application/Modules/SerialRos/Inc
    Application/Modules/ConsoleDebug/Inc
    Application/Config
)

# This is a placeholder for source files and executable definition
# to be expanded based on the full project structure if needed.
