#!/bin/bash
export TBBROOT="/Users/alatyshe/servak_dima/arvi_dima/elf/elf_our_clean_version/third_party/tbb" #
tbb_bin="/Users/alatyshe/servak_dima/arvi_dima/elf/elf_our_clean_version/cmake-build-debug/third_party/tbb/tbb_cmake_build_subdir_debug" #
if [ -z "$CPATH" ]; then #
    export CPATH="${TBBROOT}/include" #
else #
    export CPATH="${TBBROOT}/include:$CPATH" #
fi #
if [ -z "$LIBRARY_PATH" ]; then #
    export LIBRARY_PATH="${tbb_bin}" #
else #
    export LIBRARY_PATH="${tbb_bin}:$LIBRARY_PATH" #
fi #
if [ -z "$DYLD_LIBRARY_PATH" ]; then #
    export DYLD_LIBRARY_PATH="${tbb_bin}" #
else #
    export DYLD_LIBRARY_PATH="${tbb_bin}:$DYLD_LIBRARY_PATH" #
fi #
 #
