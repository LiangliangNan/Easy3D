cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


add_executable(${PROJECT_NAME}
                main.cpp
)
set_target_properties(${PROJECT_NAME} PROPERTIES
                      FOLDER "Tutorials")


target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})


find_package(OpenGL REQUIRED)
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES} glew glfw core viewer fileio)
