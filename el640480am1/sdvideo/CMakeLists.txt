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

project(sdvideo C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(sdvideo)

pico_generate_pio_header(sdvideo ${CMAKE_CURRENT_LIST_DIR}/eldata.pio)

target_sources(sdvideo PRIVATE
        main.c
        el.c
        )

add_compile_definitions(sdvideo
        PICO_SD_CLK_PIN=5
        PICO_SD_CMD_PIN=18
        PICO_SD_DAT0_PIN=19
        )

pico_set_program_name(sdvideo "sdvideo")
pico_set_program_version(sdvideo "0.1")

pico_enable_stdio_uart(sdvideo 0)
pico_enable_stdio_usb(sdvideo 0)

target_include_directories(sdvideo PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Add the standard library to the build
target_link_libraries(sdvideo pico_stdlib pico_sd_card hardware_dma)

# Add any user requested libraries
target_link_libraries(sdvideo
        hardware_pio
        )

pico_add_extra_outputs(sdvideo)

