cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})



add_library(${PROJECT_NAME} STATIC src/glew.c)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "3rd_party")

set(GLEW_INCLUDE_DIR  ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(${PROJECT_NAME} PUBLIC ${GLEW_INCLUDE_DIR})

target_compile_definitions(${PROJECT_NAME} PUBLIC GLEW_STATIC GLEW_NO_GLU)

set (OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
message(STATUS "OpenGL libraries: ${OPENGL_LIBRARIES}")
target_link_libraries(${PROJECT_NAME} PUBLIC ${OPENGL_LIBRARIES})

if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _CRT_SECURE_NO_DEPRECATE
        )
endif()
