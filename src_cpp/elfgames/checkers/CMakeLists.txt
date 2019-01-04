CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(elfgames_go)

# Source files

set(ELFGAMES_GO_SOURCES
    common/GameSelfPlay.cc
    train/client_manager.cc
    train/game_train.cc
    train/Pybind.cc

    checkers/CheckersState.cc
    checkers/CheckersBoard.cc
    checkers/CheckersFeature.cc
    checkers/CheckersStateExt.cc
)

# set(ELFGAMES_GO_INFERENCE_SOURCES
#     common/GameSelfPlay.cc
#     inference/Pybind.cc
    
#     checkers/CheckersState.cc
#     checkers/CheckersBoard.cc
#     checkers/CheckersFeature.cc
#     checkers/CheckersStateExt.cc
# )

# Main Go library

add_library(elfgames_go ${ELFGAMES_GO_SOURCES})
if(${BOARD9x9})
    message("Use 9x9 board")
    target_compile_definitions(elfgames_go PUBLIC BOARD9x9)
endif()
target_link_libraries(elfgames_go PUBLIC
    cppzmq
    elf
)

# add_library(elfgames_go_inference ${ELFGAMES_GO_INFERENCE_SOURCES})
# if(${BOARD9x9})
#     message("Use 9x9 board")
#     target_compile_definitions(elfgames_go_inference PUBLIC BOARD9x9)
# endif()
# target_link_libraries(elfgames_go_inference PUBLIC
#     elf
# )

# For unit-test purpose, build 9x9 library
add_library(elfgames_go9 ${ELFGAMES_GO_SOURCES})
target_compile_definitions(elfgames_go9 PUBLIC BOARD9x9)
target_link_libraries(elfgames_go9 PUBLIC
    cppzmq
    elf
)

# Python bindings

pybind11_add_module(_elfgames_go train/pybind_module.cc)
target_link_libraries(_elfgames_go PRIVATE
    elfgames_go
    zmq
)


# pybind11_add_module(_elfgames_go_inference inference/pybind_module.cc)
# target_link_libraries(_elfgames_go_inference PRIVATE
#     elfgames_go_inference
# )
