cmake_minimum_required(VERSION 3.1)

get_filename_component(MODULE_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
set(PROJECT_NAME "easy3d_${MODULE_NAME}")
project(${PROJECT_NAME})


set(${PROJECT_NAME}_HEADERS
    kdtree_search.h
    kdtree_search_ann.h
    kdtree_search_eth.h
    kdtree_search_flann.h
    kdtree_search_nanoflann.h
    )

set(${PROJECT_NAME}_SOURCES
    kdtree_search.cpp
    kdtree_search_ann.cpp
    kdtree_search_eth.cpp
    kdtree_search_flann.cpp
    kdtree_search_nanoflann.cpp
    )

	
add_library(${PROJECT_NAME} STATIC ${${PROJECT_NAME}_SOURCES} ${${PROJECT_NAME}_HEADERS} )

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "easy3d")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC easy3d_core 3rd_kdtree)


# The header files to be installed
file(GLOB MODULE_HEADER_FILES ${CMAKE_CURRENT_LIST_DIR}/*.h)
# Allow to install the lib and the public headers
install(TARGETS ${PROJECT_NAME}
        # IMPORTANT: Add the library to the "export-set"
        EXPORT Easy3DTargets
        PUBLIC_HEADER DESTINATION "${INSTALL_INCLUDE_DIR}/Easy3D/${MODULE_NAME}"
        LIBRARY DESTINATION "${INSTALL_LIB_DIR}/${PROJECT_NAME}"
        RUNTIME DESTINATION "${INSTALL_BIN_DIR}" COMPONENT bin)
# Install the header files
install(
        FILES ${MODULE_HEADER_FILES}
        DESTINATION "${INSTALL_INCLUDE_DIR}/Easy3D/${MODULE_NAME}"
        COMPONENT dev
)
