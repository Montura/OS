cmake_minimum_required(VERSION 3.15)
project(OS)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")

#set(CMAKE_C_COMPILER "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/cc" CACHE PATH "" FORCE)
#set(CMAKE_CXX_COMPILER "/Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin/clang++" CACHE PATH "" FORCE)

#set(CMAKE_OSX_SYSROOT "/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")

add_executable(OS
    map_logicAddr_to_phAddr_x86.cpp
)