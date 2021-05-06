get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

message(STATUS "Adding target: ${PROJECT_NAME}")
add_executable(${PROJECT_NAME}
#        ThreadPool/example.cpp
#        test_logging.cpp
#        test_signal.cpp
        test_timer.cpp
#        test_spline.cpp
#        test_hrbf.cpp
#        test_console_style.cpp
#        test_linear_algebra.cpp
        )

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "test")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)

target_link_libraries(${PROJECT_NAME} easy3d_core easy3d_util)