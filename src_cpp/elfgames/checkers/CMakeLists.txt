CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(elfgames_checkers)

# Source files

set(ELFGAMES_CHECKERS_SOURCES
    base/board_feature.cc
    base/common.cc
    base/go_state.cc
    base/board.cc
    sgf/sgf.cc
    common/game_selfplay.cc
    common/go_state_ext.cc
    train/client_manager.cc
    train/game_train.cc
    train/Pybind.cc
)

set(ELFGAMES_checkers_INFERENCE_SOURCES
    base/board_feature.cc
    base/common.cc
    base/go_state.cc
    base/board.cc
    sgf/sgf.cc
    common/game_selfplay.cc
    common/go_state_ext.cc
    inference/Pybind.cc
)

# Main Go library

add_library(elfgames_checkers ${ELFGAMES_CHECKERS_SOURCES})
if(${BOARD9x9})
    message("Use 9x9 board")
    target_compile_definitions(elfgames_checkers PUBLIC BOARD9x9)
endif()
target_link_libraries(elfgames_checkers PUBLIC
    cppzmq
    elf
)

add_library(elfgames_checkers_inference ${ELFGAMES_checkers_INFERENCE_SOURCES})
if(${BOARD9x9})
    message("Use 9x9 board")
    target_compile_definitions(elfgames_checkers_inference PUBLIC BOARD9x9)
endif()
target_link_libraries(elfgames_checkers_inference PUBLIC
    elf
)

# For unit-test purpose, build 9x9 library
add_library(elfgames_checkers9 ${ELFGAMES_CHECKERS_SOURCES})
target_compile_definitions(elfgames_checkers9 PUBLIC BOARD9x9)
target_link_libraries(elfgames_checkers9 PUBLIC
    cppzmq
    elf
)

# Python bindings

pybind11_add_module(_elfgames_checkers train/pybind_module.cc)
target_link_libraries(_elfgames_checkers PRIVATE
    elfgames_checkers
    zmq
)


pybind11_add_module(_elfgames_checkers_inference inference/pybind_module.cc)
target_link_libraries(_elfgames_checkers_inference PRIVATE
    elfgames_checkers_inference
)

#set_target_properties(_elfgames_checkers PROPERTIES
#    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

# unit-test here:
# set(CHECKERS_TEST_SOURCES
#     base/test/coord_test.cc
#     base/test/go_test.cc
#     base/test/board_feature_test.cc
#     base/test/symmetry_test.cc
#     sgf/sgf_test.cc
#     #mcts/mcts_test.cc
# )
# enable_testing()
# add_cpp_tests(test_cpp_elfgames_checkers_ elfgames_checkers9 ${CHECKERS_TEST_SOURCES})
