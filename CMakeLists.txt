cmake_minimum_required(VERSION 3.3)

# Basic third-party dependencies

find_package(Threads)

#find_package(Boost REQUIRED COMPONENTS
#    thread
#)
#include_directories(${Boost_INCLUDE_DIR})

# Include third-party dependencies

add_subdirectory(
    third_party
    ${CMAKE_CURRENT_BINARY_DIR}/third_party)
find_package(TBB REQUIRED tbb)  # Need to find_package in parent scope

# Sanity checks

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    # require at least gcc 7.1
    if (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.1)
        message(FATAL_ERROR "GCC version must be at least 7.1!")
    endif()
endif()

# Variables from Git

execute_process(COMMAND git rev-parse HEAD
    OUTPUT_VARIABLE GIT_COMMIT_HASH)
string(STRIP ${GIT_COMMIT_HASH} GIT_COMMIT_HASH)
execute_process(COMMAND git diff-index --quiet HEAD --
    RESULT_VARIABLE GIT_UNSTAGED)
if(${GIT_UNSTAGED})
    set(GIT_STAGED_STRING unstaged)
else()
    set(GIT_STAGED_STRING staged)
endif()

# Global compiler config

set(CMAKE_CXX_STANDARD 17)
set(PYBIND11_CPP_STANDARD -std=c++17)
set(CMAKE_CXX_FLAGS
    "${CMAKE_CXX_FLAGS} -g -Wno-deprecated-declarations -fPIC -march=native")
# -g
# set(CMAKE_CXX_FLAGS
#     "${CMAKE_CXX_FLAGS} -Wall -Werror -Wextra -Wno-register -Wno-deprecated-declarations -fPIC -march=native")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Define a convenience function for tests
function(add_cpp_tests prefix lib_to_link)
    set(test_list ${ARGV})
    list(REMOVE_AT test_list 0)
    list(REMOVE_AT test_list 0)
    foreach(test_file ${test_list})
        string(REPLACE "/" "_" test_name ${test_file})
        string(REPLACE ".cc" "" test_name ${test_name})
        string(CONCAT test_name ${prefix} ${test_name})
        add_executable(${test_name} ${test_file})
        add_test(${test_name} ${test_name})
        target_link_libraries(${test_name} ${lib_to_link} gtest)
    endforeach(test_file)
endfunction(add_cpp_tests)

# Include everything in src_cpp

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src_cpp/)

# Main ELF library
add_subdirectory(
    src_cpp/elf
    ${CMAKE_CURRENT_BINARY_DIR}/elf)

# ELF games
add_subdirectory(
    src_cpp/elfgames/ugolki
    ${CMAKE_CURRENT_BINARY_DIR}/elfgames/ugolki)

add_subdirectory(
    src_cpp/elfgames/american_checkers
    ${CMAKE_CURRENT_BINARY_DIR}/elfgames/american_checkers)

add_subdirectory(
    src_cpp/elfgames/russian_checkers
    ${CMAKE_CURRENT_BINARY_DIR}/elfgames/russian_checkers)