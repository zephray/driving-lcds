# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/Users/andylithia/pico/pico-sdk")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(fw C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

# Add executable. Default name is the project name, version 0.1

add_executable(fw
        fw.c
        epd.c
        image_3bpp.c
        image_6bpp.c
        image_9bpp.c
        lut.c
        )

pico_set_program_name(fw "fw")
pico_set_program_version(fw "0.1")

pico_enable_stdio_uart(fw 1)
pico_enable_stdio_usb(fw 0)

# Add the standard library to the build
target_link_libraries(fw pico_stdlib)

# Add any user requested libraries
target_link_libraries(fw
        hardware_spi
        hardware_dma
        )

pico_add_extra_outputs(fw)

