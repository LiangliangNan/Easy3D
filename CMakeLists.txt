cmake_minimum_required(VERSION 3.12)

################################################################################

set(Easy3D_MAJOR_VERSION 2)
set(Easy3D_MINOR_VERSION 5)
set(Easy3D_PATCH_VERSION 2)
set(Easy3D_VERSION ${Easy3D_MAJOR_VERSION}.${Easy3D_MINOR_VERSION}.${Easy3D_PATCH_VERSION})

project(Easy3D VERSION ${Easy3D_VERSION})

################################################################################

# specify the C++ standard
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
message("-- Setting C++11")

################################################################################

# Offer the user the choice of overriding the installation directories
# Let's also add the version to the path to allow multiple versions of Easy3D in the same directory.

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    if (WIN32 OR MSVC)  # windows
        set(CMAKE_INSTALL_PREFIX "C:/Program Files/Easy3D/${Easy3D_VERSION}" CACHE PATH "Installation directory" FORCE)
    elseif (APPLE)      # macOS
        set(CMAKE_INSTALL_PREFIX "/usr/local/Cellar/easy3d/${Easy3D_VERSION}" CACHE PATH "Installation directory" FORCE)
    else ()             # Linux: different from tradition, let's keep everything together
        set(CMAKE_INSTALL_PREFIX "/usr/local/easy3d-${Easy3D_VERSION}" CACHE PATH "Installation directory" FORCE)
    endif ()
endif ()

if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS support" FORCE)
endif ()

set(Easy3D_INSTALL_LIB_DIR ${CMAKE_INSTALL_PREFIX}/lib CACHE PATH "Installation directory for libraries")
set(Easy3D_INSTALL_BIN_DIR ${CMAKE_INSTALL_PREFIX}/bin CACHE PATH "Installation directory for executables")
set(Easy3D_INSTALL_INCLUDE_DIR ${CMAKE_INSTALL_PREFIX}/include CACHE PATH "Installation directory for header files")
set(Easy3D_INSTALL_CMAKE_DIR ${CMAKE_INSTALL_PREFIX}/lib/CMake/Easy3D CACHE PATH "Installation directory for CMake files")
set(Easy3D_INSTALL_RESOURCE_DIR ${CMAKE_INSTALL_PREFIX}/resources CACHE PATH "Installation directory for CMake files")

# The include directory of the exported header files
set(Easy3D_EXPORT_INCLUDE_DIR "${Easy3D_BINARY_DIR}/include")

# Setting RPATH
# See https://cmake.org/Wiki/CMake_RPATH_handling
set(CMAKE_INSTALL_RPATH ${Easy3D_INSTALL_LIB_DIR} CACHE PATH "Installation runtime directory" FORCE)

################################################################################

### Configuration
set(Easy3D_ROOT ${CMAKE_CURRENT_LIST_DIR})
set(Easy3D_THIRD_PARTY ${Easy3D_ROOT}/3rd_party)

set(Easy3D_INCLUDE_DIR ${Easy3D_ROOT})
set(Easy3D_SOURCE_DIR ${Easy3D_ROOT})
set(Easy3D_BINARY_DIR ${CMAKE_BINARY_DIR})

### conditionally compile certain modules depending on libraries found on the system
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)

### Where to put the libraries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${Easy3D_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${Easy3D_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${Easy3D_BINARY_DIR}/lib)

################################################################################

# Build shared libraries
option(Easy3D_BUILD_SHARED_LIBS "Build shared libaries"         ON)
# Build tutorials
option(Easy3D_BUILD_TUTORIALS "Build Easy3D tutorials"          ON)
# Build documentation
option(Easy3D_BUILD_DOCUMENTATION "Build Easy3D documentation"  OFF)
# Build tests
option(Easy3D_BUILD_TESTS "Build Easy3D tests programs"         OFF)
# Build advanced features that require CGAL (>= v5.1)
option(Easy3D_ENABLE_CGAL "Build advanced features that require CGAL (>= v5.1)"              OFF)
# Build advanced examples/applications that require Qt5 (>= v5.6)
option(Easy3D_ENABLE_QT "Build advanced examples/applications that require Qt5 (>= v5.6)"    OFF)
# Build the video encoding module that requires ffmpeg
option(Easy3D_ENABLE_FFMPEG "Build the video encoding module that requires ffmpeg (>= v3.4)" OFF)

