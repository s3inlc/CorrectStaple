cmake_minimum_required(VERSION 3.1)
project(correct-staple)
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

#include_directories(include)

add_executable(correct-staple
        src/main.cpp
        )
