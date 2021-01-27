cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(${PROJECT_NAME}_HEADERS
        )

set(${PROJECT_NAME}_SOURCES
        main.cpp
        )

add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES})

if(WIN32 OR MSVC)
    add_subdirectory(windeployqt)
    target_link_libraries(${PROJECT_NAME} windeployqt)
elseif(APPLE)
    add_subdirectory(macdeployqt)
    target_link_libraries(${PROJECT_NAME} macdeployqt)
else()
    add_subdirectory(linuxdeployqt)
    target_link_libraries(${PROJECT_NAME} linuxdeployqt)
endif()