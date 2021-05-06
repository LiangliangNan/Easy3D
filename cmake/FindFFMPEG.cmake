# ******************************************************************************
#      Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
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
# ******************************************************************************



# ------------------------------------------------------------------------------
# This file sets up FFMPEG for CMake. Once done this will define
#
#   FFMPEG_FOUND           - the system has FFMPEG
#   FFMPEG_INCLUDE_DIRS    - the FFMPEG include directories
#   FFMPEG_LIBRARIES       - the FFMPEG libraries
#
#  To use FindFFMPEG.cmake, add the following lines in your CMakeLists file (modify the Hardcoded search paths
#  if necessary):
#       include(cmake/FindFFMPEG.cmake)
#       if (FFMPEG_FOUND)
#           target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_FFMPEG)
#           target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIRS})
#           target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARIES})
#       endif()
#
#   # On macOS, use the following command to install FFMPEG:
#       brew install ffmpeg
#   # On Linux, use:
#       sudo apt-get install libavcodec-dev
#       sudo apt-get install libavformat-dev
#       sudo apt-get install libswscale-dev
# ------------------------------------------------------------------------------


# Is it already configured?
if (NOT FFMPEG_FOUND)

    # Hardcoded search paths for include directories
    set(SEARCH_PATHS_FOR_HEADERS
            "$ENV{FFMPEG_HOME}/include"
            "/usr/local/Cellar/ffmpeg/4.3.1_4/include"
            "/usr/include/x86_64-linux-gnu"
            "C:\\Program Files\\ffmpeg\\include"
            )

    # Hardcoded search paths for libraries
    set(SEARCH_PATHS_FOR_LIBRARIES
            "$ENV{FFMPEG_HOME}/lib"
            "/usr/local/Cellar/ffmpeg/4.3.1_4/lib"
            "/usr/lib/x86_64-linux-gnu"
            "C:\\Program Files\\ffmpeg\\lib"
            )

    find_path(FFMPEG_INCLUDE_DIR libavcodec/avcodec.h PATHS ${SEARCH_PATHS_FOR_HEADERS})
    # setup header file directories
    set(FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIR})

    # the libs may be installed in different places?
    find_library(FFMPEG_LIBRARY_avcodec NAMES avcodec PATHS ${SEARCH_PATHS_FOR_LIBRARIES})
    find_library(FFMPEG_LIBRARY_avdevice NAMES avdevice libavdevice.so.57 PATHS ${SEARCH_PATHS_FOR_LIBRARIES})
    find_library(FFMPEG_LIBRARY_avfilter NAMES avfilter libavfilter.so.6 PATHS ${SEARCH_PATHS_FOR_LIBRARIES})
    find_library(FFMPEG_LIBRARY_avformat NAMES avformat PATHS ${SEARCH_PATHS_FOR_LIBRARIES})
    find_library(FFMPEG_LIBRARY_avresample NAMES avresample libavresample.so.3 PATHS ${SEARCH_PATHS_FOR_LIBRARIES})
    find_library(FFMPEG_LIBRARY_swscale NAMES swscale PATHS ${SEARCH_PATHS_FOR_LIBRARIES})
    find_library(FFMPEG_LIBRARY_swresample NAMES swresample PATHS ${SEARCH_PATHS_FOR_LIBRARIES})
    find_library(FFMPEG_LIBRARY_avutil NAMES avutil PATHS ${SEARCH_PATHS_FOR_LIBRARIES})
    find_library(FFMPEG_LIBRARY_postproc NAMES postproc libpostproc.so.54 PATHS ${SEARCH_PATHS_FOR_LIBRARIES})

    # setup libraries files
    set(FFMPEG_LIBRARIES
            ${FFMPEG_LIBRARY_avcodec} ${FFMPEG_LIBRARY_avdevice} ${FFMPEG_LIBRARY_avfilter} ${FFMPEG_LIBRARY_avformat}
            ${FFMPEG_LIBRARY_avresample} ${FFMPEG_LIBRARY_swscale} ${FFMPEG_LIBRARY_swresample} ${FFMPEG_LIBRARY_avutil}
            ${FFMPEG_LIBRARY_postproc}
            )
    string(STRIP "${FFMPEG_LIBRARIES}" FFMPEG_LIBRARIES)
endif ()

# Check that FFMPEG was successfully found
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(FFMPEG DEFAULT_MSG FFMPEG_INCLUDE_DIRS)
find_package_handle_standard_args(FFMPEG DEFAULT_MSG FFMPEG_LIBRARIES)

# Hide variables from CMake-Gui options
mark_as_advanced(FFMPEG_INCLUDE_DIR FFMPEG_CXX_LIBRARY_DEBUG FFMPEG_CXX_LIBRARY_RELEASE FFMPEG_C_LIBRARY)
