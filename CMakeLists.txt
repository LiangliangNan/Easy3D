cmake_minimum_required(VERSION 3.1)

################################################################################

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

set (PROJECT_MAJOR 2)
set (PROJECT_MINOR 4)
set (PROJECT_PATCH 2)
set (PROJECT_VERSION "${PROJECT_MAJOR}.${PROJECT_MINOR}.${PROJECT_PATCH}")


################################################################################

# One shouldn't generate the BUILD project directly in the SOURCES folder!
if ( ${CMAKE_CURRENT_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR} )
        if ( NOT SAME_BUILD_AND_SOURCE_FOLDER_WARNING_ALREADY_ISSUED )
                message(FATAL_ERROR
                    "It is not advised to BUILD the binaries directly in the SOURCE folder!\n If
                    you want to proceed with this option, just CONFIGURE the project once again" )
                set( SAME_BUILD_AND_SOURCE_FOLDER_WARNING_ALREADY_ISSUED TRUE )
        endif()
endif()

################################################################################

#math(EXPR EASY3D_OS_BITS "8 * ${CMAKE_SIZEOF_VOID_P}")
#message(STATUS "Easy3d: I'am woking on ${EASY3D_OS_BITS}-bit ${CMAKE_SYSTEM_NAME}")
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "Easy3D: I'am working on a 64-bit ${CMAKE_SYSTEM_NAME}")
else()
    message(STATUS "Easy3D: I'am working on a 32-bit ${CMAKE_SYSTEM_NAME}")
endif()

################################################################################

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

################################################################################

# Detects whether this is a top-level project
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	set(EASY3D_TOPLEVEL_PROJECT ON)
else()
	set(EASY3D_TOPLEVEL_PROJECT OFF)
endif()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################

# Build Qt-based examples/applications
option(EASY3D_ENABLE_QT             "Build Qt-based examples/applications"  OFF)
# Build tutorials
option(EASY3D_BUILD_TUTORIALS       "Build Easy3D Tutorials"                ON)
# Build documentation
option(EASY3D_BUILD_DOCUMENTATION   "Build Easy3D Documentation"            OFF)
# Build sandbox
option(EASY3D_BUILD_SANDBOX         "Build sandbox (for development)"       OFF)

################################################################################

find_package(CGAL QUIET)
if (CGAL_FOUND)
    set(EASY3D_HAS_CGAL TRUE)
    add_definitions(-DHAS_CGAL)
else ()
    set(EASY3D_HAS_CGAL FALSE)
    message(WARNING "Some advanced features require CGAL but CGAL was not found. Easy3D will be built without these features. "
            "You can set 'CGAL_DIR' to the directory containing 'CGALConfig.cmake' or 'cgal-config.cmake' to help find CGAL.")
endif ()

################################################################################

### Configuration
set(EASY3D_ROOT         ${CMAKE_CURRENT_LIST_DIR})
set(EASY3D_THIRD_PARTY  ${EASY3D_ROOT}/3rd_party)

set(EASY3D_INCLUDE_DIR  ${EASY3D_ROOT} ${CMAKE_CURRENT_BINARY_DIR})
set(EASY3D_SOURCE_DIR   ${EASY3D_ROOT})

### conditionally compile certain modules depending on libraries found on the system
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

################################################################################

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY  ${CMAKE_BINARY_DIR}/lib)

################################################################################

add_subdirectory(3rd_party)

add_subdirectory(easy3d)

if(EASY3D_BUILD_TUTORIALS)
    add_subdirectory(tutorials)
endif()

if(EASY3D_BUILD_SANDBOX)
    add_subdirectory(sandbox)
endif()

add_subdirectory(applications)

################################################################################

if(EASY3D_BUILD_DOCUMENTATION)
    # generation of Easy3D documentation requires doxygen
    find_package (Doxygen)
    if(${DOXYGEN_FOUND})
        message(STATUS "Found Doxygen: " ${DOXYGEN_EXECUTABLE})
        # configures Doxygen
        configure_file (doc/Doxyfile.in Doxyfile @ONLY)
        add_custom_target (doc ALL ${DOXYGEN_EXECUTABLE} Doxyfile
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
mark_as_advanced(FORCE BUILD_TESTING)

################################################################################

## Resources
# Copy resources dirs into our shadow build directory
file( COPY ${CMAKE_CURRENT_SOURCE_DIR}/resources DESTINATION ${CMAKE_BINARY_DIR} )
