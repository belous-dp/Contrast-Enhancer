cmake_minimum_required(VERSION 3.21)
project(ContrastEnhancer)

set(CMAKE_CXX_STANDARD 17)

add_executable(hw5 main.cpp Image.h Image.cpp)

find_package(OpenMP REQUIRED)
#target_link_libraries(hw5 PRIVATE OpenMP::OpenMP_CXX)

#message(STATUS ${CMAKE_MODULE_PATH})
#get_property(dirs DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY INCLUDE_DIRECTORIES)
#message(STATUS ${dirs})
#foreach(dir ${dirs})
#    message(STATUS "dir='${dir}'")
#endforeach()
#message(STATUS ${CMAKE_CURRENT_LIST_DIR})
#message(STATUS ${CMAKE_ROOT})

#
#set(PYTHON_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/venv")
#set(Python3_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/venv")
##message("cur dir is: ${CMAKE_CURRENT_LIST_DIR}")
## find python libraries
#find_package(Python3 COMPONENTS Interpreter Development NumPy REQUIRED)
#find_package(PythonLibs 3.0 REQUIRED)
#include_directories(${PYTHON3_INCLUDE_DIRS} ${NumPy_INCLUDE_DIRS})
#
#if (Python3_FOUND)
#    message("Python include directory: " ${Python3_INCLUDE_DIRS})
#    message("Python version is: " ${Python3_VERSION})
##    include_directories(${Python3_INCLUDE_DIRS})
##    target_link_libraries(main ${Python3_LIBRARIES})
#endif (Python3_FOUND)
#
## populate matplotlib repository
#include(FetchContent)
#FetchContent_Declare(
#        matplotlib
#        GIT_REPOSITORY https://github.com/lava/matplotlib-cpp.git
#        GIT_TAG        f23347fca25219d1c42cbb91608b5556814bf572
#)
#FetchContent_GetProperties(matplotlib)
#FetchContent_MakeAvailable(matplotlib)
#include_directories(SYSTEM ${matplotlib_SOURCE_DIR})
#
## link python and numpy
#target_link_libraries(hw5 PRIVATE ${PYTHON_LIBRARIES} Python3::NumPy)

set(CMAKE_CXX_FLAGS -Wal)
set(CMAKE_CXX_FLAGS -Wextra)
set(CMAKE_CXX_FLAGS -pedantic)
