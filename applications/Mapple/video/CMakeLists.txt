cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


include( ../../../cmake/FFmpegSupport.cmake )

add_library( ${PROJECT_NAME} SHARED )

set_target_properties( ${PROJECT_NAME} PROPERTIES
	AUTOMOC OFF
	AUTORCC OFF
	AUTOUIC OFF
)

target_sources( ${PROJECT_NAME}
	PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/QVideoEncoder.h
		${CMAKE_CURRENT_LIST_DIR}/QVideoEncoder.cpp
)

target_include_directories( ${PROJECT_NAME}
	PUBLIC
		${CMAKE_CURRENT_SOURCE_DIR}
		${FFMPEG_INCLUDE_DIR}
)

target_link_libraries( ${PROJECT_NAME}
	Qt5::Gui
)

# FFmpeg support
target_link_ffmpeg( ${PROJECT_NAME} )
export_ffmpeg_dlls( ${CLOUDCOMPARE_DEST_FOLDER} )


find_library(CORE_FOUNDATION Foundation)
find_library(APPLICATION_SERVICES ApplicationServices)
find_library(CORE_SERVICES CoreServices)
set(EXTRA_LIBS ${CORE_FOUNDATION} ${APPLICATION_SERVICES} ${CORE_SERVICES})
target_link_libraries(${PROJECT_NAME} ${EXTRA_LIBS})