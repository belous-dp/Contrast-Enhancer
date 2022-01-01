cmake_minimum_required(VERSION 3.21)
project(ContrastEnhancer)

set(CMAKE_CXX_STANDARD 17)

add_executable(ContrastEnhancer main.cpp Image.h Image.cpp Time.cpp Time.h)

find_package(OpenMP REQUIRED)
target_link_libraries(ContrastEnhancer PRIVATE OpenMP::OpenMP_CXX)

set(CMAKE_CXX_FLAGS -O0)

set(CMAKE_CXX_FLAGS -Wall)
set(CMAKE_CXX_FLAGS -Wextra)
set(CMAKE_CXX_FLAGS -pedantic)
