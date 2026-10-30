get_filename_component(example ${CMAKE_CURRENT_SOURCE_DIR} NAME)
add_example(${example} "main.cpp" "easy3d::viewer")
