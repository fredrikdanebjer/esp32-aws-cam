cmake_minimum_required(VERSION 3.16)

project(FSU-Eye)

add_executable(app src/main.c)

# Tell IDF build to link against this target.
set(IDF_PROJECT_EXECUTABLE app)

# Add FreeRTOS as a subdirectory. AFR_BOARD tells which board to target.
set(AFR_BOARD espressif.esp32_devkitc CACHE INTERNAL "")
add_subdirectory(external/freertos)

# Link against the mqtt library so that we can use it. Dependencies are transitively
# linked.
target_link_libraries(app PRIVATE AFR::mqtt)

