cmake_minimum_required(VERSION 3.15)
project(OS)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "..")

include_directories(OS include)

add_executable(OS src/main.cpp
    src/threads/priority_boost_win.cpp
    src/threads/rmw_register.cpp
    src/threads/robin_round.cpp
    src/threads/rwm_locks.cpp
    src/threads/thread_synchronization.cpp
    src/threads/read_write_lock.cpp
    src/mapping_vAddr_to_phAddr_x86.cpp)

if (UNIX)
  add_executable(OS PRIVATE src/read_elf.cpp)
endif ()