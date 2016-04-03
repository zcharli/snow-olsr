cmake_minimum_required(VERSION 3.5)
project(snow_olsr)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")
set(CXX_STANDARD 14)
include_directories("usr/include/boost")
include_directories("usr/include/c++/")

add_custom_target(snow_olsr COMMA ND make -C ${PROJECT_SOURCE_DIR}
        CLION_EXE_DIR=${PROJECT_BINARY_DIR})

add_definitions(-std=c++14)
