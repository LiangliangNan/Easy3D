cmake_minimum_required(VERSION 3.12)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

# Request wxWidgets support
find_package(wxWidgets QUIET COMPONENTS core base gl)
if (NOT wxWidgets_FOUND)
    message(WARNING "${PROJECT_NAME} requires wxWidgets but wxWidgets was not found. You can set 'wxWidgets_DIR' to the "
            "directory containing 'wxWidgetsConfig.cmake' or 'wxWidgets-config.cmake'.")
    return()
endif ()

#message(STATUS "------- wxWidgets_INCLUDE_DIR: ${wxWidgets_INCLUDE_DIRS}")
#message(STATUS "------- wxWidgets_LIBRARIES: ${wxWidgets_LIBRARIES}")

# Include the wxWidgets use file to initialize various settings
if (wxWidgets_USE_FILE)
    include(${wxWidgets_USE_FILE})
#    message(STATUS "------- wxWidgets_USE_FILE: ${wxWidgets_USE_FILE}")
endif ()

set(SRC_FILES
        application.h
        application.cpp
        main_window.h
        main_window.cpp
        viewer.h
        viewer.cpp
        )

if (WIN32)
    list(APPEND SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Resources/sample.rc)    # Include a RC file for windows
elseif (APPLE)
    list(APPEND SRC_FILES ${CMAKE_CURRENT_SOURCE_DIR}/Resources/sample.icns)  # Add an icon for the apple .app file
endif ()

add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${SRC_FILES})
target_link_libraries(${PROJECT_NAME} ${wxWidgets_LIBRARIES} easy3d::core easy3d::renderer)

if (APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
            MACOSX_BUNDLE TRUE
            RESOURCE "${CMAKE_CURRENT_SOURCE_DIR}/Resources/sample.icns"
            MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Resources/Info_cocoa.plist"
            MACOSX_BUNDLE_ICON_FILE "${CMAKE_CURRENT_SOURCE_DIR}/Resources/sample.icns"
            MACOSX_BUNDLE_COPYRIGHT "Copyright liangliang.nan@gmail.com"
            MACOSX_BUNDLE_GUI_IDENTIFIER "Easy3D.Viewer.wxWidgets"
            )
endif ()
