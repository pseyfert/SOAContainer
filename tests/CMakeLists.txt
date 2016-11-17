cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(googletest)

include_directories(${gtest_SOURCE_DIR}/include)

add_executable(SOAContainerTest SOAContainerTest.cc ${headers})
target_link_libraries(SOAContainerTest gtest gtest_main)
add_test(SOAContainerTest SOAContainerTest)

# CMAKE 3
#target_include_directories(soa_container_test PUBLIC include)
#target_compile_options(soa_container_test PUBLIC -g -O2 -ffast-math -ftree-vectorize -march=native)
