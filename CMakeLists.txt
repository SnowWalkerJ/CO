cmake_minimum_required(VERSION 3.17)
project(co)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g -ggdb")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")
add_library(co CO.cpp CO.h)
add_executable(TestCo TestCo.cc co)