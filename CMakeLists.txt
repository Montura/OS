cmake_minimum_required(VERSION 3.15)
project(OS)

set(CMAKE_CXX_STANDARD 14)
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "..")

add_executable(OS main.cpp
    threads/priority_boost_win.cpp
    threads/rmw_register.cpp
    threads/robin_round.cpp
    threads/rwm_locks.cpp
    threads/thread_synchronization.cpp
    threads/read_write_lock.cpp
    mapping_vAddr_to_phAddr_x86.cpp
    read_elf.cpp
)