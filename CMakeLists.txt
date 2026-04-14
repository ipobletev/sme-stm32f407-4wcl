cmake_minimum_required(VERSION 3.22)

#
# This file is generated only once,
# and is not re-generated if converter is called multiple times.
#
# User is free to modify the file as much as necessary
#

# Setup compiler settings
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)


# Define the build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Debug")
endif()

# Set the project name
set(CMAKE_PROJECT_NAME sme-stm32f407-4wcl)

# Enable compile command to ease indexing with e.g. clangd
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

# Core project settings
project(${CMAKE_PROJECT_NAME})
message("Build type: " ${CMAKE_BUILD_TYPE})

# Enable CMake support for ASM and C languages
enable_language(C ASM)

# Create an executable object type
add_executable(${CMAKE_PROJECT_NAME})

# Add STM32CubeMX generated sources
add_subdirectory(cmake/stm32cubemx)

# Link directories setup
target_link_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined library search paths
)

# Add sources to executable
target_sources(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user sources here
    Application/RTOSLogic/Src/task_manager.c
    Application/RTOSLogic/Src/task_controller.c
    Application/RTOSLogic/Src/app_rtos.c
    Application/RTOSLogic/Src/timer_heartbeat.c
    Application/RTOSLogic/Src/task_default.c
    Application/RTOSLogic/Src/task_uart_listener.c
    Application/RTOSLogic/Src/task_mobility.c
    Application/RTOSLogic/Src/task_arm.c
    Application/Modules/OSAL/Src/osal_cmsis.c

    Application/MainLogic/Supervisor/Src/supervisor_fsm.c
    Application/Core/Src/robot_state.c
    Application/MainLogic/Supervisor/Src/States/state_init.c
    Application/MainLogic/Supervisor/Src/States/state_idle.c
    Application/MainLogic/Supervisor/Src/States/state_manual.c
    Application/MainLogic/Supervisor/Src/States/state_auto.c
    Application/MainLogic/Supervisor/Src/States/state_paused.c
    Application/MainLogic/Supervisor/Src/States/state_fault.c
    Application/MainLogic/Slaves/MobilityStateMachine/Src/mobility_fsm.c
    Application/MainLogic/Slaves/ArmStateMachine/Src/arm_fsm.c


    Drivers/BSP/Led/Src/bsp_led.c

    Drivers/BSP/Button/Src/bsp_button.c
    Drivers/BSP/Buzzer/Src/bsp_buzzer.c
    Drivers/BSP/Console/Src/bsp_console.c
    Application/Modules/ConsoleDebug/Src/debug_module.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined include paths
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
    Application/Modules/ConsoleDebug/Inc
    Application/Config
)


# Add project symbols (macros)
target_compile_definitions(${CMAKE_PROJECT_NAME} PRIVATE
    # Add user defined symbols
)

# Remove wrong libob.a library dependency when using cpp files
list(REMOVE_ITEM CMAKE_C_IMPLICIT_LINK_LIBRARIES ob)

# Add linked libraries
target_link_libraries(${CMAKE_PROJECT_NAME}
    stm32cubemx

    # Add user defined libraries
)
