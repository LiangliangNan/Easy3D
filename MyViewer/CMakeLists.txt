get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

add_executable(${PROJECT_NAME}
        main.cpp
        viewer.h
        viewer.cpp
        drawable.h
        drawable.cpp
        drawable_points.h
        drawable_points.cpp
        drawable_lines.h
        drawable_lines.cpp
        drawable_triangles.h
        drawable_triangles.cpp
        )
target_link_libraries(${PROJECT_NAME} engine)

target_compile_definitions(${PROJECT_NAME} PUBLIC "ASSET_DIR=\"${CMAKE_CURRENT_LIST_DIR}/data/\"")

