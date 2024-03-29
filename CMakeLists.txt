cmake_minimum_required(VERSION 2.8 FATAL_ERROR)
project(SOAContainer CXX)

# do not override host project's compilation if SOAContainer is not the top-level
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL CMAKE_SOURCE_DIR)
    # C++11 without GNU extensions
    # Requires CMAKE 3.1+ for Mac
    if(${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} LESS 3.1)
        add_compile_options(-std=c++11)
    else()
        cmake_policy(VERSION 3.1) # Needed for Mac
        set(CMAKE_CXX_STANDARD 11)
        set(CMAKE_CXX_EXTENSIONS OFF)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
    endif()
    # be moderately paranoid with flags
    add_compile_options(-Wpedantic -Wall -Wextra)
    add_compile_options(-ffast-math -ftree-vectorize -march=native)
endif()

# CMAKE 2
include_directories(include)
if (NOT CMAKE_BUILD_TYPE)
  message(STATUS "No build type selected, default to RelWithDebInfo")
  set(CMAKE_BUILD_TYPE "RelWithDebInfo") # -O2 -g -DNDEBUG
endif()

if(${CMAKE_GENERATOR} STREQUAL "Ninja")
  add_compile_options(-fdiagnostics-color)
endif()

if(NOT DEFINED SOAContainer_disable_tests)
  set(SOAContainer_disable_tests FALSE CACHE BOOL "disable testing")
endif()

if(NOT SOAContainer_disable_tests)
  enable_testing()
  add_subdirectory(tests)
endif()

add_subdirectory(examples)

if(NOT DEFINED SOAContainer_header_destination)
  set(SOAContainer_header_destination ${CMAKE_INSTALL_PREFIX}/include CACHE STRING "installation directory for SOAContainer headers")
endif()
install(DIRECTORY include/ DESTINATION ${SOAContainer_header_destination})

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
