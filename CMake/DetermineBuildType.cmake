include(CheckLanguage)

if(NOT DEFINED BUILD_TYPE)
    check_language(CUDA)
    
    if(CMAKE_CUDA_COMPILER)
        set(BUILD_TYPE "CUDA")
    else()
        message(STATUS "No CUDA support")
        set(BUILD_TYPE "CPU")
    endif()
endif()
