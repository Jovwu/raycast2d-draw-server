cmake_minimum_required(VERSION 3.16)
project(jovwucuda LANGUAGES CXX CUDA)

get_filename_component(PARENT_DIR ${PROJECT_SOURCE_DIR} DIRECTORY)

set(CMAKE_CXX_FLAGS "-std=c++17 -DCMAKE_CUDA_HOST_COMPILER=/usr/bin/g++-10")
set(CMAKE_CUDA_FLAGS "-g -G -std=c++17")

if(COMPILER_SUPPORTS_AVX)
    add_compile_options(-mavx)
    message(STATUS "CUDA AVX support enabled")
endif()

aux_source_directory(${PARENT_DIR}/cuda CUDA_SRC)
list(APPEND CUDA_SRC ${PARENT_DIR}/src/mredis.cc)
list(APPEND CUDA_SRC ${PARENT_DIR}/src/camera.cc)

include_directories(${PARENT_DIR}/cuda
                    ${PARENT_DIR}/include
                    ${PARENT_DIR}/external
                    ${PARENT_DIR}/external/redis-plus-plus/src
                    ${PARENT_DIR}/external/hiredis)


set(CMAKE_CUDA_ARCHITECTURES 37)

find_package(TBB REQUIRED)

set(EXECUTABLE_OUTPUT_PATH ${PARENT_DIR}/bin)
add_executable(jovwucuda ${CUDA_SRC})

target_include_directories(jovwucuda INTERFACE ${PARENT_DIR}/include)
target_include_directories(jovwucuda PRIVATE ${HIREDIS_HEADER})
target_link_libraries(jovwucuda ${HIREDIS_LIB})
target_include_directories(jovwucuda PRIVATE ${REDIS_PLUS_PLUS_HEADER})
target_link_libraries(jovwucuda ${REDIS_PLUS_PLUS_LIB})
target_link_libraries(jovwucuda TBB::tbb)

target_compile_definitions(jovwucuda PRIVATE __CUDA_COMPILE__)