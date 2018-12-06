cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(easy3d_core_HEADERS
    ./camera.h
    ./constraint.h
    ./drawable.h
    ./fields.h
    ./file_dialog.h
    ./frame.h
    ./manipulated_camera_frame.h
    ./manipulated_frame.h
    ./model.h
    ./opengl_error.h
    ./opengl_info.h
    ./primitives.h
    ./shader_code.h
    ./shader_program.h
    ./transform.h
    ./vertex_array_object.h
    ./viewer.h
    )

set(easy3d_core_SOURCES
    ./camera.cpp
    ./constraint.cpp
    ./drawable.cpp
    ./fields.cpp
    ./file_dialog.cpp
    ./frame.cpp
    ./manipulated_camera_frame.cpp
    ./manipulated_frame.cpp
    ./model.cpp
    ./opengl_error.cpp
    ./opengl_info.cpp
    ./primitives.cpp
    ./shader_code.cpp
    ./shader_program.cpp
    ./transform.cpp
    ./vertex_array_object.cpp
    ./viewer.cpp
    )


add_library(${PROJECT_NAME} STATIC ${easy3d_core_SOURCES} ${easy3d_core_HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES
                      FOLDER "Easy3D")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_glew_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_glfw_INCLUDE_DIR})


if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)
endif()
