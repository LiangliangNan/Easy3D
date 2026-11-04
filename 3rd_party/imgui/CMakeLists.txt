get_filename_component(module ${CMAKE_CURRENT_SOURCE_DIR} NAME)

set(${module}_HEADERS
        imgui.h
        backends/imgui_impl_glfw.h
        )

set(${module}_SOURCES
        imgui.cpp
        imgui_draw.cpp
        imgui_widgets.cpp
        imgui_tables.cpp
        backends/imgui_impl_glfw.cpp
        )

if(NOT Easy3D_ENABLE_VULKAN)
    list(APPEND ${module}_HEADERS backends/imgui_impl_opengl3.h backends/imgui_impl_opengl3_loader.h)
    list(APPEND ${module}_SOURCES backends/imgui_impl_opengl3.cpp)
endif()

add_3rdparty_module(3rd_${module} "${${module}_SOURCES}" "${${module}_HEADERS}")
target_include_directories(3rd_${module} PUBLIC
        ${CMAKE_CURRENT_LIST_DIR} 
        ${Easy3D_THIRD_PARTY}/glfw/include
        )
target_link_libraries(3rd_${module} PRIVATE 3rd_glfw)
if(Easy3D_ENABLE_VULKAN)
    target_compile_definitions(3rd_imgui PRIVATE GLFW_INCLUDE_NONE)
endif()
