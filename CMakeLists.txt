cmake_minimum_required(VERSION 3.15)
project(OS)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "..")

add_executable(OS main.cpp
    priority_boost_win.cpp
    robin_round.cpp
    thread_synchronization.cpp)