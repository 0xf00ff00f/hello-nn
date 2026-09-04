cmake_minimum_required(VERSION 3.10)

project(hello-nn)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(hello-nn main.cc matrix.h matrix_test.h)
