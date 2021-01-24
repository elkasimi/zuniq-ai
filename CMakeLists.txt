cmake_minimum_required (VERSION 3.15)
project (zuniq)
set(CMAKE_CXX_FLAGS "-Wall -pipe -O2 -march=native -g --std=c++17")

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

add_executable(player ${PLAYER_SOURCES})
