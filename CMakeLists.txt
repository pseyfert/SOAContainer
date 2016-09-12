cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# Set policies CMAKE 3
# cmake_policy(SET CMP0042 NEW) # ENABLE CMP0042: MACOSX_RPATH is enabled by default.

# Settable properties
set(META_PROJECT_NAME        "SOAContainer")

# C++11 without GNU extensions
# Requires CMAKE 3.1
#set(CMAKE_CXX_STANDARD 11)
#set(CMAKE_CXX_EXTENSIONS OFF)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

# CMAKE 2
include_directories(include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g -O2 -ffast-math -ftree-vectorize -march=native")

project(${META_PROJECT_NAME} CXX)

file(GLOB headers include/*)

add_executable(soa_container_test tests/SOAContainerTest.cc ${headers})
add_executable(soa_points examples/Points.cc ${headers})

# CMAKE 3
#target_include_directories(soa_container_test PUBLIC include)
#target_compile_options(soa_container_test PUBLIC -g -O2 -ffast-math -ftree-vectorize -march=native)
