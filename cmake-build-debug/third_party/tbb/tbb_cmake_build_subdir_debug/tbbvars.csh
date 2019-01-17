#!/bin/csh
setenv TBBROOT "/Users/alatyshe/servak_dima/arvi_dima/elf/elf_our_clean_version/third_party/tbb" #
setenv tbb_bin "/Users/alatyshe/servak_dima/arvi_dima/elf/elf_our_clean_version/cmake-build-debug/third_party/tbb/tbb_cmake_build_subdir_debug" #
if (! $?CPATH) then #
    setenv CPATH "${TBBROOT}/include" #
else #
    setenv CPATH "${TBBROOT}/include:$CPATH" #
endif #
if (! $?LIBRARY_PATH) then #
    setenv LIBRARY_PATH "${tbb_bin}" #
else #
    setenv LIBRARY_PATH "${tbb_bin}:$LIBRARY_PATH" #
endif #
if (! $?DYLD_LIBRARY_PATH) then #
    setenv DYLD_LIBRARY_PATH "${tbb_bin}" #
else #
    setenv DYLD_LIBRARY_PATH "${tbb_bin}:$DYLD_LIBRARY_PATH" #
endif #
 #
