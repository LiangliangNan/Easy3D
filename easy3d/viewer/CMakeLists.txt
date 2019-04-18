cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(${PROJECT_NAME}_HEADERS
    ambient_occlusion.h
    average_color_blending.h
    camera.h
    constraint.h
    drawable.h
    dual_depth_peeling.h
    eye_dome_lighting.h
    frame.h
    framebuffer_object.h
    frustum.h
    manipulated_camera_frame.h
    manipulated_frame.h
    model.h
    opengl.h
    opengl_error.h
    opengl_info.h
    primitives.h
    read_pixel.h
    setting.h
    shader_manager.h
    shader_program.h
    shadow.h
    soft_shadow.h
    opengl_timer.h
    transform.h
    transform_decompose.h
    transparency.h
    vertex_array_object.h
    viewer.h
    )

set(${PROJECT_NAME}_SOURCES
    ambient_occlusion.cpp
    average_color_blending.cpp
    camera.cpp
    constraint.cpp
    drawable.cpp
    dual_depth_peeling.cpp
    eye_dome_lighting.cpp
    frame.cpp
    framebuffer_object.cpp
    frustum.cpp
    manipulated_camera_frame.cpp
    manipulated_frame.cpp
    model.cpp
    opengl_error.cpp
    opengl_info.cpp
    primitives.cpp
    read_pixel.cpp
    setting.cpp
    shader_manager.cpp
    shader_program.cpp
    shadow.cpp
    soft_shadow.cpp
    opengl_timer.cpp
    transform.cpp
    transform_decompose.cpp
    vertex_array_object.cpp
    viewer.cpp
    )
	
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SHADERS} )

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Easy3D")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_glew_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_glfw_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_glog_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} glew glfw core util fileio tinyfiledialogs)


if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_DEPRECATE)
endif()
