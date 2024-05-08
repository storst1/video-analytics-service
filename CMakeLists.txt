cmake_minimum_required(VERSION 3.10)
project(VideoAnalysisSystem)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(CMAKE_C_COMPILER "clang")
set(CMAKE_CXX_COMPILER "clang++")

add_executable(VideoAnalysisSystem main.cpp)

include_directories(your/include/path)
link_directories(your/library/path)
