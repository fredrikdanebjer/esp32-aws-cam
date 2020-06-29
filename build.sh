#!/bin/bash

MONITOR_DEVICE=/dev/ttyUSB0

BUILD_DIR="$(pwd)/build"
COMPILER_DIR="$(pwd)/external/esp32-toolchain/xtensa-esp32-elf/bin/"

export PATH="$COMPILER_DIR:${PATH}"

which xtensa-esp32-elf-gcc && xtensa-esp32-elf-gcc --version
which ninja-build && ninja-build --version
which cmake && cmake --version

if [ $# -eq 0 ]; then
  rm -rf build
  mkdir build
  cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=external/freertos/tools/cmake/toolchains/xtensa-esp32.cmake -G Ninja
  cmake --build build
elif [[ $# -eq 2 ]] && [[ $1 = "debug" ]] && [[ "${2}" =~ ^[0-9]+$ ]]; then
  echo Setting DEBUG_PRINT_LEVEL to $2
  rm -rf build
  mkdir build
  cmake -S . -B build -D_DEBUG_PRINT_LEVEL=$2 -DIDF_SDK_CONFIG_DEFAULTS=config/esp32/sdkconfig.defaults -DCMAKE_TOOLCHAIN_FILE=external/freertos/tools/cmake/toolchains/xtensa-esp32.cmake -G Ninja
  cmake --build build
elif [[ $# -eq 1 ]] && [[ $1 = "flash" ]]; then
  cmake --build build --target flash
elif [[ $# -eq 1 ]] && [[ $1 = "monitor" ]]; then
  echo "Trying to monitor with device ${MONITOR_DEVICE}. If your connection is using another device, please change in script"
  ./external/freertos/vendors/espressif/esp-idf/tools/idf.py monitor -p ${MONITOR_DEVICE} -B build
elif [[ $# -eq 1 ]] && [[ $1 = "all" ]]; then
  rm -rf build
  mkdir build
  cmake -S . -B build -D_DEBUG_PRINT_LEVEL=9 -DIDF_SDK_CONFIG_DEFAULTS=config/esp32/sdkconfig.defaults -DCMAKE_TOOLCHAIN_FILE=external/freertos/tools/cmake/toolchains/xtensa-esp32.cmake -G Ninja
  cmake --build build --target flash
  ./external/freertos/vendors/espressif/esp-idf/tools/idf.py monitor -p ${MONITOR_DEVICE} -B build
else
  echo "Unknown command! Usage:"
  echo "./build.sh                - compile"
  echo "./build.sh debug <level>  - compile with debug logs enabled at provided level"
  echo "./build.sh flash          - flash"
  echo "./build.sh monitor        - monitor output"
  echo "./build.sh all            - builds with max highest debug resolution, flashes and starts monitoring"
fi
