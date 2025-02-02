cmake_minimum_required(VERSION 3.13)

# Pull in SDK (must be before project)
include(pico_sdk_import.cmake)

project(pwm_rgb_led C CXX ASM)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

# Initialize the SDK
pico_sdk_init()

add_executable(pwm_rgb_led
    main.c
)

# Add pico_stdlib library which aggregates commonly used features
target_link_libraries(pwm_rgb_led pico_stdlib hardware_pwm)

# Enable USB output, disable UART output
pico_enable_stdio_usb(pwm_rgb_led 1)
pico_enable_stdio_uart(pwm_rgb_led 0)

# Create map/bin/hex/uf2 file in addition to ELF.
pico_add_extra_outputs(pwm_rgb_led)
