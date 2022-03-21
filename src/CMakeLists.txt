file(GLOB_RECURSE common_sources ${CMAKE_CURRENT_SOURCE_DIR}/Common/*.cpp)

if(BUILD_TYPE STREQUAL "CUDA")
    find_package(OpenMP REQUIRED)
    
    file(GLOB_RECURSE back_sources ${CMAKE_CURRENT_SOURCE_DIR}/Cuda/*.cu)
    set(CMAKE_CUDA_ARCHITECTURES 50)

    add_library(${cpp_target_name} STATIC ${back_sources} ${common_sources})
    set_property(TARGET ${cpp_target_name} PROPERTY CUDA_STANDARD 20)
    enable_language(CUDA)

    set_target_properties(${cpp_target_name} PROPERTIES LINKER_LANGUAGE CUDA)
    set_target_properties(${cpp_target_name} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)

    target_link_libraries(${cpp_target_name} INTERFACE OpenMP::OpenMP_CXX)
else()
    find_package(OpenMP REQUIRED)

    file(GLOB_RECURSE back_sources ${CMAKE_CURRENT_SOURCE_DIR}/CPU/*.cpp)
    add_library(${cpp_target_name} STATIC ${back_sources} ${common_sources})
    set_target_properties(${cpp_target_name} PROPERTIES POSITION_INDEPENDENT_CODE TRUE)
    target_link_libraries(${cpp_target_name} INTERFACE OpenMP::OpenMP_CXX)
endif()

# Build Python Binding
if(CALL_FROM_SETUP_PY)

file(GLOB_RECURSE pybind_sources ${CMAKE_CURRENT_SOURCE_DIR}/PybindModule/*.cpp)

pybind11_add_module(${target_name} MODULE ${pybind_sources})

target_include_directories(${target_name} PRIVATE ${ROOT_DIR}/extern/pybind11/include)
target_link_libraries(${target_name} PUBLIC ${cpp_target_name})
set_target_properties(${target_name} PROPERTIES OUTPUT_NAME "bindings")

find_package(Python3 COMPONENTS Interpreter Development.Module REQUIRED)

set(PYBIND_INSTALL_PREFIX ${CMAKE_INSTALL_PREFIX})

install(
    TARGETS ${target_name}
    COMPONENT bindings
    LIBRARY DESTINATION ${PYBIND_INSTALL_PREFIX}
    ARCHIVE DESTINATION ${PYBIND_INSTALL_PREFIX}
    RUNTIME DESTINATION ${PYBIND_INSTALL_PREFIX}
)

endif()