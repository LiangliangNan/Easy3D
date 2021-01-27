cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


# find Qt5Core
find_package(Qt5 COMPONENTS Core)

if (NOT Qt5Core_FOUND)
    message(WARNING "${PROJECT_NAME} requires Qt but Qt was not found. You can set 'Qt5_DIR' to the "
            "directory containing 'Qt5Config.cmake' or 'qt5-config.cmake'. "
            "Optionally, you can set the Qt5 root directory 'QT5_ROOT_PATH' to the directory "
            "containing the 'bin' folder.")
    return()
endif ()

set(${PROJECT_NAME}_HEADERS
        src/shared.h
        )

set(${PROJECT_NAME}_SOURCES
        src/shared.cpp
        src/main.cpp
        )

if (APPLE)
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES})

    # The following define makes your compiler emit warnings if you use
    # any feature of Qt which has been marked as deprecated (the exact warnings
    # depend on your compiler). Please consult the documentation of the
    # deprecated API in order to know how to port your code away from it.
#    target_compile_definitions(${PROJECT_NAME} PRIVATE QT_DEPRECATED_WARNINGS)

    target_link_libraries(${PROJECT_NAME} Qt5::Core)

endif ()