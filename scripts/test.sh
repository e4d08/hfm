#!/bin/sh -e

BASEDIR=$(realpath "$(dirname "$0")")
ROOTDIR=$(realpath "$BASEDIR/..")
BUILDDIR="$ROOTDIR/build"

echo "Run integration tests"
"$BUILDDIR"/test/test_compress_decompress
