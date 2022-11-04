set(module util)

find_package(Threads REQUIRED)

set(private_dependencies 3rd_backward)
set(public_dependencies 3rd_easyloggingpp Threads::Threads)

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
target_compile_definitions(easy3d_${module} PUBLIC
        "$<BUILD_INTERFACE:Easy3D_RESOURCES_DIR=\"${Easy3D_ROOT}/resources\">"
        "$<INSTALL_INTERFACE:Easy3D_RESOURCES_DIR=\"${Easy3D_INSTALL_RESOURCES_DIR}\">"
        )

install_module(${module})