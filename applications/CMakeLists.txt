cmake_minimum_required(VERSION 3.1)

if(EASY3D_ENABLE_QT)
    find_package(CGAL)
    if(CGAL_FOUND)
        # find qt5 components
        find_package(Qt5 COMPONENTS Core Gui OpenGL Widgets)
        # In the case no Qt5Config.cmake file could be found, cmake will explicitly ask the user for the QT5_DIR containing it!
        if (Qt5Core_FOUND AND Qt5Gui_FOUND AND Qt5OpenGL_FOUND AND Qt5Widgets_FOUND)
            add_subdirectory(Mapple)
        else()
            message("Warning: Mapple requires Qt but Qt was not found")
        endif()
    else()
        message("Warning: Mapple requires CGAL but CGAL was not found")
    endif()
endif()


add_subdirectory(experiment)
