cmake_minimum_required(VERSION 3.1)

if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "10.12" CACHE STRING "Minimum OS X deployment version" FORCE)
    message(STATUS "Minimum OS X deployment version: ${CMAKE_OSX_DEPLOYMENT_TARGET}")
endif ()

################################################################################

project(Easy3D)

set(EASY3D_MAJOR_VERSION 2)
set(EASY3D_MINOR_VERSION 4)
set(EASY3D_PATCH_VERSION 4)
set(EASY3D_VERSION "${EASY3D_MAJOR_VERSION}.${EASY3D_MINOR_VERSION}.${EASY3D_PATCH_VERSION}")

################################################################################

# Discourage users to build Easy3D directly in its root directory
if (${CMAKE_CURRENT_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    if (NOT SAME_BUILD_AND_SOURCE_FOLDER_WARNING_ALREADY_ISSUED)
        message(WARNING "It is NOT advised to build Easy3D directly in its root directory!")
        set(SAME_BUILD_AND_SOURCE_FOLDER_WARNING_ALREADY_ISSUED TRUE)
    endif ()
endif ()

################################################################################

#math(EXPR EASY3D_OS_BITS "8 * ${CMAKE_SIZEOF_VOID_P}")
#message(STATUS "Easy3d: I'am woking on ${EASY3D_OS_BITS}-bit ${CMAKE_SYSTEM_NAME}")
if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "Easy3D: I am working on a 64-bit ${CMAKE_SYSTEM_NAME}")
    add_definitions(-DENV_64_BIT)
else ()
    message(STATUS "Easy3D: I am working on a 32-bit ${CMAKE_SYSTEM_NAME}")
    add_definitions(-DENV_32_BIT)
endif ()

################################################################################

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

################################################################################

# Detects whether this is a top-level project
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(EASY3D_TOPLEVEL_PROJECT ON)
else ()
    set(EASY3D_TOPLEVEL_PROJECT OFF)
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################

# Build tutorials
option(EASY3D_BUILD_TUTORIALS       "Build Easy3D Tutorials"                                ON)
# Build documentation
option(EASY3D_BUILD_DOCUMENTATION   "Build Easy3D Documentation"                            OFF)
# Build tests
option(EASY3D_BUILD_TESTS           "Build Easy3D Tests"                                    OFF)
# Build advanced examples/applications that require Qt (>= v5.6)
option(EASY3D_ENABLE_QT             "Build advanced examples/applications that require Qt (>= v5.6)"    OFF)
# Build advanced features that require CGAL (>= v5.1)
option(EASY3D_ENABLE_CGAL           "Build advanced features that require CGAL (>= v5.1)"               OFF)

################################################################################

if (EASY3D_ENABLE_CGAL)
    find_package(CGAL QUIET)
    if (CGAL_FOUND)
        if (CGAL_VERSION VERSION_GREATER_EQUAL "5.1")
            set(EASY3D_HAS_CGAL TRUE)
            add_definitions(-DHAS_CGAL)
            message(STATUS "Found CGAL v${CGAL_VERSION}")
        else ()
            set(EASY3D_HAS_CGAL FALSE)
            message(FATAL_ERROR "You have requested CGAL support and CGAL v${CGAL_VERSION} is found. However, Easy3D"
                    " requires at least v5.1. Please install CGAL v5.1 or later. In case you have multiple versions"
                    " of CGAL on your platform, provide the path of a suitable one to the CMake variable `CGAL_DIR`.\n"
                    " Please note that only the following surface mesh algorithms require CGAL (v5.1 or later): \n"
                    "   - reorientation; \n"
                    "   - detecting/resolving duplicate vertices/faces; \n"
                    "   - detecting/resolving self-intersection; \n"
                    "   - clipping/splitting/slicing. \n"
                    " If you don't need these algorithms, disable CGAL support by setting `EASY3D_ENABLE_CGAL` to `OFF`.\n"
                    " You will still be able to use all other features provided by Easy3D without CGAL.")
            return()
        endif ()
    else ()
        set(EASY3D_HAS_CGAL FALSE)
        message(FATAL_ERROR "You have requested CGAL support but CGAL (v5.1 or later required) was not found. You can"
                " set 'CGAL_DIR' to the directory containing `CGALConfig.cmake` or `cgal-config.cmake` to help CMake find CGAL.\n"
                " Please note that only the following surface mesh algorithms require CGAL (v5.1 or later): \n"
                "   - reorientation; \n"
                "   - detecting/resolving duplicate vertices/faces; \n"
                "   - detecting/resolving self-intersection; \n"
                "   - clipping/splitting/slicing. \n"
                " If you don't need these algorithms, disable CGAL support by setting `EASY3D_ENABLE_CGAL` to `OFF`.\n"
                " You will still be able to use all other features provided by Easy3D without CGAL.")
        return()
    endif ()
endif ()

################################################################################

### Configuration
set(EASY3D_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(EASY3D_THIRD_PARTY ${EASY3D_ROOT}/3rd_party)

set(EASY3D_INCLUDE_DIR ${EASY3D_ROOT})
set(EASY3D_SOURCE_DIR ${EASY3D_ROOT})
set(EASY3D_BINARY_DIR ${CMAKE_BINARY_DIR})

### conditionally compile certain modules depending on libraries found on the system
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

################################################################################

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

################################################################################

add_subdirectory(3rd_party)

add_subdirectory(easy3d)

if (EASY3D_BUILD_TUTORIALS)
    add_subdirectory(tutorials)
endif ()

if (EASY3D_BUILD_TESTS)
    add_subdirectory(tests)
endif ()

add_subdirectory(applications)

################################################################################

if (EASY3D_BUILD_DOCUMENTATION)
    # generation of Easy3D documentation requires doxygen
    find_package(Doxygen)
    if (${DOXYGEN_FOUND})
        message(STATUS "Found Doxygen: " ${DOXYGEN_EXECUTABLE})
        # configures Doxygen
        configure_file(doc/Doxyfile.in Doxyfile @ONLY)
        add_custom_target(doc ALL ${DOXYGEN_EXECUTABLE} Doxyfile
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Generating Doxygen documentation")
    else ()
        message(WARNING "Building documentation requires Doxygen but Doxygen was not found. Building documentation is "
                "thus disabled. You can set 'DOXYGEN_EXECUTABLE' to the directory containing the Doxygen executable "
                "if Doxygen already exists (otherwise install Doxygen first).")
    endif ()
endif ()

################################################################################

# hide some variables that might be set in 3rd_party libraries
mark_as_advanced(FORCE BUILD_SHARED_LIBS)

################################################################################
