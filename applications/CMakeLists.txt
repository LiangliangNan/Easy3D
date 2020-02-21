cmake_minimum_required(VERSION 3.1)

if(EASY3D_ENABLE_QT)
    find_package(CGAL)
    if(CGAL_FOUND)
        add_subdirectory(Mapple)
    else()
        message(WARNING "Mapple requires CGAL but CGAL was not found")
    endif()
endif()


add_subdirectory(SomeTest)