cmake_minimum_required(VERSION 3.0)
project(p2pvcv2)

find_package(OpenCV REQUIRED)
find_package(curses REQUIRED)

file(GLOB sources src/*.cpp)

set(CMAKE_CXX_STANDARD 14)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

include_directories(${OpenCV_INCLUDE_DIR})
include_directories(${CURSES_INCLUDE_DIR})

add_executable(p2pvcv2 ${sources})
target_link_libraries(p2pvcv2 ${OpenCV_LIBS})
target_link_libraries(p2pvcv2 ${CURSES_LIBRARIES})
