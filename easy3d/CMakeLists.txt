add_subdirectory(algo)
add_subdirectory(core)
add_subdirectory(fileio)
add_subdirectory(kdtree)
add_subdirectory(util)
add_subdirectory(renderer)
add_subdirectory(gui)
add_subdirectory(viewer)

if (EASY3D_HAS_CGAL)
    add_subdirectory(algo_ext)  # experimental; not stable
endif ()



#################################################################################################################
# The code below creates Easy3DConfig.cmake file. See an example here:
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/tutorials/How-to-create-a-ProjectConfig.cmake-file
#################################################################################################################

# Add all targets to the build-tree export set
export(TARGETS
        3rd_backward
        3rd_glew
        3rd_glfw
        3rd_glutess
        3rd_lastools
        3rd_kdtree
        3rd_poisson
        3rd_ransac
        3rd_triangle
        3rd_tetgen
        3rd_rply
        easy3d_util
        easy3d_core
        easy3d_kdtree
        easy3d_algo
        easy3d_fileio
        easy3d_renderer
        easy3d_gui
        easy3d_viewer
        FILE "${PROJECT_BINARY_DIR}/Easy3DTargets.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE Easy3D)

## Create the Easy3DConfig.cmake and Easy3DConfigVersion files
#file(RELATIVE_PATH REL_INCLUDE_DIR "${INSTALL_CMAKE_DIR}"
#        "${INSTALL_INCLUDE_DIR}")
# ... for the build tree
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")
configure_file(../cmake/Easy3DConfig.cmake.in
        "${PROJECT_BINARY_DIR}/Easy3DConfig.cmake" @ONLY)
# ... for the install tree
set(CONF_INCLUDE_DIRS "\${EASY3D_CMAKE_DIR}/${REL_INCLUDE_DIR}")
configure_file(../cmake/Easy3DConfig.cmake.in
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/Easy3DConfig.cmake" @ONLY)
# ... for both
configure_file(../cmake/Easy3DConfigVersion.cmake.in
        "${PROJECT_BINARY_DIR}/Easy3DConfigVersion.cmake" @ONLY)


#################################################################################################################
# The code below setups installation of Easy3D (to allow installing Easy3D and using Easy3D from its install tree)
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/tutorials/How-to-create-a-ProjectConfig.cmake-file
#################################################################################################################

## Offer the user the choice of overriding the installation directories
#set(INSTALL_LIB_DIR lib CACHE PATH "Installation directory for libraries")
#set(INSTALL_INCLUDE_DIR include CACHE PATH
#        "Installation directory for header files")
#if(WIN32 AND NOT CYGWIN)
#    set(DEF_INSTALL_CMAKE_DIR CMake)
#else()
#    set(DEF_INSTALL_CMAKE_DIR lib/CMake/FooBar)
#endif()
#set(INSTALL_CMAKE_DIR ${DEF_INSTALL_CMAKE_DIR} CACHE PATH
#        "Installation directory for CMake files")
#
## Install the Easy3DConfig.cmake and Easy3DConfigVersion.cmake
#install(FILES
#        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/Easy3DConfig.cmake"
#        "${PROJECT_BINARY_DIR}/Easy3DVersion.cmake"
#        DESTINATION "${INSTALL_CMAKE_DIR}" COMPONENT dev)
#
## Install the export set for use with the install-tree
#install(EXPORT Easy3DTargets DESTINATION
#        "${INSTALL_CMAKE_DIR}" COMPONENT dev)
