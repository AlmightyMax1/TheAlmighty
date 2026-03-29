cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(omni_fw VERSION 1 0 0)

target_sources(app PRIVATE
    src/main.c
    src/omni_service.c
    src/mux.c
    src/imu.c
    src/pressure.c
    src/heating.c
    src/spring.c
    src/harvest.c
    src/battery.c
    src/stf.c
    src/pcm.c
    src/emg.c
    src/biometric.c
    src/haptic.c
    src/gps.c
    src/mic.c
)

target_include_directories(app PRIVATE src)
