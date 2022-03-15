set(target_name "CorcaOR-Solver-MDMQKP")

project(${target_name})

cmake_minimum_required(VERSION 3.18)

set (CMAKE_CXX_STANDARD 20)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(CMake/DetermineBuildType.cmake)

include_directories(include)

add_subdirectory(src)

add_subdirectory(test)


