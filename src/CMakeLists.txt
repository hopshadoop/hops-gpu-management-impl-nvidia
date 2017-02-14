project (hops_gpu)

cmake_minimum_required (VERSION 2.8)
include(JNIFlags.cmake NO_POLICY_SCOPE)
find_package(JNI REQUIRED)
INCLUDE_DIRECTORIES(${JNI_INCLUDE_DIRS})


include_directories (${CMAKE_SOURCE_DIR}/main/native/gpu/include)
#if(NOT CUDA_HOME)
#  message(STATUS "CUDA_HOME was not defined. Set it to Cuda installation dir")
#endif()

#link_directories(${$ENV{CUDA_HOME}/lib)

SET( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall -Wextra -Wformat-security  -Woverloaded-virtual -Wno-unused-parameter -Wno-unused-but-set-variable -Wno-strict-aliasing -Wno-switch -O3 -DDBUG_OFF" )

file(GLOB SOURCE ${CMAKE_SOURCE_DIR}/main/native/gpu/src/nvml.c)

add_library(hopsnvml SHARED ${SOURCE})

target_link_libraries(hopsnvml)

function(output_directory TGT DIR)
    SET_TARGET_PROPERTIES(${TGT} PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${DIR}")
    SET_TARGET_PROPERTIES(${TGT} PROPERTIES
        ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${DIR}")
    SET_TARGET_PROPERTIES(${TGT} PROPERTIES
        LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${DIR}")
endfunction(output_directory TGT DIR)


output_directory(hopsnvml ../classes)