################################################################################

# Discourage users to build Easy3D directly in its root directory
if (${CMAKE_CURRENT_BINARY_DIR} STREQUAL ${CMAKE_CURRENT_SOURCE_DIR})
    if (NOT SAME_BUILD_AND_SOURCE_FOLDER_WARNING_ALREADY_ISSUED)
        message(WARNING "It is NOT advised to build Easy3D directly in its root directory!")
        set(SAME_BUILD_AND_SOURCE_FOLDER_WARNING_ALREADY_ISSUED TRUE)
    endif ()
endif ()

# Detect whether this is a top-level project
if (CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    set(Easy3D_TOPLEVEL_PROJECT ON)
else ()
    set(Easy3D_TOPLEVEL_PROJECT OFF)
endif ()

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

################################################################################

include(GenerateExportHeader)
if (Easy3D_BUILD_SHARED_LIBS)
    set(Easy3D_LIB_TYPE SHARED)
    #Ref: Create dlls on Windows without declspec() using new CMake export all feature
    #     https://cmake.org/cmake/help/v3.4/prop_tgt/WINDOWS_EXPORT_ALL_SYMBOLS.html#prop_tgt:WINDOWS_EXPORT_ALL_SYMBOLS
    #     https://www.kitware.com/create-dlls-on-windows-without-declspec-using-new-cmake-export-all-feature/
    if (WIN32 OR MSVC)
        set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS TRUE)
    endif ()
else ()
    set(Easy3D_LIB_TYPE STATIC)
endif ()

################################################################################

if (Easy3D_ENABLE_CGAL)
    find_package(CGAL QUIET)
    if (CGAL_FOUND)
        if (CGAL_VERSION VERSION_GREATER_EQUAL "5.1")
            set(Easy3D_HAS_CGAL TRUE)
            mark_as_advanced(BUILD_TESTING
                    CGAL_CTEST_DISPLAY_MEM_AND_TIME
                    CGAL_DEV_MODE
                    CGAL_TEST_DRAW_FUNCTIONS
                    CGAL_WITH_DRAW_FUNCTIONS
                    CGAL_WITH_GMPXX)
            message(STATUS "Found CGAL v${CGAL_VERSION}. Include dirs: ${CGAL_INCLUDE_DIRS}")
        else ()
            set(Easy3D_HAS_CGAL FALSE)
            message(FATAL_ERROR "You have requested CGAL support and CGAL v${CGAL_VERSION} is found. However, Easy3D "
                    "requires at least v5.1. Please install CGAL v5.1 or above. In case you have multiple versions "
                    "of CGAL on your platform, provide the path of a suitable one to the CMake variable `CGAL_DIR`. "
                    "Please note that only the following surface mesh algorithms require CGAL (v5.1 or later): \n"
                    "  - reorientation; \n"
                    "  - detecting/resolving duplicate vertices/faces; \n"
                    "  - detecting/resolving self-intersection; \n"
                    "  - clipping/splitting/slicing. \n"
                    "If you don't need these algorithms, disable CGAL support by setting `Easy3D_ENABLE_CGAL` to `OFF`. "
                    "You will still be able to use all other features provided by Easy3D without CGAL.")
            return()
        endif ()
    else ()
        set(Easy3D_HAS_CGAL FALSE)
        message(FATAL_ERROR "You have requested CGAL support but CGAL (v5.1 or later required) was not found. You can "
                "set 'CGAL_DIR' to the directory containing `CGALConfig.cmake` or `cgal-config.cmake` to help CMake find CGAL. "
                "Please note that only the following surface mesh algorithms require CGAL (v5.1 or later): \n"
                "  - reorientation; \n"
                "  - detecting/resolving duplicate vertices/faces; \n"
                "  - detecting/resolving self-intersection; \n"
                "  - clipping/splitting/slicing. \n"
                "If you don't need these algorithms, disable CGAL support by setting `Easy3D_ENABLE_CGAL` to `OFF`. "
                "You will still be able to use all other features provided by Easy3D without CGAL.")
        return()
    endif ()
endif ()

