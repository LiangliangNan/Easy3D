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


find_package(FFMPEG QUIET)
if (NOT FFMPEG_FOUND)
    message(WARNING "${PROJECT_NAME} requires FFMPEG to generate video files for animations but FFMPEG was not found. "
            "Without FFMPEG, animations can only be recorded as a sequence of images. "
            "To use FFMPEG, first set 'FFMPEG_DIR' to 'cmake/ffmpeg-config.cmake'. Then modify the 'ffmpeg_path' "
            "variable in 'cmake/ffmpeg-config.cmake' to point to the folder in which FFMPEG was installed."
            )
else ()
    add_definitions(-DHAS_FFMPEG)
    message(STATUS "   FFMPEG_INCLUDE_DIRS ${FFMPEG_INCLUDE_DIRS}")
    message(STATUS "   FFMPEG_LIBRARIES ${FFMPEG_LIBRARIES}")
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
		swscale
        )