cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


include(../../../cmake/UseQt5.cmake)
if (NOT QT5_FOUND)
    message(WARNING "${PROJECT_NAME} requires Qt but Qt was not found. You can set 'Qt5_DIR' to the "
            "directory containing 'Qt5Config.cmake' or 'qt5-config.cmake'. "
            "Optionally, you can set the Qt5 root directory 'QT5_ROOT_PATH' to the directory "
            "containing the 'bin' folder.")
    return()
endif ()


include(../../../cmake/FindFFMPEG.cmake)
if (NOT FFMPEG_FOUND)
    message(WARNING "${PROJECT_NAME} requires FFMPEG to generate video files but FFMPEG was not found. "
            "Without FFMPEG, animations can only be recorded as a sequence of images. "
            "To use FFMPEG, make sure FFMPEG exists on your system (install/build FFMPEG if necessary). "
            "Then modify the search paths in 'FindFFMPEG.cmake' for 'include' and 'lib' directories."
            )
    return()
endif ()


set(${PROJECT_NAME}_HEADERS
        QVideoEncoder.h
        )

set(${PROJECT_NAME}_SOURCES
        QVideoEncoder.cpp
        )

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "applications/Mapple")

target_compile_definitions(${PROJECT_NAME} PRIVATE  ${FFMPEG_DEFINITIONS})


set_target_properties(${PROJECT_NAME} PROPERTIES
        AUTOMOC OFF
        AUTORCC OFF
        AUTOUIC OFF
        )


target_include_directories(${PROJECT_NAME}
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${FFMPEG_INCLUDE_DIRS}
        )

target_link_libraries(${PROJECT_NAME}
        PUBLIC
        Qt5::Gui
        ${FFMPEG_LIBRARIES}
        )