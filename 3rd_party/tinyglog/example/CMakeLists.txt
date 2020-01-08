cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(glog_example)


add_executable(${PROJECT_NAME}
                main.cpp
)

set_target_properties(glog_example PROPERTIES FOLDER "tinyglog")

target_include_directories(glog_example PRIVATE ${EASY3D_INCLUDE_DIR})

target_link_libraries(glog_example glog)
