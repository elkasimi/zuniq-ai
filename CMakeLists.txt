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

set(COACHING_SOURCES
    NNAgent.h
    NNAgent.cc
    Position.h
    Position.cc
    Coaching.cc)

set(ZEROPLAYER_SOURCES
    NNAgent.h
    NNAgent.cc
    Position.h
    Position.cc
    ZeroPlayer.cc)

add_executable(player ${PLAYER_SOURCES})

add_executable(coaching ${COACHING_SOURCES})
target_include_directories(coaching PRIVATE "/usr/local/include")
target_link_directories(coaching PRIVATE "/usr/local/lib")
target_link_libraries(coaching fann)

add_executable(zeroplayer ${ZEROPLAYER_SOURCES})
target_include_directories(zeroplayer PRIVATE "/usr/local/include")
target_link_directories(zeroplayer PRIVATE "/usr/local/lib")
target_link_libraries(zeroplayer fann)