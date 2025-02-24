cmake_minimum_required(VERSION 3.22)
project(scmi-compiler)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(scmi source/main.cpp
    source/tokens.cpp
    source/ast.h
        source/compiler.hpp
        source/compiler.cpp
        source/semantic_analyzer.hpp
        source/semantic_analyzer.cpp)

add_subdirectory(source)
