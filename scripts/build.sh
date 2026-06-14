#!/bin/sh -e

BASEDIR=$(realpath "$(dirname "$0")")
ROOTDIR=$(realpath "$BASEDIR/..")

cmake -S "$ROOTDIR" -B "$ROOTDIR/build" -DCMAKE_BUILD_TYPE=Release
cmake --build "$ROOTDIR/build" --config Release
