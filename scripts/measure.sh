#!/bin/sh -e

export BASEDIR=$(realpath "$(dirname "$0")")
export ROOTDIR=$(realpath "$BASEDIR/..")
export DATADIR=$(realpath "$ROOTDIR/test/data")
export WORKDIR=$(realpath "$ROOTDIR/build/test")

function hfm {
    $ROOTDIR/build/src/hfm $@
}

function hfm_compress {
    hfm $1 -o "$1.hfm"
}

function hfm_decompress {
    hfm -d "$1.hfm" -o "$1".decompressed
}

export -f hfm
export -f hfm_compress
export -f hfm_decompress

cd $WORKDIR
cp "$DATADIR/$1" "$WORKDIR/$(filename $1)"

hyperfine --warmup 5 "hfm_compress $WORKDIR/$1" "gzip -fk4 $WORKDIR/$1"
