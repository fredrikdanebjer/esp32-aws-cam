cmake_minimum_required(VERSION 3.13)

set(PROJECT_NAME "FSU-Eye")

project(${PROJECT_NAME})

add_definitions(-DAPP_VERSION_MAJOR=${_VERSION_MAJOR})
add_definitions(-DAPP_VERSION_MINOR=${_VERSION_MINOR})
add_definitions(-DAPP_VERSION_BUILD=${_VERSION_BUILD})

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
        "config/http_server"
        "config/kvs"
        "config/eye"
        "include/3rd_party/aws"
        "include/fe_system"
        "include/services"
)

include_directories(${INCLUDE_DIRS})

set(IDF_PROJECT_EXECUTABLE ${PROJECT_NAME})

set(AFR_BOARD espressif.esp32_devkitc CACHE INTERNAL "")

get_filename_component(
    EXTRA_COMPONENT_DIRS
    "external/esp32-camera" ABSOLUTE
)
list(APPEND IDF_EXTRA_COMPONENT_DIRS ${EXTRA_COMPONENT_DIRS})

get_filename_component(
    EXTRA_COMPONENT_DIRS
    "external/freertos/vendors/espressif/esp-idf/components/nghttp" ABSOLUTE
)
list(APPEND IDF_EXTRA_COMPONENT_DIRS ${EXTRA_COMPONENT_DIRS})

get_filename_component(
    EXTRA_COMPONENT_DIRS
    "external/freertos/vendors/espressif/esp-idf/components/esp_http_server" ABSOLUTE
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
        AFR::ota
        3rdparty::jsmn
        )

target_compile_options(${PROJECT_NAME} PRIVATE -Wno-pointer-sign)
