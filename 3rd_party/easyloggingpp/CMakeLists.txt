cmake_minimum_required(VERSION 3.1)

project(Easyloggingpp CXX)

macro(require_cpp11)
        if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.0)
                # CMake 3.1 has built-in CXX standard checks.
                message("-- Setting C++11")
                set(CMAKE_CXX_STANDARD 11)
                set(CMAKE_CXX_STANDARD_REQUIRED on)
        else()
                if (CMAKE_CXX_COMPILER_ID MATCHES "GCC")
                    message ("-- GNU CXX (-std=c++11)")
                    list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
                elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
                    message ("-- CLang CXX (-std=c++11)")
                    list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
                else()
                    message ("-- Easylogging++ requires C++11. Your compiler does not support it.")
                endif()
        endif()
endmacro()

set(ELPP_MAJOR_VERSION "9")
set(ELPP_MINOR_VERSION "96")
set(ELPP_PATCH_VERSION "7")
set(ELPP_VERSION_STRING "${ELPP_MAJOR_VERSION}.${ELPP_MINOR_VERSION}.${ELPP_PATCH_VERSION}")



require_cpp11()
add_library(3rd_easyloggingpp STATIC src/easylogging++.cc)
set_property(TARGET 3rd_easyloggingpp PROPERTY POSITION_INDEPENDENT_CODE ON)


# EasyLogging++: request all features
target_compile_definitions(
        3rd_easyloggingpp
        PUBLIC
        ELPP_FEATURE_ALL
        ELPP_STL_LOGGING
        ELPP_THREAD_SAFE
        ELPP_NO_DEFAULT_LOG_FILE
        ELPP_DISABLE_DEFAULT_CRASH_HANDLING
)
# EasyLogging++: for stack trace
include(CheckIncludeFileCXX)
check_include_file_cxx("execinfo.h" HAVE_EXECINFO)
if (HAVE_EXECINFO)
    target_compile_definitions(
            3rd_easyloggingpp
            PUBLIC
            HAVE_EXECINFO
    )
endif ()

################################################################################
# get a clean windows
if (WIN32 OR MSVC)
    target_compile_definitions(3rd_easyloggingpp PUBLIC NOMINMAX)
endif()

set_target_properties(3rd_easyloggingpp PROPERTIES FOLDER "3rd_party")