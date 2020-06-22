#!/bin/bash

rm -rf build
mkdir build

BUILD_DIR="$(pwd)/build"
COMPILER_DIR="$(pwd)/external/esp32-toolchain/xtensa-esp32-elf/bin/"

export PATH="$COMPILER_DIR:${PATH}"

which xtensa-esp32-elf-gcc && xtensa-esp32-elf-gcc --version
which ninja-build && ninja-build --version
which cmake && cmake --version

if [ $# -eq 0 ]; then
  cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=external/freertos/tools/cmake/toolchains/xtensa-esp32.cmake -G Ninja
  cmake --build build
elif [[ $# -eq 1 ]] && [[ $1 = "flash" ]]; then
  cmake --build build --target flash
else
  echo "Unknown command! Usage:"
  echo "./build.sh          - compile"
  echo "./build.sh flash    - flash"
fi
