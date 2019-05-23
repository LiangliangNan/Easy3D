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
# This file sets up Qt for CMake.
#
# Quite simple, you only need to do the following two steps:
# -) include this file after you define your project, but before add_executable() or add_library, e.g.,
#           project(${PROJECT_NAME})
#           include( ../../cmake/qt5.cmake )
#           add_executable(${PROJECT_NAME}, main.cpp)
# -) specify the Qt libraries to link against, e.g.,
#           target_link_libraries(${PROJECT_NAME} Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)
# ------------------------------------------------------------------------------


# we will use cmake autouic/automoc/autorcc features
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set( QT5_ROOT_PATH CACHE PATH "Qt5 root directory (i.e. where the 'bin' folder lies)" )
if ( QT5_ROOT_PATH )
	list( APPEND CMAKE_PREFIX_PATH ${QT5_ROOT_PATH} )
endif()

# find qt5 components

# find_package(Qt5 COMPONENTS Core Gui OpenGL Widgets REQUIRED)

find_package(Qt5Widgets QUIET)
if (Qt5Widgets_FOUND)
    find_package(Qt5Core REQUIRED)
    find_package(Qt5Gui REQUIRED)
    find_package(Qt5OpenGL REQUIRED)

    # in the case no Qt5Config.cmake file could be found, cmake will explicitly ask the user for the QT5_DIR containing it!
    # thus no need to keep additional variables and checks

    # Starting with the QtCore lib, find the bin and root directories
    get_target_property(QT5_LIB_LOCATION Qt5::Core LOCATION_${CMAKE_BUILD_TYPE})
    get_filename_component(QT_BINARY_DIR ${QT5_LIB_LOCATION} DIRECTORY)

    # Apple uses frameworks - move up until we get to the base directory to set the bin directory properly
    if ( APPLE )
            get_filename_component(QT_BINARY_DIR ${QT_BINARY_DIR} DIRECTORY)
            set(QT_BINARY_DIR "${QT_BINARY_DIR}/bin")

            set( MACDEPLOYQT "${QT_BINARY_DIR}/macdeployqt" )
    endif()

    # set QT5_ROOT_PATH if it wasn't set by the user
    if ( NOT QT5_ROOT_PATH )
            get_filename_component(QT5_ROOT_PATH ${QT_BINARY_DIR} DIRECTORY)
    endif()

    include_directories(${Qt5OpenGL_INCLUDE_DIRS}
                        ${Qt5Widgets_INCLUDE_DIRS}
                        ${Qt5Core_INCLUDE_DIRS}
                        ${Qt5Gui_INCLUDE_DIRS}
                       )

    # turn on QStringBuilder for more efficient string construction
    #	see https://doc.qt.io/qt-5/qstring.html#more-efficient-string-construction
    add_definitions( -DQT_USE_QSTRINGBUILDER )

endif()

