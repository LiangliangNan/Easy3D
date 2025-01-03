# *************************************************************************
#      Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
#      https://3d.bk.tudelft.nl/liangliang/
#
#      This file is part of Easy3D. If it is useful in your research/work,
#      I would be grateful if you show your appreciation by citing it:
#      ------------------------------------------------------------------
#           Liangliang Nan.
#           Easy3D: a lightweight, easy-to-use, and efficient C++ library
#           for processing and rendering 3D data.
#           Journal of Open Source Software, 6(64), 3255, 2021.
#      ------------------------------------------------------------------
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
# *************************************************************************


# ------------------------------------------------------------------------------
# This file sets up Qt for CMake. When Qt6 was setup successfully, 'Qt6_FOUND'
# will be set. If Qt6 is not found, it will try to find Qt5. If Qt5 is found,
# 'Qt5_FOUND' will be set. If both Qt6 and Qt5 are not found, it will stop the
# configuration and show an error message.
# If either Qt6 or Qt5 is found, it will set QtLibs to the corresponding Qt
# libraries, e.g., Qt5Core, Qt5Gui, Qt5Widgets, Qt5OpenGL, Qt5Xml, etc.
#
# To use Qt in your program, you only need to include this file and specifying
# Qt libraries to link against, e.g.,
#       ------------------------------------------------------------------------
#           project(${PROJECT_NAME})
#           include( ../cmake/UseQt.cmake )
#           add_executable(${PROJECT_NAME}, main.cpp)
#           target_link_libraries(${PROJECT_NAME} ${QtLibs})
#       ------------------------------------------------------------------------
# NOTE: 'UseQt.cmake' must be included after you define your project but before
#       'add_executable()' or 'add_library()'.
#
#   The recommended way to specify libraries and headers with CMake is to use the
#   target_link_libraries command. This command automatically adds appropriate
#   include directories, compile definitions, the position-independent-code flag,
#   and links to the qtmain.lib library on Windows.
# ------------------------------------------------------------------------------

# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed.
set(CMAKE_AUTOMOC ON)
# Instruct CMake to run uic automatically when needed.
set(CMAKE_AUTOUIC ON)
# Instruct CMake to run rcc automatically when needed.
set(CMAKE_AUTORCC ON)

# This will find the Qt files.
find_package(Qt6 COMPONENTS Core Widgets OpenGL OpenGLWidgets QUIET)
if (Qt6_FOUND)
    message(STATUS "Found Qt6 version: ${Qt6Core_VERSION}")
    message(STATUS "Qt6 directory: ${Qt6_DIR}")
    set(QtLibs Qt::Core Qt::Widgets Qt::OpenGL Qt::OpenGLWidgets)
else()
    find_package(Qt5 COMPONENTS Core Widgets OpenGL QUIET)
    if (Qt5_FOUND)
        message(STATUS "Found Qt5 version: ${Qt5Core_VERSION}")
        message(STATUS "Qt5 directory: ${Qt5_DIR}")
        set(QtLibs Qt5::Core Qt5::Widgets Qt5::OpenGL)
    else()
        message(FATAL_ERROR "Qt is required, either Qt6 or Qt5, but both cannot be found")
    endif()
endif()