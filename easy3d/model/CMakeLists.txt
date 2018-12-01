cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


file(GLOB SOURCES ./*.cpp)
file(GLOB HEADERS ./*.h)
add_library(${PROJECT_NAME} STATIC ${SOURCES} ${HEADERS})

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})
