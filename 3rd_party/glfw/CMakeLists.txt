cmake_minimum_required(VERSION 3.12...3.20 FATAL_ERROR)

project(GLFW VERSION 3.4.0 LANGUAGES C)

set(CMAKE_LEGACY_CYGWIN_WIN32 OFF)

if (POLICY CMP0054)
    cmake_policy(SET CMP0054 NEW)
endif()

if (POLICY CMP0069)
    cmake_policy(SET CMP0069 NEW)
endif()

if (POLICY CMP0077)
    cmake_policy(SET CMP0077 NEW)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(GLFW_STANDALONE TRUE)
endif()

include(CMakeDependentOption)

if (GLFW_USE_OSMESA)
    message(FATAL_ERROR "GLFW_USE_OSMESA has been removed; set the GLFW_PLATFORM init hint")
endif()

cmake_dependent_option(GLFW_BUILD_WIN32 "Build support for Win32" ON "WIN32" OFF)
cmake_dependent_option(GLFW_BUILD_COCOA "Build support for Cocoa" ON "APPLE" OFF)
cmake_dependent_option(GLFW_BUILD_X11 "Build support for X11" ON "UNIX;NOT APPLE" OFF)
cmake_dependent_option(GLFW_BUILD_WAYLAND "Build support for Wayland"
    "${GLFW_USE_WAYLAND}" "UNIX;NOT APPLE" OFF)

cmake_dependent_option(GLFW_USE_HYBRID_HPG "Force use of high-performance GPU on hybrid systems" OFF
                       "WIN32" OFF)
cmake_dependent_option(USE_MSVC_RUNTIME_LIBRARY_DLL "Use MSVC runtime library DLL" ON
                       "MSVC" OFF)

set(GLFW_LIBRARY_TYPE "${GLFW_LIBRARY_TYPE}" CACHE STRING
    "Library type override for GLFW (SHARED, STATIC, OBJECT, or empty to follow BUILD_SHARED_LIBS)")

if (GLFW_LIBRARY_TYPE)
    if (GLFW_LIBRARY_TYPE STREQUAL "SHARED")
        set(GLFW_BUILD_SHARED_LIBRARY TRUE)
    else()
        set(GLFW_BUILD_SHARED_LIBRARY FALSE)
    endif()
else()
    set(GLFW_BUILD_SHARED_LIBRARY ${BUILD_SHARED_LIBS})
endif()

list(APPEND CMAKE_MODULE_PATH "${GLFW_SOURCE_DIR}/CMake/modules")

find_package(Threads REQUIRED)

#--------------------------------------------------------------------
# Report backend selection
#--------------------------------------------------------------------
if (GLFW_BUILD_WIN32)
    message(STATUS "Including Win32 support")
endif()
if (GLFW_BUILD_COCOA)
    message(STATUS "Including Cocoa support")
endif()
if (GLFW_BUILD_WAYLAND)
    message(STATUS "Including Wayland support")
endif()
if (GLFW_BUILD_X11)
    message(STATUS "Including X11 support")
endif()

#--------------------------------------------------------------------
# Apply Microsoft C runtime library option
# This is here because it also applies to tests and examples
#--------------------------------------------------------------------
if (MSVC AND NOT USE_MSVC_RUNTIME_LIBRARY_DLL)
    if (CMAKE_VERSION VERSION_LESS 3.15)
        foreach (flag CMAKE_C_FLAGS
                      CMAKE_C_FLAGS_DEBUG
                      CMAKE_C_FLAGS_RELEASE
                      CMAKE_C_FLAGS_MINSIZEREL
                      CMAKE_C_FLAGS_RELWITHDEBINFO)

            if (flag MATCHES "/MD")
                string(REGEX REPLACE "/MD" "/MT" ${flag} "${${flag}}")
            endif()
            if (flag MATCHES "/MDd")
                string(REGEX REPLACE "/MDd" "/MTd" ${flag} "${${flag}}")
            endif()

        endforeach()
    else()
        set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    endif()
endif()

#--------------------------------------------------------------------
# Create generated files
#--------------------------------------------------------------------
include(CMakePackageConfigHelpers)

set(GLFW_CONFIG_PATH "${CMAKE_INSTALL_LIBDIR}/cmake/glfw3")

configure_package_config_file(CMake/glfw3Config.cmake.in
                              src/glfw3Config.cmake
                              INSTALL_DESTINATION "${GLFW_CONFIG_PATH}"
                              NO_CHECK_REQUIRED_COMPONENTS_MACRO)

write_basic_package_version_file(src/glfw3ConfigVersion.cmake
                                 VERSION ${GLFW_VERSION}
                                 COMPATIBILITY SameMajorVersion)

#--------------------------------------------------------------------
# Add subdirectories
#--------------------------------------------------------------------
add_subdirectory(src)