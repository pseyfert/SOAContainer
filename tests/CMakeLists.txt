cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

add_subdirectory(googletest EXCLUDE_FROM_ALL)

include_directories(${gtest_SOURCE_DIR}/include)

add_executable(SOAContainerVectorSimple SOAContainerVectorSimple.cc)
target_link_libraries(SOAContainerVectorSimple gtest gtest_main)
add_test(SOAContainerVectorSimple SOAContainerVectorSimple)

add_executable(SOAContainerDequeSimple SOAContainerDequeSimple.cc)
target_link_libraries(SOAContainerDequeSimple gtest gtest_main)
add_test(SOAContainerDequeSimple SOAContainerDequeSimple)

add_executable(SOAContainerVectorSimpleSkin SOAContainerVectorSimpleSkin.cc)
target_link_libraries(SOAContainerVectorSimpleSkin gtest gtest_main)
add_test(SOAContainerVectorSimpleSkin SOAContainerVectorSimpleSkin)

add_executable(SOAContainerDequeSimpleSkin SOAContainerDequeSimpleSkin.cc)
target_link_libraries(SOAContainerDequeSimpleSkin gtest gtest_main)
add_test(SOAContainerDequeSimpleSkin SOAContainerDequeSimpleSkin)

add_executable(SOAContainerArrayAsField SOAContainerArrayAsField.cc)
target_link_libraries(SOAContainerArrayAsField gtest gtest_main)
add_test(SOAContainerArrayAsField SOAContainerArrayAsField)

add_executable(SOAContainerVectorPartition SOAContainerVectorPartition.cc)
target_link_libraries(SOAContainerVectorPartition gtest gtest_main)
add_test(SOAContainerVectorPartition SOAContainerVectorPartition)

add_executable(SOAContainerVectorConvenientSkin SOAContainerVectorConvenientSkin.cc)
target_link_libraries(SOAContainerVectorConvenientSkin gtest gtest_main)
add_test(SOAContainerVectorConvenientSkin SOAContainerVectorConvenientSkin)

add_executable(SOAContainerVectorZipViews SOAContainerVectorZipViews.cc)
target_link_libraries(SOAContainerVectorZipViews gtest gtest_main)
add_test(SOAContainerVectorZipViews SOAContainerVectorZipViews)

add_executable(SOAContainerVectorFieldExtraction SOAContainerVectorFieldExtraction.cc)
target_link_libraries(SOAContainerVectorFieldExtraction gtest gtest_main)
add_test(SOAContainerVectorFieldExtraction SOAContainerVectorFieldExtraction)

add_executable(SOAViewSimple SOAViewSimple.cc)
target_link_libraries(SOAViewSimple gtest gtest_main)
add_test(SOAViewSimple SOAViewSimple)

add_executable(Branchless Branchless.cc)
target_link_libraries(Branchless gtest gtest_main)
add_test(Branchless Branchless)

add_executable(SOAContainerVectorEmplaceBack SOAContainerVectorEmplaceBack.cc)
target_link_libraries(SOAContainerVectorEmplaceBack gtest gtest_main)
add_test(SOAContainerVectorEmplaceBack SOAContainerVectorEmplaceBack)

add_executable(SOAIteratorRangeTest SOAIteratorRangeTest.cc)
target_link_libraries(SOAIteratorRangeTest gtest gtest_main)
add_test(SOAIteratorRangeTest SOAIteratorRangeTest)

# CMAKE 3
#target_include_directories(soa_container_test PUBLIC include)
#target_compile_options(soa_container_test PUBLIC -g -O2 -ffast-math -ftree-vectorize -march=native)

# Copyright (C) CERN for the benefit of the LHCb collaboration
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
