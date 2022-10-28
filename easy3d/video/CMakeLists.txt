cmake_minimum_required(VERSION 3.12)

get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(PROJECT_NAME "easy3d_${MODULE_NAME}")
project(${PROJECT_NAME})

set(${PROJECT_NAME}_HEADERS
        video_encoder.h
        )

set(${PROJECT_NAME}_SOURCES
        video_encoder.cpp
        )

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "easy3d")

target_include_directories(${PROJECT_NAME} PUBLIC ${Easy3D_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${FFMPEG_INCLUDE_DIRS})

target_link_libraries(${PROJECT_NAME} PUBLIC easy3d_util ${FFMPEG_LIBRARIES})

# Alias target (recommended by policy CMP0028) and it looks nicer
message(STATUS "Adding target: easy3d::${MODULE_NAME} (${PROJECT_NAME})")
add_library(easy3d::${MODULE_NAME} ALIAS ${PROJECT_NAME})
# Export as easy3d::${MODULE_NAME}
set_property(TARGET ${PROJECT_NAME} PROPERTY EXPORT_NAME easy3d::${MODULE_NAME})