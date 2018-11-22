find_package(ROOT COMPONENTS Core Gui Graf3d Gpad)
include_directories(SYSTEM ${ROOT_INCLUDE_DIRS})
add_compile_options(-O3)
add_executable(benchmark benchmark.cc)
if(ROOT_FOUND)
add_executable(guitest guitest.cc)
set(quicktest "${CMAKE_SOURCE_DIR}/examples/nbody")
if (CMAKE_CURRENT_SOURCE_DIR STREQUAL quicktest)
set_property(TARGET guitest PROPERTY CXX_STANDARD 14)
endif()
find_package(TBB)
if(TBB_FOUND)
target_link_libraries(guitest ${ROOT_LIBRARIES} ${TBB_LIBRARIES})
else(TBB_FOUND)
target_link_libraries(guitest ${ROOT_LIBRARIES})
endif(TBB_FOUND)
endif(ROOT_FOUND)

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
