# =============================================================== #
#                             BASIC SETUP                         #
# =============================================================== #
# Sets the minimum cmake version needed to build this project.
cmake_minimum_required(VERSION 3.20.0)

# This is a crucial line. It finds the main Zephyr RTOS installation (using the ZEPHYR_BASE environment variable).
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})

# Names the project "cyborg-app"
project(esp32s3-wifi-app)



# =============================================================== #
#                PROJECT SPECIFIC SETTING                         #
# =============================================================== #

# This line tells the compiler where to find your custom header files. i.e., inside the "lib" folder inside "src"
target_include_directories(app PRIVATE 
                                lib/led
                                lib/wifi
                                lib/udp
                                lib/tcp)

# This line tells the build system to link the C++ standard library.
target_link_libraries(app PUBLIC stdc++)

# Find all the source files relating led control and add them into led_sources
FILE(GLOB led_sources
        lib/led/*.cpp)

# Find all the source files relating wifi and add them into wifi_sources
FILE(GLOB wifi_sources
        lib/wifi/*.cpp)

# Find all the source files relating udp and add them into udp_sources
FILE(GLOB udp_sources
        lib/udp/*.cpp)

# Find all the source files relating tcp and add them into tcp_sources
FILE(GLOB tcp_sources
        lib/tcp/*.cpp)

# Take all these source files and compile them into my app target.
target_sources(app PRIVATE 
    ${led_sources}
    ${wifi_sources}
    ${udp_sources}
    ${tcp_sources}
    src/main.cpp)
