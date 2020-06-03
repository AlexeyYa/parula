#!/bin/bash

set -euo pipefail

mkdir -p build_coverty && cd build_coverty

wget https://github.com/schuhschuh/cmake-basis-modules/blob/develop/FindTBB.cmake

cmake .. -DTBB_ROOT=/usr -DCMAKE_PREFIX_PATH=.

cmake --build .

cd ..
