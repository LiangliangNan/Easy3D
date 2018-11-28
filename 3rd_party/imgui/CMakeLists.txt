cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

add_library(${PROJECT_NAME} STATIC
		imgui.cpp
		imgui_demo.cpp
		imgui_draw.cpp
                imgui_widgets.cpp
		impl/imgui_impl_glfw.cpp
		impl/imgui_impl_opengl3.cpp
)


target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_glew_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_glfw_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_imgui_INCLUDE_DIR})

if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)
endif()
