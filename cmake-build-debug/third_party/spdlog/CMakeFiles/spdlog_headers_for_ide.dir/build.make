# CMAKE generated file: DO NOT EDIT!
# Generated by "Unix Makefiles" Generator, CMake Version 3.10

# Delete rule output on recipe failure.
.DELETE_ON_ERROR:


#=============================================================================
# Special targets provided by cmake.

# Disable implicit rules so canonical targets will work.
.SUFFIXES:


# Remove some rules from gmake that .SUFFIXES does not remove.
SUFFIXES =

.SUFFIXES: .hpux_make_needs_suffix_list


# Suppress display of executed commands.
$(VERBOSE).SILENT:


# A target that is always out of date.
cmake_force:

.PHONY : cmake_force

#=============================================================================
# Set environment variables for the build.

# The shell in which to execute make rules.
SHELL = /bin/sh

# The CMake executable.
CMAKE_COMMAND = /Applications/CLion.app/Contents/bin/cmake/bin/cmake

# The command to remove a file.
RM = /Applications/CLion.app/Contents/bin/cmake/bin/cmake -E remove -f

# Escaping for special characters.
EQUALS = =

# The top-level source directory on which CMake was run.
CMAKE_SOURCE_DIR = /Users/alatyshe/servak_dima/arvi_dima/elf/elf_our_clean_version

# The top-level build directory on which CMake was run.
CMAKE_BINARY_DIR = /Users/alatyshe/servak_dima/arvi_dima/elf/elf_our_clean_version/cmake-build-debug

# Utility rule file for spdlog_headers_for_ide.

# Include the progress variables for this target.
include third_party/spdlog/CMakeFiles/spdlog_headers_for_ide.dir/progress.make

spdlog_headers_for_ide: third_party/spdlog/CMakeFiles/spdlog_headers_for_ide.dir/build.make

.PHONY : spdlog_headers_for_ide

# Rule to build all files generated by this target.
third_party/spdlog/CMakeFiles/spdlog_headers_for_ide.dir/build: spdlog_headers_for_ide

.PHONY : third_party/spdlog/CMakeFiles/spdlog_headers_for_ide.dir/build

third_party/spdlog/CMakeFiles/spdlog_headers_for_ide.dir/clean:
	cd /Users/alatyshe/servak_dima/arvi_dima/elf/elf_our_clean_version/cmake-build-debug/third_party/spdlog && $(CMAKE_COMMAND) -P CMakeFiles/spdlog_headers_for_ide.dir/cmake_clean.cmake
.PHONY : third_party/spdlog/CMakeFiles/spdlog_headers_for_ide.dir/clean

third_party/spdlog/CMakeFiles/spdlog_headers_for_ide.dir/depend:
	cd /Users/alatyshe/servak_dima/arvi_dima/elf/elf_our_clean_version/cmake-build-debug && $(CMAKE_COMMAND) -E cmake_depends "Unix Makefiles" /Users/alatyshe/servak_dima/arvi_dima/elf/elf_our_clean_version /Users/alatyshe/servak_dima/arvi_dima/elf/elf_our_clean_version/third_party/spdlog /Users/alatyshe/servak_dima/arvi_dima/elf/elf_our_clean_version/cmake-build-debug /Users/alatyshe/servak_dima/arvi_dima/elf/elf_our_clean_version/cmake-build-debug/third_party/spdlog /Users/alatyshe/servak_dima/arvi_dima/elf/elf_our_clean_version/cmake-build-debug/third_party/spdlog/CMakeFiles/spdlog_headers_for_ide.dir/DependInfo.cmake --color=$(COLOR)
.PHONY : third_party/spdlog/CMakeFiles/spdlog_headers_for_ide.dir/depend

