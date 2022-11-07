cmake_minimum_required(VERSION 3.12)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_LIST_DIR} NAME)
project(${PROJECT_NAME})


include(../../cmake/UseQt5.cmake)
if (NOT QT5_FOUND)
    message(WARNING "${PROJECT_NAME} requires Qt but Qt was not found. You can set 'Qt5_DIR' to the "
            "directory containing 'Qt5Config.cmake' or 'qt5-config.cmake'. "
            "Optionally, you can set the Qt5 root directory 'QT5_ROOT_PATH' to the directory "
            "containing the 'bin' folder.")
    return()
endif ()


set(${PROJECT_NAME}_HEADERS
        main_window.h
        paint_canvas.h
        walk_through.h

        dialogs/dialog.h
        dialogs/dialog_surface_mesh_curvature.h
        dialogs/dialog_surface_mesh_fairing.h
        dialogs/dialog_surface_mesh_from_text.h
        dialogs/dialog_surface_mesh_hole_filling.h
        dialogs/dialog_surface_mesh_remeshing.h
        dialogs/dialog_surface_mesh_parameterization.h
        dialogs/dialog_surface_mesh_simplification.h
        dialogs/dialog_surface_mesh_smoothing.h
        dialogs/dialog_gaussian_noise.h
        dialogs/dialog_poisson_reconstruction.h
        dialogs/dialog_properties.h
        dialogs/dialog_point_cloud_normal_estimation.h
        dialogs/dialog_point_cloud_ransac_primitive_extraction.h
        dialogs/dialog_surface_mesh_sampling.h
        dialogs/dialog_snapshot.h
        dialogs/dialog_point_cloud_simplification.h
        dialogs/dialog_walk_through.h

        widgets/widget_checker_sphere.h
        widgets/widget_drawable.h
        widgets/widget_drawable_points.h
        widgets/widget_drawable_lines.h
        widgets/widget_drawable_triangles.h
        widgets/widget_global_setting.h
        widgets/widget_log.h

        tools/canvas.h
        tools/tool.h
        tools/tool_manager.h
        tools/tool_segmentation_surface_mesh.h
        tools/tool_selection_point_cloud.h
        tools/tool_selection_surface_mesh.h
        )

set(${PROJECT_NAME}_SOURCES
        main.cpp
        main_window.cpp
        paint_canvas.cpp
        paint_canvas_snapshot.cpp
        walk_through.cpp

        dialogs/dialog.cpp
        dialogs/dialog_surface_mesh_curvature.cpp
        dialogs/dialog_surface_mesh_fairing.cpp
        dialogs/dialog_surface_mesh_from_text.cpp
        dialogs/dialog_surface_mesh_hole_filling.cpp
        dialogs/dialog_surface_mesh_parameterization.cpp
        dialogs/dialog_surface_mesh_remeshing.cpp
        dialogs/dialog_surface_mesh_simplification.cpp
        dialogs/dialog_surface_mesh_smoothing.cpp
        dialogs/dialog_gaussian_noise.cpp
        dialogs/dialog_poisson_reconstruction.cpp
        dialogs/dialog_properties.cpp
        dialogs/dialog_point_cloud_normal_estimation.cpp
        dialogs/dialog_point_cloud_ransac_primitive_extraction.cpp
        dialogs/dialog_surface_mesh_sampling.cpp
        dialogs/dialog_snapshot.cpp
        dialogs/dialog_point_cloud_simplification.cpp
        dialogs/dialog_walk_through.cpp

        widgets/widget_checker_sphere.cpp
        widgets/widget_drawable.cpp
        widgets/widget_drawable_points.cpp
        widgets/widget_drawable_lines.cpp
        widgets/widget_drawable_triangles.cpp
        widgets/widget_global_setting.cpp
        widgets/widget_model_list.cpp
        widgets/widget_log.cpp

        tools/tool.cpp
        tools/tool_manager.cpp
        tools/tool_segmentation_surface_mesh.cpp
        tools/tool_selection_point_cloud.cpp
        tools/tool_selection_surface_mesh.cpp
        )

set(${PROJECT_NAME}_FORMS
        main_window.ui

        dialogs/dialog_surface_mesh_curvature.ui
        dialogs/dialog_surface_mesh_fairing.ui
        dialogs/dialog_surface_mesh_from_text.ui
        dialogs/dialog_surface_mesh_hole_filling.ui
        dialogs/dialog_surface_mesh_parameterization.ui
        dialogs/dialog_surface_mesh_remeshing.ui
        dialogs/dialog_surface_mesh_simplification.ui
        dialogs/dialog_surface_mesh_smoothing.ui
        dialogs/dialog_gaussian_noise.ui
        dialogs/dialog_poisson_reconstruction.ui
        dialogs/dialog_properties.ui
        dialogs/dialog_point_cloud_normal_estimation.ui
        dialogs/dialog_point_cloud_ransac_primitive_extraction.ui
        dialogs/dialog_surface_mesh_sampling.ui
        dialogs/dialog_snapshot.ui
        dialogs/dialog_point_cloud_simplification.ui
        dialogs/dialog_walk_through.ui

        widgets/widget_drawable_points.ui
        widgets/widget_drawable_lines.ui
        widgets/widget_drawable_triangles.ui
        widgets/widget_global_setting.ui
        widgets/widget_model_list.h
        )

set(${PROJECT_NAME}_RESOURCES
        resources/Mapple.qrc
        resources/Mapple.rc
        )

