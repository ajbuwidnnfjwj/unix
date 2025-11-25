cmake_minimum_required(VERSION 4.0)
project(unix CXX)

set(CMAKE_CXX_STANDARD 11)

add_executable(unix main.cpp
        include/states/state.h
        include/utils.h
        include/states/upload_state.h
        include/states/download_state.h
        include/states/idle_state.h
)

