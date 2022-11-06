get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME)

macro(require_cpp11)
        if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.0)
                # CMake 3.1 has built-in CXX standard checks.
                set(CMAKE_CXX_STANDARD 11)
                set(CMAKE_CXX_STANDARD_REQUIRED ON)
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
add_library(3rd_${module} STATIC easylogging++.h easylogging++.cc)
set_property(TARGET 3rd_${module} PROPERTY POSITION_INDEPENDENT_CODE ON)
if (MSVC)
	target_compile_definitions(3rd_${module} PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE)
endif()

# EasyLogging++: request all features
target_compile_definitions(
        3rd_${module}
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
            3rd_${module}
            PUBLIC
            HAVE_EXECINFO
    )
endif ()

################################################################################
# get a clean windows
if (WIN32 OR MSVC)
    target_compile_definitions(3rd_${module} PUBLIC NOMINMAX)
endif()

set_target_properties(3rd_${module} PROPERTIES FOLDER "3rd_party")


set_target_properties(3rd_${module} PROPERTIES PUBLIC_HEADER "easylogging++.h")
install(TARGETS 3rd_${module}
        # IMPORTANT: Add the library to the "export-set"
        EXPORT Easy3DTargets
        LIBRARY DESTINATION "${Easy3D_INSTALL_LIB_DIR}" COMPONENT lib
        PUBLIC_HEADER DESTINATION "${Easy3D_INSTALL_INCLUDE_DIR}/3rd_party/easyloggingpp"
        COMPONENT dev)