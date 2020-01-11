# ------------------------------------------------------------------------------
#      Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
#      https://3d.bk.tudelft.nl/liangliang/
#
#      This file is part of Easy3D: software for processing and rendering
#      meshes and point clouds.
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
# This file sets up Gurobi for CMake. Once done this will define
#
#   GUROBI_FOUND           - system has GUROBI
#   GUROBI_INCLUDE_DIRS    - the GUROBI include directories
#   GUROBI_LIBRARIES       - Link these to use GUROBI
#
#  In your CMakeLists file, you need to add, e.g. (modify it if necessary):
#        if (GUROBI_FOUND)
#            message(STATUS "Gurobi include dir: " ${GUROBI_INCLUDE_DIRS})
#            message(STATUS "Gurobi libraries: " ${GUROBI_LIBRARIES})
#            target_compile_definitions(${PROJECT_NAME} PUBLIC HAS_GUROBI)
#            target_include_directories(${PROJECT_NAME} PRIVATE ${GUROBI_INCLUDE_DIRS})
#            target_link_libraries(${PROJECT_NAME} PRIVATE ${GUROBI_LIBRARIES})
#        endif()
# ------------------------------------------------------------------------------


# Is it already configured?
if (NOT GUROBI_FOUND)

    # Hardcoded search paths
    set(SEARCH_PATHS_FOR_HEADERS
        "$ENV{GUROBI_HOME}/include"
        "/Library/gurobi810/mac64/include"
        "C:\\dev\\gurobi810\\win64\\include"
    )

    set(SEARCH_PATHS_FOR_LIBRARIES
        "$ENV{GUROBI_HOME}/lib"
        "/Library/gurobi810/mac64/lib"
        "C:\\dev\\gurobi810\\win64\\lib"
    )

    find_path(GUROBI_INCLUDE_DIR gurobi_c++.h
      PATHS ${SEARCH_PATHS_FOR_HEADERS}
    )


    find_library( GUROBI_C_LIBRARY
                  NAMES gurobi81
                  PATHS ${SEARCH_PATHS_FOR_LIBRARIES}
                  )

    find_library( GUROBI_CXX_LIBRARY_DEBUG
                NAMES gurobi_c++ gurobi_c++mdd2017
                PATHS ${SEARCH_PATHS_FOR_LIBRARIES}
                )

    find_library( GUROBI_CXX_LIBRARY_RELEASE
                NAMES gurobi_c++ gurobi_c++md2017
                PATHS ${SEARCH_PATHS_FOR_LIBRARIES}
                )

    # setup header file directories
    set(GUROBI_INCLUDE_DIRS ${GUROBI_INCLUDE_DIR})

endif()


# setup libraries files
set(GUROBI_LIBRARIES ${GUROBI_C_LIBRARY}
    debug ${GUROBI_CXX_LIBRARY_DEBUG}
    optimized ${GUROBI_CXX_LIBRARY_RELEASE}
    )


# Check that Gurobi was successfully found
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(GUROBI DEFAULT_MSG GUROBI_INCLUDE_DIRS)
find_package_handle_standard_args(GUROBI DEFAULT_MSG GUROBI_LIBRARIES)

# Hide variables from CMake-Gui options
mark_as_advanced(GUROBI_LIBRARIES GUROBI_INCLUDE_DIRS GUROBI_INCLUDE_DIR)
