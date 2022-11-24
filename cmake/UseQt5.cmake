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
# This file sets up Qt5 for CMake. When Qt5 was setup successfuly, QT5_FOUND
# will be set.
#
# To use QT5_FOUND, you only need to include this file and specifying Qt libraries
# to link against, e.g.,
#       ------------------------------------------------------------------------
#           project(${PROJECT_NAME})
#           include( ../../cmake/UseQt5.cmake )
#           add_executable(${PROJECT_NAME}, main.cpp)
#           target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)
#       ------------------------------------------------------------------------
# NOTE: 'UseQt5.cmake' must be included after you define your project but before
#       'add_executable()' or 'add_library()'.
#
#   The recommended way to specify libraries and headers with CMake is to use the
#   target_link_libraries command. This command automatically adds appropriate
#   include directories, compile definitions, the position-independent-code flag,
#   and links to the qtmain.lib library on Windows.
# ------------------------------------------------------------------------------


# we will use cmake autouic/automoc/autorcc features
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)  # Tell CMake to run moc when necessary
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON) # As moc files are generated in the binary dir, tell CMake to always look for includes there

set(QT5_ROOT_PATH CACHE PATH "Qt5 root directory (i.e. where the 'bin' folder lies)")
if (QT5_ROOT_PATH)
    list(APPEND CMAKE_PREFIX_PATH ${QT5_ROOT_PATH})
endif ()

# find qt5 components
find_package(Qt5 COMPONENTS Core Gui OpenGL Widgets)
# or
# find_package(Qt5Core QUIET)
# find_package(Qt5Gui QUIET)
# find_package(Qt5OpenGL QUIET)
# find_package(Qt5Widgets QUIET)

# In the case no Qt5Config.cmake file could be found, cmake will explicitly ask the user for the QT5_DIR containing it!

if (Qt5Core_FOUND AND Qt5Gui_FOUND AND Qt5OpenGL_FOUND AND Qt5Widgets_FOUND)
    set(QT5_FOUND TRUE)
endif ()

if (QT5_FOUND)
    # Starting with the QtCore lib, find the bin and root directories
    get_target_property(QT5_LIB_LOCATION Qt5::Core LOCATION_${CMAKE_BUILD_TYPE})
    get_filename_component(QT_BINARY_DIR ${QT5_LIB_LOCATION} DIRECTORY)

    if (APPLE)
        # Apple uses frameworks - move up until we get to the base directory to set the bin directory properly
        get_filename_component(QT_BINARY_DIR ${QT_BINARY_DIR} DIRECTORY)
        set(QT_BINARY_DIR ${QT_BINARY_DIR}/bin)
        set(MACDEPLOYQT ${QT_BINARY_DIR}/macdeployqt)
        message(STATUS "macdeployqt: ${MACDEPLOYQT}")
    elseif(WIN32)
        set(WINDEPLOYQT ${QT_BINARY_DIR}/windeployqt.exe)
        message(STATUS "windeployqt: ${WINDEPLOYQT}")
    endif ()

    # set QT5_ROOT_PATH if it wasn't set by the user
    if (NOT QT5_ROOT_PATH)
        get_filename_component(QT5_ROOT_PATH ${QT_BINARY_DIR} DIRECTORY)
    endif ()

    # turn on QStringBuilder for more efficient string construction
    #	see https://doc.qt.io/qt-5/qstring.html#more-efficient-string-construction
    add_definitions(-DQT_USE_QSTRINGBUILDER)
endif ()
