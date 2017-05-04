cmake_minimum_required(VERSION 3.7)
project(shell)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES main.cpp)
add_executable(shell ${SOURCE_FILES})