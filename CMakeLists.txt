cmake_minimum_required(VERSION 3.14)
project(aeternum)

set(CMAKE_CXX_STANDARD 14)

add_executable(aeternum main.cpp atom.h crc32.h tagged.h)

set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)

find_package(Boost 1.59 REQUIRED COMPONENTS unit_test_framework date_time regex)

include_directories(${Boost_INCLUDE_DIRS})

target_link_libraries(aeternum ${Boost_LIBRARIES})