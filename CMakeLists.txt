cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# C++11 without GNU extensions
# Requires CMAKE 3.1+ for Mac
if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.1)
    cmake_policy(VERSION 3.1) # Needed for Mac
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
endif()

# CMAKE 2
include_directories(include)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O2 -ffast-math -ftree-vectorize -march=native")

project(SOAContainer CXX)

file(GLOB headers include/*)

enable_testing()
add_subdirectory(tests)
add_subdirectory(examples)

# CMAKE 3
#target_include_directories(soa_container_test PUBLIC include)
#target_compile_options(soa_container_test PUBLIC -g -O2 -ffast-math -ftree-vectorize -march=native)