if (WIN32 OR MSVC)
    add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_FORMS} ${${PROJECT_NAME}_RESOURCES})
    # if you want to get rid of the (system) console, use
    #	add_executable( ${PROJECT_NAME} WIN32 ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_FORMS} ${${PROJECT_NAME}_RESOURCES} )
    #       target_link_libraries( ${PROJECT_NAME} Qt5::WinMain )

#    add_custom_command(
#            TARGET ${PROJECT_NAME}
#            POST_BUILD
#            COMMAND ${WINDEPLOYQT} --no-translations --verbose 0 $<TARGET_FILE:${PROJECT_NAME}>
#    )

elseif (APPLE)
    function(resource VAR SOURCE_PATH DESTINATION PATTERN)
        file(GLOB_RECURSE _LIST CONFIGURE_DEPENDS ${SOURCE_PATH}/${PATTERN})
        foreach (RESOURCE ${_LIST})
            get_filename_component(_PARENT ${RESOURCE} DIRECTORY)
            if (${_PARENT} STREQUAL ${SOURCE_PATH})
                set(_DESTINATION ${DESTINATION})
            else ()
                file(RELATIVE_PATH _DESTINATION ${SOURCE_PATH} ${_PARENT})
                set(_DESTINATION ${DESTINATION}/${_DESTINATION})
            endif ()
            set_property(SOURCE ${RESOURCE} PROPERTY MACOSX_PACKAGE_LOCATION ${_DESTINATION})
        endforeach (RESOURCE)
        set(${VAR} ${_LIST} PARENT_SCOPE)
    endfunction()

    resource(VERTEX_SHADERS ${Easy3D_ROOT}/resources/shaders Resources/shaders *.vert)
    resource(GEOMETRY_SHADERS ${Easy3D_ROOT}/resources/shaders Resources/shaders *.geom)
    resource(FRAGMENT_SHADERS ${Easy3D_ROOT}/resources/shaders Resources/shaders *.frag)
    resource(COLORMAPS ${Easy3D_ROOT}/resources/colormaps Resources/colormaps *.png)
    resource(FONTS ${Easy3D_ROOT}/resources/fonts Resources/fonts *.ttf)
    resource(IMAGES ${Easy3D_ROOT}/resources/images Resources/images overview.jpg)  # overview.jpg is for splash screen
    resource(TEXTURES ${Easy3D_ROOT}/resources/textures Resources/textures *.png)

    # This tells CMake where to find and install the ICON file
    set(${PROJECT_NAME}_ICON ${CMAKE_CURRENT_LIST_DIR}/resources/icons/Mapple.icns)

    #Append the new files to the resources file list
    list(APPEND ${PROJECT_NAME}_RESOURCES ${${PROJECT_NAME}_ICON} ${VERTEX_SHADERS} ${GEOMETRY_SHADERS} ${FRAGMENT_SHADERS} ${COLORMAPS} ${FONTS} ${IMAGES} ${TEXTURES})

    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${${PROJECT_NAME}_ICON} ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_FORMS} ${${PROJECT_NAME}_RESOURCES})

    set_source_files_properties(${${PROJECT_NAME}_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")

    ## Note: On macOS, high-DPI support is enabled by settings in the Info.plist file. Make sure they are present.
    ## For more information, see: http://doc.qt.io/qt-5/highdpi.html
    ## Enable high DPI support by using my Info.plist file in which I added NSHighResolutionCapable to be true.
    configure_file("${CMAKE_CURRENT_LIST_DIR}/resources/Mac/Mapple.plist.in" "${CMAKE_CURRENT_BINARY_DIR}/Mapple/resources/Mac/Mapple.plist" @ONLY)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
            MACOSX_BUNDLE_GUI_IDENTIFIER ${PROJECT_NAME}
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_BINARY_DIR}/Mapple/resources/Mac/Mapple.plist"
            MACOSX_BUNDLE_ICON_FILE "Mapple.icns"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${Easy3D_MAJOR_VERSION}.${Easy3D_MINOR_VERSION}.${Easy3D_PATCH_VERSION}"
            MACOSX_BUNDLE_LONG_VERSION_STRING "${Easy3D_MAJOR_VERSION}.${Easy3D_MINOR_VERSION}.${Easy3D_PATCH_VERSION}"
            MACOSX_BUNDLE_BUNDLE_VERSION "${Easy3D_MAJOR_VERSION}.${Easy3D_MINOR_VERSION}.${Easy3D_PATCH_VERSION}"
            INSTALL_RPATH "@executable_path/../Frameworks"
            )
else ()
    add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_FORMS} ${${PROJECT_NAME}_RESOURCES})
endif ()

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "applications")

target_include_directories(${PROJECT_NAME} PRIVATE ${Easy3D_INCLUDE_DIR})

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
target_compile_definitions(${PROJECT_NAME} PRIVATE QT_DEPRECATED_WARNINGS GLEW_NO_GLU)

if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_DEPRECATE)
endif ()

target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL)

target_link_libraries(${PROJECT_NAME} PRIVATE easy3d::renderer easy3d::algo easy3d::gui)
if (Easy3D_HAS_CGAL)
    target_link_libraries(${PROJECT_NAME} PRIVATE easy3d::algo_ext)
endif ()
if (Easy3D_HAS_FFMPEG)
    target_link_libraries(${PROJECT_NAME} PRIVATE easy3d::video)
endif ()
