# All available Easy3D libraries
set(CONF_EASY3D_LIBRARIES)

# Add all targets to the build-tree export set
set(EXPORT_TARGET_SET)

add_subdirectory(util)
list(APPEND CONF_EASY3D_LIBRARIES "easy3d::util")
list(APPEND EXPORT_TARGET_SET "easy3d_util" "3rd_backward" "3rd_easyloggingpp")

add_subdirectory(core)
list(APPEND CONF_EASY3D_LIBRARIES "easy3d::core")
list(APPEND EXPORT_TARGET_SET "easy3d_core")

add_subdirectory(fileio)
list(APPEND CONF_EASY3D_LIBRARIES "easy3d::fileio")
list(APPEND EXPORT_TARGET_SET "easy3d_fileio" "3rd_lastools" "3rd_rply")

add_subdirectory(kdtree)
list(APPEND CONF_EASY3D_LIBRARIES "easy3d::kdtree")
list(APPEND EXPORT_TARGET_SET "easy3d_kdtree" "3rd_kdtree")

add_subdirectory(algo)
list(APPEND CONF_EASY3D_LIBRARIES "easy3d::algo")
list(APPEND EXPORT_TARGET_SET "easy3d_algo" "3rd_poisson" "3rd_ransac" "3rd_triangle" "3rd_tetgen" "3rd_glutess")

if (EASY3D_HAS_CGAL)
    add_subdirectory(algo_ext)  # experimental
    list(APPEND CONF_EASY3D_LIBRARIES "easy3d::algo_ext")
    list(APPEND EXPORT_TARGET_SET "easy3d_algo_ext")
endif ()

add_subdirectory(renderer)
list(APPEND CONF_EASY3D_LIBRARIES "easy3d::renderer")
list(APPEND EXPORT_TARGET_SET "easy3d_renderer" "3rd_glew")

add_subdirectory(gui)
list(APPEND CONF_EASY3D_LIBRARIES "easy3d::gui")
list(APPEND EXPORT_TARGET_SET "easy3d_gui")

add_subdirectory(viewer)
list(APPEND CONF_EASY3D_LIBRARIES "easy3d::viewer")
list(APPEND EXPORT_TARGET_SET "easy3d_viewer" "3rd_glfw")


export(TARGETS
        ${EXPORT_TARGET_SET}
        FILE "${PROJECT_BINARY_DIR}/Easy3DTargets.cmake")

# Export the package for use from the build-tree
# (this registers the build-tree with a global CMake-registry)
export(PACKAGE Easy3D)

## Create the Easy3DConfig.cmake and Easy3DConfigVersion files
# ... for the build tree
#set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}" "${PROJECT_BINARY_DIR}")  # also include the build directory
set(CONF_INCLUDE_DIRS "${PROJECT_SOURCE_DIR}")
configure_file(../cmake/Easy3DConfig.cmake.in
        "${PROJECT_BINARY_DIR}/Easy3DConfig.cmake" @ONLY)
# ... for the install tree
set(CONF_INCLUDE_DIRS "${CMAKE_INSTALL_PREFIX}/include")
configure_file(../cmake/Easy3DConfig.cmake.in
        "${PROJECT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/Easy3DConfig.cmake" @ONLY)
# ... for both
configure_file(../cmake/Easy3DConfigVersion.cmake.in
        "${PROJECT_BINARY_DIR}/Easy3DConfigVersion.cmake" @ONLY)



# TODO: Install Easy3D and use Easy3D from its install tree?
# https://gitlab.kitware.com/cmake/community/-/wikis/doc/tutorials/How-to-create-a-ProjectConfig.cmake-file