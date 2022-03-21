set(target_name "CorcaOR_Solver_QKP")
set(cpp_target_name "CPP_${target_name}")

project(${target_name})

set(CMAKE_PROJECT_VERSION "0.0.1")

cmake_minimum_required(VERSION 3.18)

set (CMAKE_CXX_STANDARD 20)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/lib)

set(ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

include(CMake/DetermineBuildType.cmake)

add_subdirectory(extern/pybind11)

include_directories(include)

add_subdirectory(src)

add_subdirectory(test)


