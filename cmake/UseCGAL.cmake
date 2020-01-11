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
# This file sets up CGAL for CMake. When CGAL was setup successfuly, CGAL_FOUND
# will be set.
#
# To use CGAL, you only need to add the following line in you CMakeLists file
#           include( ../../cmake/UseCGAL.cmake )
# NOTE: this must be done after add_executable() or add_library()
# ------------------------------------------------------------------------------


#---------------------------------------------------------------------------------------------
# CGAL
#---------------------------------------------------------------------------------------------

find_package(CGAL REQUIRED COMPONENTS Core)
if(CGAL_FOUND)
    message(STATUS "Found CGAL-${CGAL_MAJOR_VERSION}.${CGAL_MINOR_VERSION}.${CGAL_BUGFIX_VERSION}")

    message(STATUS "   CGAL_USE_FILE: ${CGAL_USE_FILE}")
    include(${CGAL_USE_FILE})

#    message(STATUS "   CGAL_DEFINITIONS: ${CGAL_DEFINITIONS}")
#    message(STATUS "   CGAL_3RD_PARTY_DEFINITIONS: ${CGAL_3RD_PARTY_DEFINITIONS}")
#    target_compile_definitions(${PROJECT_NAME} PRIVATE ${CGAL_DEFINITIONS} ${CGAL_3RD_PARTY_DEFINITIONS})

#    message(STATUS "   CGAL_INCLUDE_DIRS: ${CGAL_INCLUDE_DIRS}")
#    message(STATUS "   CGAL_3RD_PARTY_INCLUDE_DIRS: ${CGAL_3RD_PARTY_INCLUDE_DIRS}")
#    target_include_directories(${PROJECT_NAME} PRIVATE ${CGAL_INCLUDE_DIRS})
#    target_include_directories(${PROJECT_NAME} PRIVATE ${CGAL_3RD_PARTY_INCLUDE_DIRS} )

    message(STATUS "   CGAL_LIBRARIES: ${CGAL_LIBRARIES}")
    message(STATUS "   CGAL_3RD_PARTY_LIBRARIES: ${CGAL_3RD_PARTY_LIBRARIES}")
#   The recommended way to specify libraries and headers with CMake is to use the
#   target_link_libraries command. This command automatically adds appropriate
#   include directories, compile definitions, the position-independent-code lags.
    target_link_libraries(${PROJECT_NAME} ${CGAL_LIBRARIES} ${CGAL_3RD_PARTY_LIBRARIES})

else()
    message(FATAL_ERROR "CGAL was not found.")
endif()
#---------------------------------------------------------------------------------------------
# CGAL
#---------------------------------------------------------------------------------------------


#---------------------------------------------------------------------------------------------
# GMP
#---------------------------------------------------------------------------------------------
find_package(GMP REQUIRED)
if(GMP_FOUND)
    message(STATUS "   GMP_INCLUDE_DIR: ${GMP_INCLUDE_DIR}")
#    target_include_directories(${PROJECT_NAME} PRIVATE ${GMP_INCLUDE_DIR})

    message(STATUS "   GMP_LIBRARIES: ${GMP_LIBRARIES}")
#   The recommended way to specify libraries and headers with CMake is to use the
#   target_link_libraries command. This command automatically adds appropriate
#   include directories, compile definitions, the position-independent-code lags.
    target_link_libraries(${PROJECT_NAME} ${GMP_LIBRARIES})
else()
    message(FATAL_ERROR "GMP was not found.")
endif()
#---------------------------------------------------------------------------------------------
# GMP
#---------------------------------------------------------------------------------------------


#---------------------------------------------------------------------------------------------
# MPFR
#---------------------------------------------------------------------------------------------
find_package(MPFR REQUIRED)
if(MPFR_FOUND)
    message(STATUS "   MPFR_INCLUDE_DIR: ${MPFR_INCLUDE_DIR}")
#    target_include_directories(${PROJECT_NAME} PRIVATE ${MPFR_INCLUDE_DIR})

    message(STATUS "   MPFR_LIBRARIES: ${MPFR_LIBRARIES}")
#   The recommended way to specify libraries and headers with CMake is to use the
#   target_link_libraries command. This command automatically adds appropriate
#   include directories, compile definitions, the position-independent-code flags.
    target_link_libraries(${PROJECT_NAME} ${MPFR_LIBRARIES})
else()
    message(FATAL_ERROR "MPFR was not found.")
endif()
#---------------------------------------------------------------------------------------------
# MPFR
#---------------------------------------------------------------------------------------------
