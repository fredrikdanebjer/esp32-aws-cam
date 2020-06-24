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
elif [[ $# -eq 1 ]] && [[ $1 = "flash" ]]; then
  cmake --build build --target flash
elif [[ $# -eq 1 ]] && [[ $1 = "monitor" ]]; then
  echo "Trying to monitor with device ${MONITOR_DEVICE}. If your connection is using another device, please change in script"
  ./external/freertos/vendors/espressif/esp-idf/tools/idf.py monitor -p ${MONITOR_DEVICE} -B build
else
  echo "Unknown command! Usage:"
  echo "./build.sh          - compile"
  echo "./build.sh flash    - flash"
  echo "./build.sh monitor  - monitor output"
fi
