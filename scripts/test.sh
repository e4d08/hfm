#!/bin/sh -e

BASEDIR=$(realpath "$(dirname "$0")")
ROOTDIR=$(realpath "$BASEDIR/..")

cmake -S "$ROOTDIR" -B "$ROOTDIR/build" -DCMAKE_BUILD_TYPE=Debug -DENABLE_SANITIZERS=ON
cmake --build "$ROOTDIR/build" --config Debug
ctest --test-dir build
