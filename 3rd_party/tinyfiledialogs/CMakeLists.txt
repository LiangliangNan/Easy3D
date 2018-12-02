cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})



add_library(${PROJECT_NAME} STATIC src/glew.c)


target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_glew_INCLUDE_DIR})
