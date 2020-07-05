cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME "FSU-Eye")

project(${PROJECT_NAME})

file(GLOB SOURCES
        "src/*.c"
        "src/fe_system/src/*.c"
        "fsu/system_controller/src/*.c"
)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

set(INCLUDE_DIRS ${INCLUDE_DIRS}
        "config/aws"
        "config/wifi"
        "include/fe_system/include"
        "fsu/system_controller/include/system_controller"
        "external/freertos/demos/include"
        "external/freertos/demos/network_manager"
        "external/freertos/demos/dev_mode_key_provisioning/include"
)

include_directories(${INCLUDE_DIRS})

set(IDF_PROJECT_EXECUTABLE ${PROJECT_NAME})

set(IDF_SDKCONFIG_DEFAULTS "${CMAKE_CURRENT_LIST_DIR}/config/esp32/sdkconfig.defaults")

set(AFR_BOARD espressif.esp32_devkitc CACHE INTERNAL "")

add_subdirectory("external/freertos")

target_link_libraries(${PROJECT_NAME} PRIVATE
        AFR::wifi
        AFR::utils
        AFR::common
        AFR::pkcs11
        AFR::platform
        AFR::mqtt
        )

target_compile_options(${PROJECT_NAME} PRIVATE -Wno-pointer-sign)
