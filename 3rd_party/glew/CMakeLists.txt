cmake_minimum_required(VERSION 3.1)

project(3rd_glew)

add_library(3rd_glew STATIC src/glew.c)

set_target_properties(3rd_glew PROPERTIES FOLDER "3rd_party")

set(GLEW_INCLUDE_DIR  ${CMAKE_CURRENT_LIST_DIR}/include)
target_include_directories(3rd_glew PUBLIC ${GLEW_INCLUDE_DIR})

target_compile_definitions(3rd_glew PUBLIC GLEW_STATIC GLEW_NO_GLU)

set (OpenGL_GL_PREFERENCE GLVND)
find_package(OpenGL REQUIRED)
message(STATUS "OpenGL libraries: ${OPENGL_LIBRARIES}")
target_link_libraries(3rd_glew PUBLIC ${OPENGL_LIBRARIES})

if (MSVC)
    target_compile_definitions(3rd_glew PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _CRT_SECURE_NO_DEPRECATE
        )
endif()