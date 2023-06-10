cmake_minimum_required(VERSION 3.12)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
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
        window.h
        viewer.h
        )

set(${PROJECT_NAME}_SOURCES
        main.cpp
        window.cpp
        viewer.cpp
        )

set(${PROJECT_NAME}_FORMS
        window.ui
        )

set(${PROJECT_NAME}_RESOURCES
        resources/ViewerQt.qrc
		resources/ViewerQt.rc
        )

if (WIN32 OR MSVC)
    add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_FORMS} ${${PROJECT_NAME}_RESOURCES})
    # if you want to get rid of the (system) console, use
    #	add_executable( ${PROJECT_NAME} WIN32 ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_FORMS} ${${PROJECT_NAME}_RESOURCES} )
    #       target_link_libraries( ${PROJECT_NAME} Qt5::WinMain )
elseif (APPLE)
    # This tells CMake where to find and install the ICON file
    set(${PROJECT_NAME}_ICON ${CMAKE_CURRENT_SOURCE_DIR}/resources/icons/ViewerQt.icns)
    #Append the new files to the resources file list
    list(APPEND ${PROJECT_NAME}_RESOURCES ${${PROJECT_NAME}_ICON})
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_FORMS} ${${PROJECT_NAME}_RESOURCES})
    set_source_files_properties(${${PROJECT_NAME}_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    ## Note: On macOS, high-DPI support is enabled by settings in the Info.plist file. Make sure they are present.
    ## For more information, see: http://doc.qt.io/qt-5/highdpi.html
    ## Enable high DPI support by using my Info.plist file in which I added NSHighResolutionCapable to be true.
    set_target_properties(${PROJECT_NAME} PROPERTIES
            INSTALL_RPATH "@executable_path/../Frameworks"
            MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/resources/Mac/ViewerQt.plist"
            MACOSX_BUNDLE_ICON_FILE "ViewerQt.icns"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "${Easy3D_MAJOR_VERSION}.${Easy3D_MINOR_VERSION}.${Easy3D_PATCH_VERSION}"
            MACOSX_BUNDLE_LONG_VERSION_STRING "${Easy3D_MAJOR_VERSION}.${Easy3D_MINOR_VERSION}.${Easy3D_PATCH_VERSION}"
            MACOSX_BUNDLE_BUNDLE_VERSION "${Easy3D_MAJOR_VERSION}.${Easy3D_MINOR_VERSION}.${Easy3D_PATCH_VERSION}"
			MACOSX_BUNDLE_COPYRIGHT "Copyright liangliang.nan@gmail.com"
            MACOSX_BUNDLE_GUI_IDENTIFIER "Easy3D.Viewer.Qt"
            )
else ()
    add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_FORMS} ${${PROJECT_NAME}_RESOURCES})
endif ()

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
target_compile_definitions(${PROJECT_NAME} PRIVATE QT_DEPRECATED_WARNINGS)

if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_DEPRECATE)
endif ()

# The app title
target_compile_definitions(${PROJECT_NAME} PRIVATE "EXAMPLE_TITLE=\"${PROJECT_NAME}\"")

target_include_directories(${PROJECT_NAME} PRIVATE ${Easy3D_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE Qt5::Core Qt5::Gui Qt5::Widgets Qt5::OpenGL easy3d::renderer)
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tutorials")
