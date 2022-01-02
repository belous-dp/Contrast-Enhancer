cmake_minimum_required(VERSION 3.21)
project(ContrastEnhancer)

set(CMAKE_CXX_STANDARD 17)

add_executable(hw5 main.cpp Image.h Image.cpp Time.cpp Time.h)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    set(OpenMP_CXX_FLAGS "-fopenmp=libomp")
    target_link_libraries(hw5 PRIVATE "${OpenMP_CXX_FLAGS}")
    target_compile_options(hw5 PRIVATE "${OpenMP_CXX_FLAGS}")
else()
    find_package(OpenMP REQUIRED)
    target_link_libraries(hw5 PRIVATE OpenMP::OpenMP_CXX)
endif()

set(CMAKE_CXX_FLAGS -O3)

set(CMAKE_CXX_FLAGS -Wall)
set(CMAKE_CXX_FLAGS -Wextra)
set(CMAKE_CXX_FLAGS -pedantic)