if (Easy3D_ENABLE_FFMPEG)
    include(./cmake/FindFFMPEG.cmake)
    if (FFMPEG_FOUND)
        set(Easy3D_HAS_FFMPEG TRUE)
    else ()
        set(Easy3D_HAS_FFMPEG FALSE)
        message(WARNING "You have requested ffmpeg support but ffmpeg was not found (v3.4 or later required). "
                "The ffmpeg support allows encoding/recording animation into a video file. You can ignore "
                "this warning if you don't need this feature. \n"
                "To enable video encoding, make sure ffmpeg exists on your system. Modify the search paths of "
                "'include' and 'lib' in 'Easy3D/cmake/FindFFMPEG.cmake', and then run CMake. \n"
                "How to install ffmpeg (v3.4 or later required)? \n"
                "    On Windows, you can download ffmpeg from:  \n"
                "       https://ffmpeg.org/download.html \n"
                "    On macOS, use the following command to install ffmpeg: \n"
                "       brew install ffmpeg \n"
                "    On Linux, one has to install the libraries using: \n"
                "       sudo apt-get install libavcodec-dev \n"
                "       sudo apt-get install libavformat-dev \n"
                "       sudo apt-get install libswscale-dev \n"
                "       sudo apt-get install libavutil-dev \n"
                )
    endif ()
endif ()

################################################################################

# Make relative paths absolute (needed later on)
foreach (path LIB BIN INCLUDE CMAKE)
    set(var Easy3D_INSTALL_${path}_DIR)
    if (NOT IS_ABSOLUTE "${${var}}")
        set(${var} "${Easy3D_INSTALL_DIR}/${${var}}")
    endif ()
endforeach ()

function(add_module module headers sources private_dependencies public_dependencies)
    add_library(easy3d_${module} ${Easy3D_LIB_TYPE} ${headers} ${sources})
    #Ref: Create dlls on Windows without declspec() using new CMake export all feature
    #     https://www.kitware.com/create-dlls-on-windows-without-declspec-using-new-cmake-export-all-feature/
    generate_export_header(easy3d_${module} EXPORT_FILE_NAME "${Easy3D_EXPORT_INCLUDE_DIR}/easy3d/${module}/export.h")
    target_include_directories(easy3d_${module} PUBLIC
            "$<BUILD_INTERFACE:${Easy3D_INCLUDE_DIR}>"
            "$<BUILD_INTERFACE:${Easy3D_EXPORT_INCLUDE_DIR}>"
            "$<INSTALL_INTERFACE:${Easy3D_INSTALL_INCLUDE_DIR}>"
            )
    target_link_libraries(easy3d_${module}
            PRIVATE ${private_dependencies}
            PUBLIC ${public_dependencies}
            )
    set_target_properties(easy3d_${module} PROPERTIES
            FOLDER "easy3d"
            SOVERSION ${Easy3D_MAJOR_VERSION}
            VERSION ${Easy3D_VERSION}
            )
    if (WIN32 OR MSVC)
        target_compile_definitions(easy3d_${module} PUBLIC NOMINMAX WIN32_LEAN_AND_MEAN VC_EXTRALEAN _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE)  # get a clean windows
    endif ()

    # Alias target (recommended by policy CMP0028) and it looks nicer
    message(STATUS "Adding target: easy3d::${module} (easy3d_${module})")
    add_library(easy3d::${module} ALIAS easy3d_${module})
    # Export as easy3d::${MODULE_NAME}
    set_property(TARGET easy3d_${module} PROPERTY EXPORT_NAME easy3d::${module})
endfunction()

function(install_module module)
    set_target_properties(easy3d_${module} PROPERTIES PUBLIC_HEADER "${${module}_headers}")
    install(TARGETS easy3d_${module}
            # IMPORTANT: Add the library to the "export-set"
            EXPORT Easy3DTargets
            RUNTIME DESTINATION "${Easy3D_INSTALL_BIN_DIR}" COMPONENT bin
            LIBRARY DESTINATION "${Easy3D_INSTALL_LIB_DIR}" COMPONENT lib
            ARCHIVE DESTINATION "${Easy3D_INSTALL_LIB_DIR}" COMPONENT lib
            PUBLIC_HEADER DESTINATION "${Easy3D_INSTALL_INCLUDE_DIR}/easy3d/${module}"
            COMPONENT dev)
endfunction()

################################################################################

add_subdirectory(3rd_party)

