cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(${PROJECT_NAME}_HEADERS
    ./imgui_fonts_droid_sans.h
    ./ImGuiHelpers.h
    ./imguitraits.h
    ./viewer_imgui.h

    ./imgui/imgui.h
    ./imgui/impl/imgui_impl_glfw.h
    ./imgui/impl/imgui_impl_opengl3.h
    )

set(${PROJECT_NAME}_SOURCES
    ./main.cpp
    ./viewer_imgui.cpp

    ./imgui/imgui.cpp
    ./imgui/imgui_demo.cpp
    ./imgui/imgui_draw.cpp
    ./imgui/imgui_widgets.cpp
    ./imgui/impl/imgui_impl_glfw.cpp
    ./imgui/impl/imgui_impl_opengl3.cpp
    )


add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS})

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Tutorials")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_glew_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_glfw_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR}/imgui)

target_link_libraries(${PROJECT_NAME} glew glfw core)
