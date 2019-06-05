cmake_minimum_required(VERSION 2.8 FATAL_ERROR)

set(tests
  SOAContainerVectorSimple
  SOAContainerDequeSimple
  SOAContainerVectorSimpleSkin
  SOAContainerDequeSimpleSkin
  SOAContainerArrayAsField
  SOAContainerVectorPartition
  SOAContainerVectorConvenientSkin
  SOAContainerVectorZipViews
  SOAZipTestPaul
  SOAContainerVectorFieldExtraction
  SOAViewSimple
  Branchless
  SOAContainerVectorEmplaceBack
  SOAIteratorRangeTest
  SOATaggedType
  SOAAlgorithms
  )

foreach(test ${tests})
  add_executable(${test} ${test}.cc)
endforeach()

target_compile_options(SOAContainerDequeSimpleSkin PUBLIC "-Wno-deprecated-declarations")
target_compile_options(SOAContainerDequeSimple PUBLIC "-Wno-deprecated-declarations")

add_compile_options(-Wno-suggest-override)

add_subdirectory(googletest EXCLUDE_FROM_ALL)

include_directories(SYSTEM ${gtest_SOURCE_DIR}/include)

foreach(test ${tests})
  target_link_libraries(${test} gtest gtest_main pthread)
  if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} LESS 3.0)
    add_test(${test} ${test})
  else()
    add_test(NAME ${test} COMMAND ${test})
  endif()
endforeach()

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
#
# In applying this licence, CERN does not waive the privileges and immunities
# granted to it by virtue of its status as an Intergovernmental Organization
# or submit itself to any jurisdiction.
