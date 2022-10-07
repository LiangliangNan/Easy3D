cmake_minimum_required(VERSION 3.12)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

include(../../cmake/FindFFMPEG.cmake)
if (NOT FFMPEG_FOUND)
    message(FATAL_ERROR "${PROJECT_NAME} requires FFMPEG to generate video files but FFMPEG was not found. "
            "Without FFMPEG, animations can only be recorded as a sequence of images. "
            "To use FFMPEG, make sure FFMPEG exists on your system (install/build FFMPEG if necessary). "
            "Modify the search paths in 'FindFFMPEG.cmake' for 'include' and 'lib' directories, and then run CMake."
            )
	return()
endif ()

add_executable(${PROJECT_NAME} 
		video_encoder.h
		video_encoder.cpp
		main.cpp
)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "applications")

target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} PRIVATE ${FFMPEG_LIBRARIES} easy3d_fileio)

if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_DEPRECATE)
endif ()