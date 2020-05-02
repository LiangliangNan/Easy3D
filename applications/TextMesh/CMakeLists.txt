get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


find_package(Freetype)
if (NOT FREETYPE_FOUND)
    message(WARNING "Freetype not found, so the 3DText is disabled")
    return()
endif()

message(STATUS "Freetype include dirs: ${FREETYPE_INCLUDE_DIRS} and libs: ${FREETYPE_LIBRARIES}")

add_executable(${PROJECT_NAME}
        ftgl/Point.cpp
        ftgl/Contour.cpp
        ftgl/Vectoriser.cpp
        poly2tri/common/shapes.cc
        poly2tri/sweep/cdt.cc
        poly2tri/sweep/advancing_front.cc
        poly2tri/sweep/sweep_context.cc
        poly2tri/sweep/sweep.cc

        text_mesh.h
        text_mesh.cpp
        main.cpp
        )
set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "tutorials")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR} ${FREETYPE_INCLUDE_DIRS} ${GLUT_INCLUDE_DIR})

target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)

target_link_libraries(${PROJECT_NAME} ${FREETYPE_LIBRARIES}  ${GLUT_LIBRARIES} core viewer fileio)