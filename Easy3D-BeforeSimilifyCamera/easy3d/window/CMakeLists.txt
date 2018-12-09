cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(${PROJECT_NAME}_HEADERS
    ./imgui_fonts_droid_sans.h
    ./ImGuiHelpers.h
    ./imguitraits.h
    ./main_window.h
    ./panel.h
    ./plugin.h
    )

set(${PROJECT_NAME}_SOURCES
    ./main_window.cpp
    ./panel.cpp
    ./plugin.cpp
    )


add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES
                      FOLDER "Easy3D")

target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_glew_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_glfw_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_imgui_INCLUDE_DIR})
