cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(googletest)

include_directories(${gtest_SOURCE_DIR}/include)

add_executable(SOAContainerVectorSimple SOAContainerVectorSimple.cc ${headers})
target_link_libraries(SOAContainerVectorSimple gtest gtest_main)
add_test(SOAContainerVectorSimple SOAContainerVectorSimple)

add_executable(SOAContainerDequeSimple SOAContainerDequeSimple.cc ${headers})
target_link_libraries(SOAContainerDequeSimple gtest gtest_main)
add_test(SOAContainerDequeSimple SOAContainerDequeSimple)

add_executable(SOAContainerVectorSimpleSkin SOAContainerVectorSimpleSkin.cc ${headers})
target_link_libraries(SOAContainerVectorSimpleSkin gtest gtest_main)
add_test(SOAContainerVectorSimpleSkin SOAContainerVectorSimpleSkin)

add_executable(SOAContainerDequeSimpleSkin SOAContainerDequeSimpleSkin.cc ${headers})
target_link_libraries(SOAContainerDequeSimpleSkin gtest gtest_main)
add_test(SOAContainerDequeSimpleSkin SOAContainerDequeSimpleSkin)

add_executable(SOAContainerArrayAsField SOAContainerArrayAsField.cc ${headers})
target_link_libraries(SOAContainerArrayAsField gtest gtest_main)
add_test(SOAContainerArrayAsField SOAContainerArrayAsField)

add_executable(SOAContainerVectorPartition SOAContainerVectorPartition.cc ${headers})
target_link_libraries(SOAContainerVectorPartition gtest gtest_main)
add_test(SOAContainerVectorPartition SOAContainerVectorPartition)

add_executable(SOAContainerVectorConvenientSkin SOAContainerVectorConvenientSkin.cc ${headers})
target_link_libraries(SOAContainerVectorConvenientSkin gtest gtest_main)
add_test(SOAContainerVectorConvenientSkin SOAContainerVectorConvenientSkin)

add_executable(SOAContainerVectorZipViews SOAContainerVectorZipViews.cc ${headers})
target_link_libraries(SOAContainerVectorZipViews gtest gtest_main)
add_test(SOAContainerVectorZipViews SOAContainerVectorZipViews)

add_executable(SOAContainerVectorFieldExtraction SOAContainerVectorFieldExtraction.cc ${headers})
target_link_libraries(SOAContainerVectorFieldExtraction gtest gtest_main)
add_test(SOAContainerVectorFieldExtraction SOAContainerVectorFieldExtraction)

add_executable(SOAViewSimple SOAViewSimple.cc ${headers})
target_link_libraries(SOAViewSimple gtest gtest_main)
add_test(SOAViewSimple SOAViewSimple)

add_executable(Branchless Branchless.cc ${headers})
target_link_libraries(Branchless gtest gtest_main)
add_test(Branchless Branchless)

add_executable(EmplaceBackTest EmplaceBackTest.cc ${headers})
target_link_libraries(EmplaceBackTest gtest gtest_main)
add_test(EmplaceBackTest EmplaceBackTest)

add_executable(SOAIteratorRangeTest SOAIteratorRangeTest.cc ${headers})
target_link_libraries(SOAIteratorRangeTest gtest gtest_main)
add_test(SOAIteratorRangeTest SOAIteratorRangeTest)

# CMAKE 3
#target_include_directories(soa_container_test PUBLIC include)
#target_compile_options(soa_container_test PUBLIC -g -O2 -ffast-math -ftree-vectorize -march=native)
