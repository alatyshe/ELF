CMAKE_MINIMUM_REQUIRED(VERSION 3.3)
project(elfgames_checkers)

# Source files
set(ELFGAMES_CHECKERS_SOURCES
    common/ClientGameSelfPlay.cc
    
    checkers/CheckersState.cc
    checkers/CheckersBoard.cc
    checkers/CheckersFeature.cc
    checkers/CheckersStateExt.cc
    
    train/client_manager.cc
    train/server/ServerGameTrain.cc
    
    pybind/Pybind.cc
)

# Main Checkers library
add_library(elfgames_checkers ${ELFGAMES_CHECKERS_SOURCES})
target_link_libraries(elfgames_checkers PUBLIC
    cppzmq
    elf
)

# Python bindings
pybind11_add_module(_elfgames_checkers pybind/pybind_module.cc)
target_link_libraries(_elfgames_checkers PRIVATE
    elfgames_checkers
    zmq
)

