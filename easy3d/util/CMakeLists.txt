set(module util)

find_package(Threads REQUIRED)
set(private_dependencies 3rd_backward)
set(public_dependencies Threads::Threads)

set(${module}_headers
        console_style.h
        dialogs.h
        file_system.h
        line_stream.h
        logging.h
        progress.h
        resources.h
        stack_tracer.h
        stop_watch.h
        string.h
        timer.h
        tokenizer.h
        )

set(${module}_sources
        console_style.cpp
        dialogs.cpp
        file_system.cpp
        logging.cpp
        progress.cpp
        resources.cpp
        stack_tracer.cpp
        stop_watch.cpp
        string.cpp
        )

add_module(${module} "${${module}_headers}" "${${module}_sources}" "${private_dependencies}" "${public_dependencies}")

# The resources directory
target_compile_definitions(easy3d_${module} 
        PUBLIC
        ELPP_FEATURE_ALL
        ELPP_STL_LOGGING
        ELPP_THREAD_SAFE
        ELPP_NO_DEFAULT_LOG_FILE
        ELPP_DISABLE_DEFAULT_CRASH_HANDLING
        "$<BUILD_INTERFACE:Easy3D_RESOURCE_DIR=\"${Easy3D_ROOT}/resources\">"
        "$<INSTALL_INTERFACE:Easy3D_RESOURCE_DIR=\"${Easy3D_INSTALL_RESOURCE_DIR}\">"
        )

if (Easy3D_BUILD_SHARED_LIBS)
    target_compile_definitions(easy3d_${module}
            PUBLIC ELPP_AS_DLL
            PRIVATE ELPP_EXPORT_SYMBOLS)
endif ()

install_module(${module})
# Install easylogging++.h to the installation directory
install(FILES "${Easy3D_THIRD_PARTY}/easyloggingpp/easylogging++.h"
        DESTINATION "${Easy3D_INSTALL_INCLUDE_DIR}/3rd_party/easyloggingpp")