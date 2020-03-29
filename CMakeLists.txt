cmake_minimum_required(VERSION 3.0)
project(p2pvcv2)

option(USE_OPENCV "Use OpenCV.  Required for camera support" ON)
option(USE_CURSES "Use ncurses.  Required for video rendering" ON)

file(GLOB sources src/*.cpp)

set(CMAKE_CXX_STANDARD 14)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(p2pvcv2 ${sources})

if (USE_OPENCV)
    find_package(OpenCV REQUIRED)
    target_include_directories(p2pvcv2 PUBLIC ${OpenCV_INCLUDE_DIR})
    target_link_libraries(p2pvcv2 ${OpenCV_LIBS})
    target_compile_definitions(p2pvcv2 PUBLIC USE_OPENCV=1)
endif()
if (USE_CURSES)
    find_package(curses REQUIRED)
    target_include_directories(p2pvcv2 PUBLIC ${CURSES_INCLUDE_DIR})
    target_link_libraries(p2pvcv2 ${CURSES_LIBRARIES})
    target_compile_definitions(p2pvcv2 PUBLIC USE_CURSES=1)
endif()
