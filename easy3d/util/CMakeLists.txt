set(module util)

find_package(Threads REQUIRED)
set(private_dependencies)
set(public_dependencies Threads::Threads)

set(${module}_headers
        console_style.h
        dialog.h
        file_system.h
        initializer.h
        line_stream.h
        logging.h
        progress.h
        resource.h
        setting.h
        signal.h
        stop_watch.h
        string.h
        timer.h
        tokenizer.h
        version.h
        )

set(${module}_sources
        console_style.cpp
        dialog.cpp
        file_system.cpp
        initializer.cpp
        logging.cpp
        progress.cpp
        resource.cpp
        setting.cpp
        stop_watch.cpp
        string.cpp
        version.cpp
        )

add_module(${module} "${${module}_headers}" "${${module}_sources}" "${private_dependencies}" "${public_dependencies}")

# The resources directory
target_compile_definitions(easy3d_${module}
        PUBLIC
        "Easy3D_VERSION_MAJOR=${Easy3D_VERSION_MAJOR}"
        "Easy3D_VERSION_MINOR=${Easy3D_VERSION_MINOR}"
        "Easy3D_VERSION_PATCH=${Easy3D_VERSION_PATCH}"
        "Easy3D_VERSION_STRING=\"${Easy3D_VERSION_STRING}\""
        "Easy3D_VERSION_NUMBER=${Easy3D_VERSION_NUMBER}"
        ELPP_FEATURE_ALL
        ELPP_STL_LOGGING
        ELPP_THREAD_SAFE
        ELPP_NO_DEFAULT_LOG_FILE
        ELPP_DISABLE_DEFAULT_CRASH_HANDLING
        "$<BUILD_INTERFACE:Easy3D_RESOURCE_DIR=\"${Easy3D_ROOT}/resources\">"
        "$<INSTALL_INTERFACE:Easy3D_RESOURCE_DIR=\"${Easy3D_INSTALL_RESOURCE_DIR}\">"
        )

if (Easy3D_BUILD_SHARED_LIBS)
# We do want to export the symbols in easyloggingpp
    target_compile_definitions(easy3d_${module}
            PUBLIC ELPP_AS_DLL
            PRIVATE ELPP_EXPORT_SYMBOLS)
endif ()

### update EASY3D_VERSION_NR and EASY3D_RELEASE_DATE
file(READ "${CMAKE_CURRENT_LIST_DIR}/version.h" file_content)
string(REGEX REPLACE "EASY3D_VERSION_NR 10[0-9]0[0-9]0[0-9]" "EASY3D_VERSION_NR ${Easy3D_VERSION_NUMBER}" file_content "${file_content}")
string(TIMESTAMP TODAY "%Y%m%d")
string(REGEX REPLACE "EASY3D_RELEASE_DATE [0-9][0-9][0-9][0-9][0-9][0-9][0-9][0-9]" "EASY3D_RELEASE_DATE ${TODAY}" file_content "${file_content}")
file(WRITE ${CMAKE_CURRENT_LIST_DIR}/version.h "${file_content}")

install_module(${module})
# Install easylogging++.h to the installation directory
install(FILES "${Easy3D_THIRD_PARTY}/easyloggingpp/easylogging++.h"
        DESTINATION "${Easy3D_INSTALL_INCLUDE_DIR}/3rd_party/easyloggingpp")