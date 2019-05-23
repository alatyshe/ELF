CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(elfgames_russian_checkers)

# Source files
set(ELFGAMES_RUSSIAN_CHECKERS_SOURCES
    game/CheckersState.cc
    game/CheckersBoard.cc
    game/CheckersFeature.cc
    game/CheckersStateExt.cc

    common/ClientGameSelfPlay.cc
    train/server/ServerGameTrain.cc
    train/client_manager.cc
    
    pybind/Pybind.cc
)

# Main Checkers library
add_library(elfgames_russian_checkers ${ELFGAMES_RUSSIAN_CHECKERS_SOURCES})
target_link_libraries(elfgames_russian_checkers PUBLIC
    cppzmq
    elf
)

# Python bindings
pybind11_add_module(_elfgames_russian_checkers pybind/pybind_module.cc)
target_link_libraries(_elfgames_russian_checkers PRIVATE
    elfgames_russian_checkers
    zmq
)

