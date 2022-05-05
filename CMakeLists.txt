cmake_minimum_required(VERSION 3.22)

project(Yall VERSION 1.00 LANGUAGES CXX)

include_directories(includes)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/includes)

option(TEST "Build TEST" OFF)

if (TEST)
    add_executable(test test/YallTest.cpp)
endif ()