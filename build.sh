#!/bin/bash

set -euo pipefail

doxygen ./Doxyfile

mkdir -p build && cd build

wget https://github.com/schuhschuh/cmake-basis-modules/blob/develop/FindTBB.cmake

cmake .. -DTBB_ROOT=/usr -DCMAKE_PREFIX_PATH=.

cmake --build .

ctest -j $(nproc)

cpack -G DEB
