cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

# C++11 without GNU extensions
# Requires CMAKE 3.1+ for Mac
if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} LESS 3.1)
    add_compile_options(-std=c++14)
else()
    cmake_policy(VERSION 3.1) # Needed for Mac
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_EXTENSIONS OFF)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
endif()
# be moderately paranoid with flags
add_compile_options(-pedantic -Wall -Wextra)
#add_compile_options(-stdlib=libc++)

# CMAKE 2
include_directories(include)
add_compile_options(-g -O2 -ffast-math -ftree-vectorize -march=native)

project(SOAContainer CXX)

file(GLOB headers include/*)

enable_testing()
add_subdirectory(tests)
add_subdirectory(examples)

# CMAKE 3
#target_include_directories(soa_container_test PUBLIC include)
#target_compile_options(soa_container_test PUBLIC -g -O2 -ffast-math -ftree-vectorize -march=native)
