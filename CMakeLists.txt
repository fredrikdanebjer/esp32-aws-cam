cmake_minimum_required(VERSION 3.16)

set(PROJECT_NAME "FSU-Eye")

project(${PROJECT_NAME})

# Add primary entry-point
add_executable(${PROJECT_NAME}
  src/main.c
)

# Tell IDF build to link against this target.
set(IDF_PROJECT_EXECUTABLE ${PROJECT_NAME})

# Add subdirectories. AFR_BOARD tells which board to target.
set(AFR_BOARD espressif.esp32_devkitc CACHE INTERNAL "")
add_subdirectory(external/freertos)
add_subdirectory(src)

# Link against modules
target_link_libraries(${PROJECT_NAME}
  PRIVATE
  fsu_eye_freertos
  AFR::mqtt
)