add_subdirectory(easy3d)

if (Easy3D_BUILD_TUTORIALS)
    add_subdirectory(tutorials)
endif ()

if (Easy3D_BUILD_TESTS)
    add_subdirectory(tests)
endif ()

add_subdirectory(applications)

################################################################################

if (Easy3D_BUILD_DOCUMENTATION)
    # generation of Easy3D documentation requires doxygen
    find_package(Doxygen)
    if (${DOXYGEN_FOUND})
        message(STATUS "Found Doxygen: " ${DOXYGEN_EXECUTABLE})
        # configures Doxygen
        configure_file(docs/Doxyfile.in Doxyfile @ONLY)
        add_custom_target(Documentation ALL ${DOXYGEN_EXECUTABLE} Doxyfile
                WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
                COMMENT "Generating Doxygen documentation")
    else ()
        message(WARNING "Building documentation requires Doxygen but Doxygen was not found. Building documentation is "
                "thus disabled. You can set 'DOXYGEN_EXECUTABLE' to the directory containing the Doxygen executable "
                "if Doxygen already exists (otherwise install Doxygen first).")
    endif ()
endif ()

################################################################################

install(DIRECTORY
        "${Easy3D_ROOT}/resources/fonts"
        "${Easy3D_ROOT}/resources/shaders"
        "${Easy3D_ROOT}/resources/colormaps"
        "${Easy3D_ROOT}/resources/textures"
        DESTINATION
        "${Easy3D_INSTALL_RESOURCE_DIR}")

install(FILES
        "${Easy3D_ROOT}/ReadMe.md"
        "${Easy3D_ROOT}/LICENSE"
        DESTINATION
        "${CMAKE_INSTALL_PREFIX}"
        )

install(DIRECTORY
        "${Easy3D_EXPORT_INCLUDE_DIR}/easy3d"
        DESTINATION
        "${Easy3D_INSTALL_INCLUDE_DIR}")

################################################################################

### Summary
message(STATUS "****************************************************************************")
message(STATUS "    Package       : Easy3D")
message(STATUS "    Version       : ${Easy3D_VERSION}")

message(STATUS "    Generator     : ${CMAKE_GENERATOR}")
if (CMAKE_CONFIGURATION_TYPES)
    message(STATUS "    Configuration : ${CMAKE_CONFIGURATION_TYPES}")
elseif (CMAKE_BUILD_TYPE)
    message(STATUS "    Build type    : ${CMAKE_BUILD_TYPE}")

endif ()

if (CMAKE_SIZEOF_VOID_P EQUAL 8)
    message(STATUS "    System        : 64-bit ${CMAKE_SYSTEM_NAME}")
elseif (CMAKE_SIZEOF_VOID_P EQUAL 4)
    message(STATUS "    System        : 32-bit ${CMAKE_SYSTEM_NAME}")
    message(FATAL_ERROR "32-bit not supported! You should specify CMake option '-DCMAKE_GENERATOR_PLATFORM=x64'")
endif ()
if (APPLE)
    message(STATUS "    Minimum macOS support : ${CMAKE_OSX_DEPLOYMENT_TARGET}")
    mark_as_advanced(CMAKE_OSX_ARCHITECTURES)
endif ()

message(STATUS "----------------------------------------------------------------------------")

message(STATUS "    Build shared libraries :  ${Easy3D_BUILD_SHARED_LIBS}")
message(STATUS "    Build tutorials        :  ${Easy3D_BUILD_TUTORIALS}")
message(STATUS "    Build documentation    :  ${Easy3D_BUILD_DOCUMENTATION}")
message(STATUS "    Build tests            :  ${Easy3D_BUILD_TESTS}")
message(STATUS "    With CGAL (>= v5.1)    :  ${Easy3D_ENABLE_CGAL}")
message(STATUS "    With Qt5 (>= v5.6)     :  ${Easy3D_ENABLE_QT}")
message(STATUS "    With ffmpeg (>= v3.4)  :  ${Easy3D_ENABLE_FFMPEG}")

message(STATUS "----------------------------------------------------------------------------")

message(STATUS "    Build directory        :  ${Easy3D_BINARY_DIR}")
message(STATUS "    Installation directory :  ${CMAKE_INSTALL_PREFIX}")

message(STATUS "****************************************************************************")


