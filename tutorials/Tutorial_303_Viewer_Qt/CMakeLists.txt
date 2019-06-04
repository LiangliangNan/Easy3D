cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

include( cmake/qt5.cmake )

if (Qt5Widgets_FOUND)

    set(${PROJECT_NAME}_HEADERS
        main_window.h
        viewer_qt.h
        )

    set(${PROJECT_NAME}_SOURCES
        main.cpp
        main_window.cpp
        viewer_qt.cpp
        )

    set(${PROJECT_NAME}_FORMS
        main_window.ui
        )

    set(${PROJECT_NAME}_RESOURCES
        Resources/ViewerQt.qrc
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

    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_QT5)

    if (MSVC)
        target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_DEPRECATE)
    endif()

    target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})
    target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_glog_INCLUDE_DIR})

    target_link_libraries( ${PROJECT_NAME} Qt5::Core )
    target_link_libraries( ${PROJECT_NAME} Qt5::Gui )
    target_link_libraries( ${PROJECT_NAME} Qt5::Widgets )
    target_link_libraries( ${PROJECT_NAME} Qt5::OpenGL )
    target_link_libraries( ${PROJECT_NAME} viewer)

    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Tutorials")

    ## Note: On macOS, high-DPI support is enabled by settings in the Info.plist file. Make sure they are present.
    ## For more information, see: http://doc.qt.io/qt-5/highdpi.html
    ## Enable high DPI support by using my Info.plist file in which I added NSHighResolutionCapable to be true.
    if( APPLE )
        set_target_properties( ${PROJECT_NAME} PROPERTIES MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/Mac/ViewerQt.plist )
    endif()

    ## Resources
    # Copy resources dirs into our shadow build directory
    #file( COPY ${CMAKE_CURRENT_SOURCE_DIR}/Resources DESTINATION ${CMAKE_CURRENT_BINARY_DIR} )
    file( COPY ${CMAKE_CURRENT_SOURCE_DIR}/Resources DESTINATION ${CMAKE_BINARY_DIR}/bin )

else()
    message(WARNING "The ViewerQt example requires Qt5, which could not be found! Providing the QT5_ROOT_PATH (i.e., where the 'bin' folder resides) to CMake may solve it (if Qt exists).")

    set(${PROJECT_NAME}_SOURCES
        main.cpp
        )

    add_executable( ${PROJECT_NAME} ${${PROJECT_NAME}_SOURCES})

    set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "Tutorials")
endif()
