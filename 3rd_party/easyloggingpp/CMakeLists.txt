get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME)

macro(require_cpp11)
    if (${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION} GREATER 3.0)
        # CMake 3.1 has built-in CXX standard checks.
        set(CMAKE_CXX_STANDARD 11)
        set(CMAKE_CXX_STANDARD_REQUIRED ON)
    else ()
        if (CMAKE_CXX_COMPILER_ID MATCHES "GCC")
            message("-- GNU CXX (-std=c++11)")
            list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
        elseif (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
            message("-- CLang CXX (-std=c++11)")
            list(APPEND CMAKE_CXX_FLAGS "-std=c++11")
        else ()
            message("-- Easylogging++ requires C++11. Your compiler does not support it.")
        endif ()
    endif ()
endmacro()

set(ELPP_MAJOR_VERSION "9")
set(ELPP_MINOR_VERSION "96")
set(ELPP_PATCH_VERSION "7")
set(ELPP_VERSION_STRING "${ELPP_MAJOR_VERSION}.${ELPP_MINOR_VERSION}.${ELPP_PATCH_VERSION}")


require_cpp11()

add_3rdparty_module(3rd_${module} "easylogging++.h" "easylogging++.cc")

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

if (NOT ${Easy3D_BUILD_SHARED_LIBS})
    install(TARGETS 3rd_${module}
            # IMPORTANT: Add the library to the "export-set"
            EXPORT Easy3DTargets
            LIBRARY DESTINATION "${Easy3D_INSTALL_LIB_DIR}" COMPONENT lib
            COMPONENT dev)
endif ()

## Install easylogging++.h to the installation directory
install(FILES "${Easy3D_THIRD_PARTY}/easyloggingpp/easylogging++.h"
        DESTINATION "${Easy3D_INSTALL_INCLUDE_DIR}/3rd_party/easyloggingpp")