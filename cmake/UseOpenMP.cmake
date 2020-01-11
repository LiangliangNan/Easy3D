# ------------------------------------------------------------------------------
#      Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
#      https://3d.bk.tudelft.nl/liangliang/
#
#      This file is part of Easy3D. If it is useful in your research/work,
#      I would be grateful if you show your appreciation by citing it:
#       ------------------------------------------------------------------
#           Liangliang Nan.
#           Easy3D: a lightweight, easy-to-use, and efficient C++
#           library for processing and rendering 3D data. 2018.
#       ------------------------------------------------------------------
#
#      Easy3D is free software; you can redistribute it and/or modify
#      it under the terms of the GNU General Public License Version 3
#      as published by the Free Software Foundation.
#
#      Easy3D is distributed in the hope that it will be useful,
#      but WITHOUT ANY WARRANTY; without even the implied warranty of
#      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
#      GNU General Public License for more details.
#
#      You should have received a copy of the GNU General Public License
#      along with this program. If not, see <http://www.gnu.org/licenses/>.
# ------------------------------------------------------------------------------


# ------------------------------------------------------------------------------
# This file sets up OpenMP for CMake. When OpenMP was setup successfuly, OPENMP_FOUND
# will be set.
#
# To use OpenMP, you only need to add the following line in you CMakeLists file
#           include( ../../cmake/UseOpenMP.cmake )
# ------------------------------------------------------------------------------


#---------------------------------------------------------------------------------------------
# OpenMP
#---------------------------------------------------------------------------------------------

find_package(OpenMP)
if (OPENMP_FOUND)
    message(STATUS "Found OpenMP")
    message(STATUS "   OpenMP_C_FLAGS: ${OpenMP_C_FLAGS}")
    message(STATUS "   OpenMP_CXX_FLAGS: ${OpenMP_CXX_FLAGS}")
    set (CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()
#---------------------------------------------------------------------------------------------
# OpenMP
#---------------------------------------------------------------------------------------------

