cmake_minimum_required (VERSION 3.15)
project (zuniq)
set(CMAKE_CXX_FLAGS "-Wall -pipe -O2 -march=native -g --std=c++17")
include_directories("/usr/local/include")
link_directories("/usr/local/lib")
link_libraries(fann)

set(PLAYER_SOURCES
    robin_hood.h
    Common.h
    McRaveAgent.h
    McRaveAgent.cc
    Opening.cc
    Position.h
    Position.cc
    RNG.h
    main.cc)

set(COACHING_SOURCES
    NNAgent.h
    NNAgent.cc
    Position.h
    Position.cc
    Coaching.cc)

add_executable(player ${PLAYER_SOURCES})
add_executable(coaching ${COACHING_SOURCES})