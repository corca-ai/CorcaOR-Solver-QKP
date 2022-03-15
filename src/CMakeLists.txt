file(GLOB_RECURSE common_sources ${CMAKE_CURRENT_SOURCE_DIR}/Common/*.cpp)

if(BUILD_TYPE STREQUAL "CUDA")
    find_package(OpenMP REQUIRED)
    
    file(GLOB_RECURSE back_sources ${CMAKE_CURRENT_SOURCE_DIR}/Cuda/*.cu)
    set(CMAKE_CUDA_ARCHITECTURES 50)

    add_library(${target_name} STATIC ${back_sources} ${common_sources})
    set_property(TARGET ${target_name} PROPERTY CUDA_STANDARD 20)
    enable_language(CUDA)

    set_target_properties(${target_name} PROPERTIES LINKER_LANGUAGE CUDA)
    set_target_properties(${target_name} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

    target_link_libraries(${target_name} INTERFACE OpenMP::OpenMP_CXX)
else()
    find_package(OpenMP REQUIRED)

    file(GLOB_RECURSE back_sources ${CMAKE_CURRENT_SOURCE_DIR}/CPU/*.cpp)
    add_library(${target_name} STATIC ${back_sources} ${common_sources})
    set_target_properties(${target_name} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
    target_link_libraries(${target_name} INTERFACE OpenMP::OpenMP_CXX)
endif()