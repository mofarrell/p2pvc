cmake_minimum_required(VERSION 3.0)
project(p2pvcv2)

SET(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake" ${CMAKE_MODULE_PATH})

option(USE_OPENCV "Use OpenCV.  Required for camera support" ON)
option(USE_CURSES "Use ncurses.  Required for video rendering" ON)
option(USE_PORTAUDIO "Use portaudio.  Required for audio input/output" ON)

file(GLOB sources src/*.cpp)

set(CMAKE_CXX_STANDARD 14)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

set(CMAKE_CXX_FLAGS_RELEASE "-O3")

add_executable(p2pvcv2 ${sources})

if (USE_OPENCV)
    find_package(OpenCV REQUIRED)
    target_include_directories(p2pvcv2 PRIVATE ${OpenCV_INCLUDE_DIR})
    target_link_libraries(p2pvcv2 ${OpenCV_LIBS})
    target_compile_definitions(p2pvcv2 PRIVATE USE_OPENCV=1)
endif()
if (USE_CURSES)
    find_package(curses REQUIRED)
    target_include_directories(p2pvcv2 PRIVATE ${CURSES_INCLUDE_DIR})
    target_link_libraries(p2pvcv2 ${CURSES_LIBRARIES})
    target_compile_definitions(p2pvcv2 PRIVATE USE_CURSES=1)
endif()
if (USE_PORTAUDIO)
    find_package(Portaudio REQUIRED)
    target_include_directories(p2pvcv2 PRIVATE ${PORTAUDIO_INCLUDE_DIR})
    target_link_libraries(p2pvcv2 ${PORTAUDIO_LIBRARIES})
    target_compile_definitions(p2pvcv2 PRIVATE USE_PORTAUDIO=1)
endif()
