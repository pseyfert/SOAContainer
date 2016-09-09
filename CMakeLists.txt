cmake_minimum_required(VERSION 3.1 FATAL_ERROR)

# Set policies
cmake_policy(SET CMP0042 NEW) # ENABLE CMP0042: MACOSX_RPATH is enabled by default.

# Settable properties
set(META_PROJECT_NAME        "SOAContainer")

# C++11 without GNU extensions
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD_REQUIRED ON)



project(${META_PROJECT_NAME} C CXX)

file(GLOB headers include/*)

add_executable(soa_container_test tests/SOAContainerTest.cc ${headers})

target_include_directories(soa_container_test PUBLIC include)
target_compile_options(soa_container_test PUBLIC -g -O2 -ffast-math -ftree-vectorize -march=native)
