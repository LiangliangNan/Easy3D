cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(${PROJECT_NAME}_HEADERS
    ./arcball.h
    ./box.h
    ./camera.h
	./constraint.h
    ./drawable.h
    ./file_dialog.h
	./frame.h
    ./IO.h
	./manipulated_camera_frame.h
	./manipulated_frame.h
    ./mat.h
    ./math_types.h
    ./model.h
    ./opengl_error.h
	./opengl_info.h
    ./point_cloud.h
    ./properties.h
    ./quat.h
    ./resources.h
    ./shader.h
	./shader_program.h
    ./surface_mesh.h
    ./transform.h
    ./vec.h
    ./vertex_array_object.h
    ./viewer.h
    )

set(${PROJECT_NAME}_SOURCES
	./arcball.cpp
    ./camera.cpp
	./constraint.cpp
    ./drawable.cpp
    ./file_dialog.cpp
	./frame.cpp
    ./IO.cpp
    ./IO_obj.cpp
    ./IO_off.cpp
    ./IO_poly.cpp
    ./IO_stl.cpp
	./manipulated_camera_frame.cpp
	./manipulated_frame.cpp
    ./model.cpp
    ./opengl_error.cpp
	./opengl_info.cpp
    ./point_cloud.cpp
    ./resources.cpp
    ./shader.cpp
	./shader_program.cpp
    ./surface_mesh.cpp
    ./transform.cpp
    ./vertex_array_object.cpp
    ./viewer.cpp
	)

	

add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})
set_target_properties(${PROJECT_NAME} PROPERTIES
                      FOLDER "Easy3D")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_glew_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_glfw_INCLUDE_DIR})


if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC _CRT_SECURE_NO_WARNINGS)
endif()

