cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(${PROJECT_NAME}_HEADERS
        chrono_watch.h
        dialogs.h
        easylogging++.h
        file_system.h
        line_stream.h
        logging.h
        progress.h
        stack_tracer.h
        stop_watch.h
        string.h
        timer.h
        tokenizer.h
        )

set(${PROJECT_NAME}_SOURCES
        chrono_watch.cpp
        dialogs.cpp
        easylogging++.cc
        file_system.cpp
        logging.cpp
        progress.cpp
        stack_tracer.cpp
        stop_watch.cpp
        string.cpp
        )

	
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS} )

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "easy3d")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})

find_package(Threads REQUIRED)
target_link_libraries(${PROJECT_NAME} Threads::Threads backward)

if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_WARNINGS _CRT_SECURE_NO_DEPRECATE)
endif()