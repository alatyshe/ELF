CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(elfgames_american_checkers)

# Source files
set(ELFGAMES_AMERICAN_CHECKERS_SOURCES
    common/ClientGameSelfPlay.cc
    
    game/GameState.cc
    game/GameBoard.cc
    game/BoardFeature.cc
    game/GameStateExt.cc
    
    train/client_manager.cc
    train/server/ServerGameTrain.cc
    
    pybind/Pybind.cc
)

# Main Checkers library
add_library(elfgames_american_checkers ${ELFGAMES_AMERICAN_CHECKERS_SOURCES})
target_link_libraries(elfgames_american_checkers PUBLIC
    cppzmq
    elf
)

# Python bindings
pybind11_add_module(_elfgames_american_checkers pybind/pybind_module.cc)
target_link_libraries(_elfgames_american_checkers PRIVATE
    elfgames_american_checkers
    zmq
)

