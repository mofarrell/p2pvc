cmake_minimum_required(VERSION 3.0)
project(p2pvc2)

file(GLOB sources src/*.cpp)

set(CMAKE_CXX_STANDARD 14)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(p2pvc2 ${sources})

