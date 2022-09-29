cmake_minimum_required(VERSION 3.12)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

# Request wxWidgets support
find_package(wxWidgets QUIET COMPONENTS core base gl)
if (NOT wxWidgets_FOUND)
    message(WARNING "${PROJECT_NAME} requires wxWidgets but wxWidgets was not found. "
					"NOTE: CMake findwxWidgets module may not detect wxWidgets in versions of CMake older than 3.24. "
					"You can update your CMake to a version greater or equal to 3.24, or copy the latest version of "
					"FindwxWidgets.cmake from the following link to your system to fix this: "
					"https://gitlab.kitware.com/cmake/cmake/-/blob/master/Modules/FindwxWidgets.cmake"
					"If you build wxWidgets itself using CMake, you can also use the CONFIG mode of find_package() "
					"which works even with older CMake versions.")
    return()
else ()
    message(STATUS "Found wxWidgets v${wxWidgets_VERSION_STRING}")
    message(STATUS "wxWidgets include directory: ${wxWidgets_INCLUDE_DIRS}")
    message(STATUS "wxWidgets libraries: ${wxWidgets_LIBRARIES}")
    if (wxWidgets_VERSION_STRING VERSION_LESS "3.1.0")
        message(FATAL_ERROR
                "Found wxWidgets v${wxWidgets_VERSION_STRING}. However, the minimum version of wxWidgets required is "
                "v3.1.0. Please install wxWidgets v3.1.0 or above. If you don't need wxWidgets, disable wxWidgets and "
                "you will still be able to use all other features provided by Easy3D.")
        return()
    endif ()
endif ()

# Include the wxWidgets use file to initialize various settings
if (wxWidgets_USE_FILE)
    include(${wxWidgets_USE_FILE})
    message(STATUS "wxWidgets_USE_FILE: ${wxWidgets_USE_FILE}")
endif ()

set(SRC_FILES
        main.cpp
        application.h
        application.cpp
        main_window.h
        main_window.cpp
        viewer.h
        viewer.cpp
        )

if (WIN32 OR MSVC)
	option(WIN32_wxWidget_Viewer_No_Console "Build the wxWidget viewer without a console window" OFF)

    list(APPEND SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Resources/sample.rc)    # Include a RC file for windows
	if (WIN32_wxWidget_Viewer_No_Console)
        add_executable(${PROJECT_NAME} WIN32 ${SRC_FILES})
		target_compile_definitions(${PROJECT_NAME} PRIVATE WIN32_VIEWER_WITHOUT_CONSOLE)
        message(STATUS "Creating the wxWidgets viewer without a console window")
    else ()
        add_executable(${PROJECT_NAME} ${SRC_FILES})
        message(STATUS "Creating the wxWidgets viewer with a console window")
	endif ()
elseif (APPLE)
    set(${PROJECT_NAME}_ICON ${CMAKE_CURRENT_SOURCE_DIR}/Resources/sample.icns)
    list(APPEND SRC_FILES ${${PROJECT_NAME}_ICON})  # Add an icon for the apple .app file

    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${SRC_FILES})
    set_source_files_properties(${${PROJECT_NAME}_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set_target_properties(${PROJECT_NAME} PROPERTIES
            INSTALL_RPATH "@executable_path/../Frameworks"
            MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Info.plist"
            MACOSX_BUNDLE_ICON_FILE "sample.icns"
            MACOSX_BUNDLE_COPYRIGHT "Copyright liangliang.nan@gmail.com"
            MACOSX_BUNDLE_GUI_IDENTIFIER "Easy3D.Viewer.wxWidgets"
            )
endif ()

target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES} easy3d::core easy3d::renderer)

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tutorials")
