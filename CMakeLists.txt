# Generated Cmake Pico project file

cmake_minimum_required(VERSION 3.13)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Initialise pico_sdk from installed location
# (note this can come from environment, CMake cache etc)

# == DO NOT EDIT THE FOLLOWING LINES for the Raspberry Pi Pico VS Code Extension to work ==
if(WIN32)
    set(USERHOME $ENV{USERPROFILE})
else()
    set(USERHOME $ENV{HOME})
endif()
set(sdkVersion 2.2.0)
set(toolchainVersion 14_2_Rel1)
set(picotoolVersion 2.2.0)
set(picoVscode ${USERHOME}/.pico-sdk/cmake/pico-vscode.cmake)
if (EXISTS ${picoVscode})
    include(${picoVscode})
endif()
# ====================================================================================
set(PICO_BOARD pimoroni_pico_plus2_w_rp2350 CACHE STRING "Board type")
set(PICOCALC_TERMINAL_VERSION "0.1" CACHE STRING "PicoCalc Terminal version")

# Pull in Raspberry Pi Pico SDK (must be before project)
include(pico_sdk_import.cmake)

project(picocalc-terminal C CXX ASM)

# Initialise the Raspberry Pi Pico SDK
pico_sdk_init()

file(GLOB STARTER_SOURCES CONFIGURE_DEPENDS
    modules/picocalc-text-starter/drivers/*.c
)

# Add executable. Default name is the project name, version 0.1

add_executable(picocalc-terminal
    modules/picocalc-text-starter/drivers/audio.c
    modules/picocalc-text-starter/drivers/clib.c
    modules/picocalc-text-starter/drivers/display.c
    modules/picocalc-text-starter/drivers/fat32.c
    modules/picocalc-text-starter/drivers/font-5x10.c
    modules/picocalc-text-starter/drivers/font-8x10.c
    modules/picocalc-text-starter/drivers/keyboard.c
    modules/picocalc-text-starter/drivers/lcd.c
    modules/picocalc-text-starter/drivers/picocalc.c
    modules/picocalc-text-starter/drivers/southbridge.c
    main.c
    )

pico_set_program_name(picocalc-terminal "picocalc-terminal")
pico_set_program_version(picocalc-terminal "${PICOCALC_TERMINAL_VERSION}")
pico_set_program_url(picocalc-terminal "https://github.com/BlairLeduc/picocalc-terminal")

# Generate PIO header
pico_generate_pio_header(picocalc-terminal ${CMAKE_CURRENT_LIST_DIR}/modules/picocalc-text-starter/drivers/audio.pio)

# Modify the below lines to enable/disable output over UART/USB
pico_enable_stdio_uart(picocalc-terminal 0)
pico_enable_stdio_usb(picocalc-terminal 0)

# Add the standard library to the build
target_link_libraries(picocalc-terminal
    pico_stdlib)

# Add the standard include files to the build
target_include_directories(picocalc-terminal PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/modules/picocalc-text-starter
)

configure_file(
    ${CMAKE_CURRENT_SOURCE_DIR}/version.h.in
    ${CMAKE_CURRENT_BINARY_DIR}/version.h
    @ONLY
)

# Add any user requested libraries
target_link_libraries(picocalc-terminal 
    pico_stdlib
    pico_printf
    pico_multicore
    hardware_gpio
    hardware_i2c
    hardware_spi
    hardware_pio
    hardware_clocks
    )

pico_add_extra_outputs(picocalc-terminal)

