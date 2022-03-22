include_directories(${ROOT_DIR}/extern/doctest/doctest)

FILE(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(BenchmarkTest ${sources})

target_link_libraries(BenchmarkTest PRIVATE ${cpp_target_name})