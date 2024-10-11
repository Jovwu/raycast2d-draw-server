cmake_minimum_required(VERSION 3.16)
project(jovwuserver LANGUAGES CXX)

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
  add_definitions(-DDEBUG)
else()
set(CMAKE_BUILD_TYPE Release)
endif()

message(STATUS "Current CMAKE_BUILD_TYPE: ${CMAKE_BUILD_TYPE}")

include(CheckCXXCompilerFlag)
check_cxx_compiler_flag("-mavx" COMPILER_SUPPORTS_AVX)
if(COMPILER_SUPPORTS_AVX)
    add_compile_options(-mavx)
endif()
set(COMPILER_SUPPORTS_AVX ${COMPILER_SUPPORTS_AVX} CACHE INTERNAL "Does the compiler support AVX?")

add_subdirectory(cuda)

aux_source_directory(${PROJECT_SOURCE_DIR}/src SRC)
include_directories(${PROJECT_SOURCE_DIR}/include 
                    ${PROJECT_SOURCE_DIR}/external
                    ${PROJECT_SOURCE_DIR}/external/uWebSockets
                    ${PROJECT_SOURCE_DIR}/external/spdlog
                    ${PROJECT_SOURCE_DIR}/external/redis-plus-plus/src
                    ${PROJECT_SOURCE_DIR}/external/hiredis
                    ${PROJECT_SOURCE_DIR}/external/libconfig) 

find_package(fmt REQUIRED)
find_package(ZLIB REQUIRED)
find_package(Protobuf REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(UUID REQUIRED uuid)
find_package(TBB REQUIRED)

find_path(HIREDIS_HEADER hiredis.h PATHS ${PROJECT_SOURCE_DIR}/external/hiredis)
find_library(HIREDIS_LIB hiredis PATHS ${PROJECT_SOURCE_DIR}/external/hiredis)
find_path(REDIS_PLUS_PLUS_HEADER sw/redis++/redis++.h PATHS ${PROJECT_SOURCE_DIR}/external/redis-plus-plus/src)
find_library(REDIS_PLUS_PLUS_LIB redis++ PATHS ${PROJECT_SOURCE_DIR}/external/redis-plus-plus/src)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
add_executable(jovwuserver ${SRC})

target_include_directories(jovwuserver PRIVATE ${HIREDIS_HEADER} ${REDIS_PLUS_PLUS_HEADER})
target_link_libraries(jovwuserver 
    ${HIREDIS_LIB} 
    ${REDIS_PLUS_PLUS_LIB} 
    fmt::fmt 
    ZLIB::ZLIB 
    ${PROJECT_SOURCE_DIR}/lib/uSockets.a 
    ${Protobuf_LIBRARIES} 
    ${UUID_LIBRARIES} 
    TBB::tbb
)

add_compile_options(
    -O3
    -fdump-tree-all -save-temps
    -Wall -Wextra -Wunused-variable -Wunused-function
)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(jovwuserver PRIVATE -fsanitize=address -g)
        target_link_options(jovwuserver PRIVATE -fsanitize=address)
        message(STATUS "AddressSanitizer enabled")
    endif()
endif()