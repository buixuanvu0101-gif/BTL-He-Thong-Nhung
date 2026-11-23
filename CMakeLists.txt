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
set(CMAKE_PROJECT_NAME BTL_Nhung)

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
    # HAL drivers
    hal/src/hal_gpio.c
    hal/src/hal_uart.c
    hal/src/hal_buzzer.c
    hal/src/hal_i2c.c
    hal/src/hal_i2c_stm32.c

    # BSP drivers
    BSP/Src/bsp_button.c
    BSP/Src/bsp_alarm.c
    BSP/Src/bsp_alarm_stm32.c
    BSP/Src/bsp_oled.c

    # Application & Tasks
    App/Src/app_ipc.c
    App/Src/sensor_dht11.c
    App/Src/sensor_hcsr04.c
    App/Src/sensor_hcsr04_callback.c
    App/Src/task_sensor.c
    App/Src/button_logic.c
    App/Src/display_format.c
    App/Src/display_outputs.c
    App/Src/task_display.c
    App/Src/comm_parser.c
    App/Src/task_comm.c
)

# Add include paths
target_include_directories(${CMAKE_PROJECT_NAME} PRIVATE
    hal/inc
    BSP/Inc
    App/Inc
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