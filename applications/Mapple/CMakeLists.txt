cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

include( ../../cmake/qt5.cmake )

set(${PROJECT_NAME}_HEADERS
    main_window.h
    paint_canvas.h
    dialog_poisson_reconstruction.h
    dialog_ransac_primitive_extraction.h
    dialog_surface_sampling.h
    dialog_snapshot.h
    widget_checker_sphere.h
    widget_lighting.h
    widget_point_cloud_renderer.h
    widget_surface_mesh_renderer.h
    widget_surface_mesh_texturing.h
    widget_scalar_field.h
    widget_vector_field.h
    )

set(${PROJECT_NAME}_SOURCES
    main.cpp
    main_window.cpp
    paint_canvas.cpp
    paint_canvas_snapshot.cpp
    dialog_poisson_reconstruction.cpp
    dialog_ransac_primitive_extraction.cpp
    dialog_surface_sampling.cpp
    dialog_snapshot.cpp
    widget_checker_sphere.cpp
    widget_lighting.cpp
    widget_point_cloud_renderer.cpp
    widget_surface_mesh_renderer.cpp
    widget_surface_mesh_texturing.cpp
    widget_scalar_field.cpp
    widget_vector_field.cpp
    )

set(${PROJECT_NAME}_FORMS
    main_window.ui
    dialog_poisson_reconstruction.ui
    dialog_ransac_primitive_extraction.ui
    dialog_surface_sampling.ui
    dialog_snapshot.ui
    widget_lighting.ui
    widget_point_cloud_renderer.ui
    widget_surface_mesh_renderer.ui
    widget_surface_mesh_texturing.ui
    widget_scalar_field.ui
    widget_vector_field.ui
    )

set(${PROJECT_NAME}_RESOURCES
    Resources/Mapple.qrc
    )

if( MSVC )
    add_executable( ${PROJECT_NAME} ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_FORMS} ${${PROJECT_NAME}_RESOURCES} )
        # if you want to get rid of the (system) console, use
#	add_executable( ${PROJECT_NAME} WIN32 ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_FORMS} ${${PROJECT_NAME}_RESOURCES} )
#       target_link_libraries( ${PROJECT_NAME} Qt5::WinMain )
elseif( APPLE )
    add_executable( ${PROJECT_NAME} MACOSX_BUNDLE ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_FORMS} ${${PROJECT_NAME}_RESOURCES} )
else()
    add_executable( ${PROJECT_NAME} ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_FORMS} ${${PROJECT_NAME}_RESOURCES} )
endif()


# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
target_compile_definitions(${PROJECT_NAME} PRIVATE QT_DEPRECATED_WARNINGS GLEW_NO_GLU)


target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_glog_INCLUDE_DIR})

if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_DEPRECATE)
endif()

target_link_libraries( ${PROJECT_NAME} Qt5::Core )
target_link_libraries( ${PROJECT_NAME} Qt5::Gui )
target_link_libraries( ${PROJECT_NAME} Qt5::Widgets )
target_link_libraries( ${PROJECT_NAME} Qt5::OpenGL )
target_link_libraries( ${PROJECT_NAME} viewer algo algo_ext)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "demos")

## Note: On macOS, high-DPI support is enabled by settings in the Info.plist file. Make sure they are present.
## For more information, see: http://doc.qt.io/qt-5/highdpi.html
## Enable high DPI support by using my Info.plist file in which I added NSHighResolutionCapable to be true.
if( APPLE )
    set_target_properties( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Mac/Mapple.plist )
endif()

## Resources
# Copy resources dirs into our shadow build directory
#file( COPY ${CMAKE_CURRENT_SOURCE_DIR}/Resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
file( COPY ${CMAKE_CURRENT_SOURCE_DIR}/Resources DESTINATION ${CMAKE_BINARY_DIR}/bin )
