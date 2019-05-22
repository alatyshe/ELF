CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(elfgames_ugolki)

# Source files
set(ELFGAMES_UGOLKI_SOURCES
    common/ClientGameSelfPlay.cc
    
    game/GameState.cc
    game/GameBoard.cc
    game/BoardFeature.cc
    game/SimpleAgent.cc
    game/GameStateExt.cc
    
    train/client_manager.cc
    train/server/ServerGameTrain.cc
    
    pybind/Pybind.cc
)

# Main Checkers library
add_library(elfgames_ugolki ${ELFGAMES_UGOLKI_SOURCES})
target_link_libraries(elfgames_ugolki PUBLIC
    cppzmq
    elf
)

# Python bindings
pybind11_add_module(_elfgames_ugolki pybind/pybind_module.cc)
target_link_libraries(_elfgames_ugolki PRIVATE
    elfgames_ugolki
    zmq
)

