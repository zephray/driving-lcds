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

project(lcddemo C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

add_executable(lcddemo)

pico_generate_pio_header(lcddemo ${CMAKE_CURRENT_LIST_DIR}/lcddata.pio)

target_sources(lcddemo PRIVATE
        core1.c
        main.c
        lcd.c
        )

add_compile_definitions(lcddemo
        PICO_SD_CLK_PIN=26
        PICO_SD_CMD_PIN=18
        PICO_SD_DAT0_PIN=19
        PICO_SD_CARD_EXTRA_CLOCK_DIVIDER=2
        )

pico_set_program_name(lcddemo "lcddemo")
pico_set_program_version(lcddemo "0.1")

pico_enable_stdio_uart(lcddemo 0)
pico_enable_stdio_usb(lcddemo 0)

target_include_directories(lcddemo PRIVATE ${CMAKE_CURRENT_LIST_DIR})

# Add the standard library to the build
target_link_libraries(lcddemo pico_stdlib hardware_dma hardware_pwm)

# Add any user requested libraries
target_link_libraries(lcddemo
        pico_stdlib
        pico_multicore
        pico_sd_card
        hardware_pio
        )

pico_add_extra_outputs(lcddemo)

