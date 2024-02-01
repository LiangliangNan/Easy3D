cmake_minimum_required(VERSION 3.1)

get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(PROJECT_NAME 3rd_${MODULE_NAME})
project(${PROJECT_NAME})



add_library(${PROJECT_NAME} STATIC
    include/GL/eglew.h
    include/GL/glew.h
    include/GL/glxew.h
    include/GL/wglew.h
    src/glew.c
    )
set_target_properties(${PROJECT_NAME} PROPERTIES
                      FOLDER "3rd_party")


target_include_directories(${PROJECT_NAME} PRIVATE ${ADTREE_glew_INCLUDE_DIR})


target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)

