cmake_minimum_required(VERSION 3.1)

project(3rd_imgui)

set(imgui_HEADERS
    imgui.h
    backends/imgui_impl_glfw.h
    backends/imgui_impl_opengl3.h
    )

set(imgui_SOURCES
    imgui.cpp
    imgui_demo.cpp
    imgui_draw.cpp
    imgui_widgets.cpp
    imgui_tables.cpp
    backends/imgui_impl_glfw.cpp
    backends/imgui_impl_opengl3.cpp
    )


add_library(3rd_imgui STATIC ${imgui_SOURCES} ${imgui_HEADERS})

set_target_properties(3rd_imgui PROPERTIES FOLDER "3rd_party")

set(IMGUI_INCLUDE_DIR  ${CMAKE_CURRENT_LIST_DIR}  ${EASY3D_THIRD_PARTY}/glew/include)
target_include_directories(3rd_imgui PUBLIC ${IMGUI_INCLUDE_DIR})

target_link_libraries(3rd_imgui PUBLIC 3rd_glew 3rd_glfw)


if (MSVC)
    target_compile_definitions(3rd_imgui PRIVATE
        _CRT_SECURE_NO_WARNINGS
        _CRT_SECURE_NO_DEPRECATE
        )
endif()