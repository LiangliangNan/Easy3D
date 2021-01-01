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


find_package(FFMPEG)
if (NOT FFMPEG_FOUND)
    message(WARNING "${PROJECT_NAME} requires FFMPEG but FFMPEG was not found. You can set 'FFMPEG_DIR' to the "
            "directory containing 'FFMPEGConfig.cmake' or 'ffmpeg-config.cmake'")
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

add_library(${PROJECT_NAME} SHARED ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})

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
        Qt5::Gui
        ${FFMPEG_LIBRARIES}
		swscale
        )

