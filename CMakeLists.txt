cmake_minimum_required(VERSION 4.0)
project(unix CXX)

set(CMAKE_CXX_STANDARD 11)

add_executable(unix main.cpp
        include/state.h
        include/utils.h
)

