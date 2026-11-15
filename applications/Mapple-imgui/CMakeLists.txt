get_filename_component(name ${CMAKE_CURRENT_SOURCE_DIR} NAME)

add_executable(${name}
        main.cpp
        mapple_viewer.h
        mapple_viewer.cpp
        imgui_layer.h
        imgui_layer.cpp
        ui/menu.cpp
        ui/panels.cpp
        )
set_target_properties(${name} PROPERTIES FOLDER "applications")
target_include_directories(${name} PRIVATE ${Easy3D_INCLUDE_DIR})
target_link_libraries(${name} PRIVATE
        3rd_imgui
        easy3d::core
        easy3d::gui
        easy3d::renderer
        easy3d::util
        easy3d::viewer
        )

if (Easy3D_ENABLE_VULKAN)
    target_compile_definitions(${name} PRIVATE "APP_TITLE=\"${name} - Vulkan\"")
else ()
    target_compile_definitions(${name} PRIVATE "APP_TITLE=\"${name} - OpenGL\"")
endif ()
