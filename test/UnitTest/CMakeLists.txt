include_directories(${ROOT_DIR}/extern/doctest/doctest)

FILE(GLOB_RECURSE sources ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(UnitTest ${sources})

target_link_libraries(UnitTest PRIVATE ${target_name})