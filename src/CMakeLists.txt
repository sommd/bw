cmake_minimum_required(VERSION 3.2)
project(bw VERSION 1.0)

# Generate config.h
set(PROJECT_BUGREPORT "<https://github.com/sommd/bw/issues/new>")
configure_file("${PROJECT_SOURCE_DIR}/config.h.in" "${PROJECT_BINARY_DIR}/config.h")
include_directories("${PROJECT_BINARY_DIR}")

set(SOURCE_FILES bw.c bitwise.c utils.c)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O2 -Wall")
add_executable(bw ${SOURCE_FILES})
