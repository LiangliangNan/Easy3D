cmake_minimum_required(VERSION 3.1)

if(EASY3D_ENABLE_QT)
    include( ../cmake/qt5.cmake )
    if (QT5_FOUND)
        find_package(CGAL)
        if(CGAL_FOUND)
            add_subdirectory(Mapple)
        else()
            message("Warning: Mapple requires CGAL but CGAL was not found")
        endif()
    else()
        message("Warning: Mapple requires Qt but Qt was not found")
    endif()
endif()


add_subdirectory(experiment)
