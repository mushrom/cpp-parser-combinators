cmake_minimum_required(VERSION 3.13)
project(ParserLib VERSION 0.1)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(PROJECT_SOURCE
    src/autolist.cpp
    src/parser.cpp
    src/ebnfish.cpp)

include_directories(include)
add_library(ParserLib ${PROJECT_SOURCE})
add_executable(MainTest ${PROJECT_SOURCE} src/main.cpp)
