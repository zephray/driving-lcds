# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# initalize pico_sdk from installed location
# (note this can come from environment, CMake cache etc)
set(PICO_SDK_PATH "/home/wenting/pico/pico-sdk")
set(PICO_EXTRAS_PATH "/home/wenting/pico/pico-extras")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

# We also need PICO EXTRAS
include(pico_extras_import.cmake)

project(eldemo C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(eldemo)

pico_generate_pio_header(eldemo ${CMAKE_CURRENT_LIST_DIR}/eldata.pio)

target_sources(eldemo PRIVATE
        main.c
        el.c
        )

pico_set_program_name(eldemo "eldemo")
pico_set_program_version(eldemo "0.1")

pico_enable_stdio_uart(eldemo 0)
pico_enable_stdio_usb(eldemo 0)

target_include_directories(eldemo PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Add the standard library to the build
target_link_libraries(eldemo pico_stdlib hardware_dma)

# Add any user requested libraries
target_link_libraries(eldemo
        hardware_pio
        )

pico_add_extra_outputs(eldemo)

