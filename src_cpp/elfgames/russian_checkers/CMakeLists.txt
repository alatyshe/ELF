CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(elfgames_russian_checkers)

# Source files
set(ELFGAMES_RUSSIAN_CHECKERS_SOURCES
    base/CheckersState.cc
    base/CheckersBoard.cc
    base/CheckersFeature.cc
    base/CheckersStateExt.cc

    common/ClientGameSelfPlay.cc
    train/server/ServerGameTrain.cc
    train/client_manager.cc
    
    train/Pybind.cc
)

# Main Checkers library
add_library(elfgames_russian_checkers ${ELFGAMES_RUSSIAN_CHECKERS_SOURCES})
target_link_libraries(elfgames_russian_checkers PUBLIC
    cppzmq
    elf
)

# Python bindings
pybind11_add_module(_elfgames_russian_checkers train/pybind_module.cc)
target_link_libraries(_elfgames_russian_checkers PRIVATE
    elfgames_russian_checkers
    zmq
)

