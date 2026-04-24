# Force re-configure after telemetry refactor
cmake_minimum_required(VERSION 3.22)
project(sme-stm32f407-4wcl C CXX ASM)

# Enable CMake support for ASM and C languages
enable_language(C CXX ASM)

# Add the executable target
add_executable(${CMAKE_PROJECT_NAME})

# Include directories
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
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
    Application/MainLogic/Slaves/MobilityStateMachine/Inc/States
    Application/MainLogic/Slaves/ArmStateMachine/Inc
    Application/MainLogic/Slaves/ArmStateMachine/Inc/States
    Application/Modules/OSAL/Inc
    Drivers/BSP/Inc
    Drivers/BSP/Led/Inc
    Drivers/BSP/Button/Inc
    Drivers/BSP/Buzzer/Inc
    Drivers/BSP/Console/Inc
    Drivers/BSP/SystemSensors/Inc
    Drivers/BSP/SerialRos/Inc
    Drivers/BSP/IMU/Inc
    Drivers/BSP/Motor/Inc
    Drivers/BSP/FlashStorage/Inc
    Application/Modules/MotorControl/Inc
    Application/Modules/SerialRos/Inc
    Application/Modules/ConsoleDebug/Inc
    Application/Modules/Buzzer/Inc
    Application/Config/Inc
    Application/Modules/USBJoystick/Inc
    Application/Modules/CommandDispatcher/Inc
    USB_HOST/App
    USB_HOST/Target
)

# Custom Application Source Files
file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    "Application/*.c"
    "Application/*.cpp"
)

# Custom BSP Source Files
file(GLOB_RECURSE BSP_SOURCES CONFIGURE_DEPENDS
    "Drivers/BSP/*.c"
)

# USB Host Source Files
file(GLOB_RECURSE USB_SOURCES CONFIGURE_DEPENDS
    "USB_HOST/*.c"
)

# Add custom sources to the target
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    ${APP_SOURCES}
    ${BSP_SOURCES}
    ${USB_SOURCES}
)

# Include the STM32CubeMX generated CMake logic
# This will add MX_Include_Dirs, MX_Application_Src, STM32_Drivers_Src, etc.
add_subdirectory(cmake/stm32cubemx)

# The generated logic already calls target_sources and target_link_libraries
# for ${CMAKE_PROJECT_NAME}, so we just need to ensure the order is correct.
# Updated state machine file structure - Standardizing ABORT Handlers and Diagnostic states.
