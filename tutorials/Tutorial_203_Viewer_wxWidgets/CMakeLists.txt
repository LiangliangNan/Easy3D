cmake_minimum_required(VERSION 3.12)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

# Request wxWidgets support
find_package(wxWidgets COMPONENTS core base gl QUIET)
if (NOT wxWidgets_FOUND)
    message(WARNING "${PROJECT_NAME} requires wxWidgets but wxWidgets was not found. You can set 'wxWidgets_ROOT_DIR' "
		   "to the root directory of wxWidgets (e.g., 'D:/dev/wxWidgets-3.2.1') to help CMake find wxWidgets. "
		   "Without wxWidgets, you will still be able to use all other features provided by Easy3D.")
    if (${CMAKE_VERSION} VERSION_LESS "3.24.0") 
    	message(WARNING "Your CMake version is ${CMAKE_VERSION}. The 'findwxWidgets' module in CMake older than 3.24 may not detect wxWidgets. "
		       "You can update your CMake to a version >= 3.24, or copy the latest version of 'FindwxWidgets.cmake' from the following "
		       "link to your system to fix this: https://gitlab.kitware.com/cmake/cmake/-/blob/master/Modules/FindwxWidgets.cmake")

    endif()
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

set(${PROJECT_NAME}_HEADERS
        application.h
        viewer.h
		window.h
        )

set(${PROJECT_NAME}_SOURCES
        main.cpp
        application.cpp
        viewer.cpp
		window.cpp
        )

if (WIN32 OR MSVC)
	set(${PROJECT_NAME}_RESOURCES "${CMAKE_CURRENT_LIST_DIR}/resources/sample.rc")
    add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_RESOURCES}) # you can add "WIN32" to disable the console window
    message(STATUS "Creating the wxWidgets viewer with a console window")
elseif (APPLE)
    set(${PROJECT_NAME}_ICON "${CMAKE_CURRENT_LIST_DIR}/resources/icons/sample.icns")
    #Append the new files to the resources file list
    list(APPEND ${PROJECT_NAME}_RESOURCES ${${PROJECT_NAME}_ICON})
    add_executable(${PROJECT_NAME} MACOSX_BUNDLE ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_RESOURCES})
    set_source_files_properties(${${PROJECT_NAME}_ICON} PROPERTIES MACOSX_PACKAGE_LOCATION "Resources")
    set_target_properties(${PROJECT_NAME} PROPERTIES
            INSTALL_RPATH "@executable_path/../Frameworks"
            MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
            MACOSX_BUNDLE TRUE
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_LIST_DIR}/resources/Mac/Info.plist"
            MACOSX_BUNDLE_ICON_FILE "sample.icns"
			MACOSX_BUNDLE_SHORT_VERSION_STRING "${Easy3D_MAJOR_VERSION}.${Easy3D_MINOR_VERSION}.${Easy3D_PATCH_VERSION}"
            MACOSX_BUNDLE_LONG_VERSION_STRING "${Easy3D_MAJOR_VERSION}.${Easy3D_MINOR_VERSION}.${Easy3D_PATCH_VERSION}"
            MACOSX_BUNDLE_BUNDLE_VERSION "${Easy3D_MAJOR_VERSION}.${Easy3D_MINOR_VERSION}.${Easy3D_PATCH_VERSION}"
            MACOSX_BUNDLE_COPYRIGHT "Copyright liangliang.nan@gmail.com"
            MACOSX_BUNDLE_GUI_IDENTIFIER "Easy3D.Viewer.wxWidgets"
            )
else ()
    add_executable(${PROJECT_NAME} ${${PROJECT_NAME}_HEADERS} ${${PROJECT_NAME}_SOURCES}) 
endif ()

# The resources directory and app title
target_compile_definitions(${PROJECT_NAME} PRIVATE
        "RESOURCE_DIR=\"${CMAKE_CURRENT_LIST_DIR}/resources\""
        "EXAMPLE_TITLE=\"${PROJECT_NAME}\""
        )

target_include_directories(${PROJECT_NAME} PRIVATE ${Easy3D_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} easy3d::core easy3d::renderer ${wxWidgets_LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tutorials")
