cmake_minimum_required(VERSION 3.1)


add_subdirectory(liblbfgs)



get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

add_executable(${PROJECT_NAME}
        main.cpp
        polygonization.h
        polygonization.cpp
        polycube/polycube.cpp
        polycube/polycube_optimize_normals.cpp
        polycube/polycube_optimize_vertices.cpp
        )

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tutorials")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)

target_link_libraries(${PROJECT_NAME} core viewer algo liblbfgs)
