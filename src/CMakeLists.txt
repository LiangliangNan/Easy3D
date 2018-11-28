cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(Easy3D)


add_library(Easy3D STATIC
                basic_viewer.cpp
                plugin.cpp
                plugin_examples.cpp
                viewer.cpp
                window.cpp
)

target_include_directories(Easy3D PRIVATE ${EASY3D_glew_INCLUDE_DIR})
target_include_directories(Easy3D PRIVATE ${EASY3D_glfw_INCLUDE_DIR})
target_include_directories(Easy3D PRIVATE ${EASY3D_imgui_INCLUDE_DIR})


if (MSVC)
    target_compile_definitions(Easy3D PRIVATE GLEW_STATIC)
endif()
