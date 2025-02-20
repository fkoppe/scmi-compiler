cmake_minimum_required(VERSION 3.22)
project(scmi-compiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(scmi main.cpp)
