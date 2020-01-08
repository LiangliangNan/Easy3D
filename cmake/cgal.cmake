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
# This file sets up CGAL for CMake.
#
# Super easy! You only need to add the following line in you CMakeLists file
#           include( ../../cmake/cgal.cmake )
# Note: this must be done after add_executable() or add_library()
# ------------------------------------------------------------------------------


#---------------------------------------------------------------------------------------------
# CGAL
#---------------------------------------------------------------------------------------------

find_package(CGAL REQUIRED COMPONENTS Core)
if(CGAL_FOUND)
    message("Found CGAL.")
    include(${CGAL_USE_FILE})

    message("CGAL_CXX_FLAGS_INIT: ${CGAL_CXX_FLAGS_INIT}")
    add_definitions(${CGAL_CXX_GLAFS_INIT})

    message("CGAL_LIBRARIES: ${CGAL_LIBRARIES}")
    message("CGAL_3RD_PARTY_LIBRARIES: ${CGAL_3RD_PARTY_LIBRARIES}")

#   For performance reasons, the variable CMAKE_BUILD_TYPE is set to "Release"
#   set(CMAKE_BUILD_TYPE Release)

    target_link_libraries(${PROJECT_NAME} ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES})
else()
    message(FATAL_ERROR "CGAL is required.")
endif()

find_package(GMP REQUIRED)
if(GMP_FOUND)
    message("GMP_INCLUDE_DIR: ${GMP_INCLUDE_DIR}")
    message("GMP_LIBRARIES: ${GMP_LIBRARIES}")
    target_link_libraries(${PROJECT_NAME} ${GMP_LIBRARIES})
else()
    message(FATAL_ERROR "GMP is required.")
endif()

find_package(MPFR REQUIRED)
if(MPFR_FOUND)
    message("MPFR_INCLUDE_DIR: ${MPFR_INCLUDE_DIR}")
    message("MPFR_LIBRARIES: ${MPFR_LIBRARIES}")
    target_link_libraries(${PROJECT_NAME} ${MPFR_LIBRARIES})
else()
    message(FATAL_ERROR "MPFR is required.")
endif()


#---------------------------------------------------------------------------------------------
# CGAL
#---------------------------------------------------------------------------------------------
