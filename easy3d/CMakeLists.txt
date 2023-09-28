# All available Easy3D libraries
set(CONF_Easy3D_COMPONENTS)

# Add all targets to the build-tree export set
set(EXPORT_TARGET_SET)

add_subdirectory(util)
list(APPEND CONF_Easy3D_COMPONENTS util)
list(APPEND EXPORT_TARGET_SET easy3d_util)

add_subdirectory(core)
list(APPEND CONF_Easy3D_COMPONENTS core)
list(APPEND EXPORT_TARGET_SET easy3d_core)

add_subdirectory(fileio)
list(APPEND CONF_Easy3D_COMPONENTS fileio)
list(APPEND EXPORT_TARGET_SET easy3d_fileio 3rd_lastools 3rd_rply)

add_subdirectory(kdtree)
list(APPEND CONF_Easy3D_COMPONENTS kdtree)
list(APPEND EXPORT_TARGET_SET easy3d_kdtree 3rd_kdtree)

add_subdirectory(algo)
list(APPEND CONF_Easy3D_COMPONENTS algo)
list(APPEND EXPORT_TARGET_SET easy3d_algo 3rd_poisson 3rd_ransac 3rd_triangle 3rd_tetgen 3rd_polypartition 3rd_glutess 3rd_opcode)

if (Easy3D_HAS_CGAL)
    add_subdirectory(algo_ext)  # experimental
    list(APPEND CONF_Easy3D_COMPONENTS algo_ext)
    list(APPEND EXPORT_TARGET_SET easy3d_algo_ext)
endif ()

if (Easy3D_HAS_FFMPEG)
    add_subdirectory(video)  # experimental
    list(APPEND CONF_Easy3D_COMPONENTS video)
    list(APPEND EXPORT_TARGET_SET easy3d_video)
endif ()

add_subdirectory(renderer)
list(APPEND CONF_Easy3D_COMPONENTS renderer)
list(APPEND EXPORT_TARGET_SET easy3d_renderer)

add_subdirectory(gui)
list(APPEND CONF_Easy3D_COMPONENTS gui)
list(APPEND EXPORT_TARGET_SET easy3d_gui)

add_subdirectory(viewer)
list(APPEND CONF_Easy3D_COMPONENTS viewer)
list(APPEND EXPORT_TARGET_SET easy3d_viewer 3rd_glfw)


export(TARGETS
        ${EXPORT_TARGET_SET}
        FILE "${Easy3D_BINARY_DIR}/Easy3DTargets.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE Easy3D)

## Create the Easy3DConfig.cmake and Easy3DConfigVersion files

# Use relative path?
#file(RELATIVE_PATH REL_INCLUDE_DIR "${Easy3D_INSTALL_CMAKE_DIR}" "${Easy3D_INSTALL_INCLUDE_DIR}")
#file(RELATIVE_PATH REL_LIB_DIR "${Easy3D_INSTALL_CMAKE_DIR}" "${Easy3D_INSTALL_LIB_DIR}")

# ... for the build tree
set(CONF_Easy3D_INCLUDE_DIRS "${Easy3D_INCLUDE_DIR}")
set(CONF_Easy3D_LIBRARY_DIRS "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
set(CONF_Easy3D_VERSION ${Easy3D_VERSION})
configure_file(../cmake/Easy3DConfig.cmake.in
        "${Easy3D_BINARY_DIR}/Easy3DConfig.cmake" @ONLY)
# ... for the install tree
set(CONF_Easy3D_INCLUDE_DIRS "${Easy3D_INSTALL_INCLUDE_DIR}")
set(CONF_Easy3D_LIBRARY_DIRS "${Easy3D_INSTALL_LIB_DIR}")
configure_file(../cmake/Easy3DConfig.cmake.in
        "${Easy3D_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/Easy3DConfig.cmake" @ONLY)
# ... for both
configure_file(../cmake/Easy3DConfigVersion.cmake.in
        "${Easy3D_BINARY_DIR}/Easy3DConfigVersion.cmake" @ONLY)

# Install the Easy3DConfig.cmake and Easy3DConfigVersion.cmake
install(FILES
        "${Easy3D_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/Easy3DConfig.cmake"
        "${Easy3D_BINARY_DIR}/Easy3DConfigVersion.cmake"
        DESTINATION "${Easy3D_INSTALL_CMAKE_DIR}" COMPONENT dev)

# Install the export set for use with the install-tree
install(EXPORT Easy3DTargets DESTINATION
        "${Easy3D_INSTALL_CMAKE_DIR}" COMPONENT dev)


# References:
#   https://gitlab.kitware.com/cmake/community/-/wikis/doc/tutorials/How-to-create-a-ProjectConfig.cmake-file