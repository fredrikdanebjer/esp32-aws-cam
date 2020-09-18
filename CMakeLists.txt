cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME "FSU-Eye")

project(${PROJECT_NAME})

file(GLOB SOURCES
        "src/*.c"
        "src/fe_system/*.c"
        "src/services/*.c"
        "src/rtos/*.c"
        "src/3rd_party/aws/*.c"
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

set(INCLUDE_DIRS ${INCLUDE_DIRS}
        "config/aws"
        "config/wifi"
        "include/3rd_party/aws"
        "include/fe_system"
        "include/services"
)

include_directories(${INCLUDE_DIRS})

set(IDF_PROJECT_EXECUTABLE ${PROJECT_NAME})

set(IDF_SDKCONFIG_DEFAULTS "${CMAKE_CURRENT_LIST_DIR}/config/esp32/sdkconfig.defaults")

set(AFR_BOARD espressif.esp32_devkitc CACHE INTERNAL "")

get_filename_component(
    EXTRA_COMPONENT_DIRS
    "external/esp32-camera" ABSOLUTE
)
list(APPEND IDF_EXTRA_COMPONENT_DIRS ${EXTRA_COMPONENT_DIRS})

add_subdirectory("external/freertos")

target_link_libraries(${PROJECT_NAME} PRIVATE
        AFR::wifi
        AFR::utils
        AFR::common
        AFR::pkcs11
        AFR::platform
        AFR::mqtt
        3rdparty::jsmn
        )

target_compile_options(${PROJECT_NAME} PRIVATE -Wno-pointer-sign)
