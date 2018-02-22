add_executable(Points Points.cc)
add_executable(PointsWithConstructor PointsWithConstructor.cc)
add_executable(tutorial00 tutorial00.cc)
add_subdirectory(nbody)

find_package(BOOST QUIET)
if(${Boost_FOUND})
  if(${Boost_VERSION} VERSION_GREATER 106199)
    add_executable(SmallVectorPoints SmallVectorPoints.cc)
  else()
    message(STATUS "skipping SmallVectorPoints example because BOOST version is ${Boost_VERSION}, which is older than 1.62.")
  endif()
else()
  message(STATUS "skipping SmallVectorPoints example because BOOST is not available.")
endif()

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
