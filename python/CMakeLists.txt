cmake_minimum_required(VERSION 3.12)
project(PyEasy3D)

set(CMAKE_CXX_STANDARD 11)  # If you use pybind11::overload_cast<...>, then it requires compiling in C++14 mode

#file(GLOB_RECURSE BINDING_SOURCES "bindings/*.cpp")
#string(REPLACE ";" "\n" BINDING_SOURCES_NEWLINE "${BINDING_SOURCES}")   # Replace spaces with newlines for better readability
#message(STATUS "BINDING_SOURCES:\n${BINDING_SOURCES_NEWLINE}")          # Print each file name on a new line

set(BINDING_SOURCES
        "bindings/easy3d.cpp"

        "bindings/easy3d/algo/collider.cpp"
        "bindings/easy3d/algo/delaunay.cpp"
        "bindings/easy3d/algo/extrusion.cpp"
        "bindings/easy3d/algo/gaussian_noise.cpp"
        "bindings/easy3d/algo/point_cloud_normals.cpp"
        "bindings/easy3d/algo/point_cloud_poisson_reconstruction.cpp"
        "bindings/easy3d/algo/point_cloud_ransac.cpp"
        "bindings/easy3d/algo/point_cloud_simplification.cpp"
        "bindings/easy3d/algo/polygon_partition.cpp"
        "bindings/easy3d/algo/surface_mesh_components.cpp"
        "bindings/easy3d/algo/surface_mesh_curvature.cpp"
        "bindings/easy3d/algo/surface_mesh_enumerator.cpp"
        "bindings/easy3d/algo/surface_mesh_factory.cpp"
        "bindings/easy3d/algo/surface_mesh_fairing.cpp"
        "bindings/easy3d/algo/surface_mesh_features.cpp"
        "bindings/easy3d/algo/surface_mesh_geodesic.cpp"
        "bindings/easy3d/algo/surface_mesh_geometry.cpp"
        "bindings/easy3d/algo/surface_mesh_hole_filling.cpp"
        "bindings/easy3d/algo/surface_mesh_parameterization.cpp"
        "bindings/easy3d/algo/surface_mesh_polygonization.cpp"
        "bindings/easy3d/algo/surface_mesh_remeshing.cpp"
        "bindings/easy3d/algo/surface_mesh_sampler.cpp"
        "bindings/easy3d/algo/surface_mesh_simplification.cpp"
        "bindings/easy3d/algo/surface_mesh_smoothing.cpp"
        "bindings/easy3d/algo/surface_mesh_stitching.cpp"
        "bindings/easy3d/algo/surface_mesh_subdivision.cpp"
        "bindings/easy3d/algo/surface_mesh_tetrahedralization.cpp"
        "bindings/easy3d/algo/surface_mesh_topology.cpp"
        "bindings/easy3d/algo/surface_mesh_triangulation.cpp"
        "bindings/easy3d/algo/tessellator.cpp"
        "bindings/easy3d/algo/text_mesher.cpp"
        "bindings/easy3d/algo/triangle_mesh_kdtree.cpp"

        "bindings/easy3d/core/box.cpp"
        "bindings/easy3d/core/constant.cpp"
        "bindings/easy3d/core/graph.cpp"
        "bindings/easy3d/core/line.cpp"
        "bindings/easy3d/core/mat.cpp"
        "bindings/easy3d/core/model.cpp"
        "bindings/easy3d/core/plane.cpp"
        "bindings/easy3d/core/point_cloud.cpp"
        "bindings/easy3d/core/poly_mesh.cpp"
        "bindings/easy3d/core/property.cpp"
        "bindings/easy3d/core/quat.cpp"
        "bindings/easy3d/core/random.cpp"
        "bindings/easy3d/core/surface_mesh.cpp"
        "bindings/easy3d/core/surface_mesh_builder.cpp"
        "bindings/easy3d/core/types.cpp"
        "bindings/easy3d/core/vec.cpp"

        "bindings/easy3d/fileio/graph_io.cpp"
        "bindings/easy3d/fileio/image_io.cpp"
        "bindings/easy3d/fileio/point_cloud_io.cpp"
        "bindings/easy3d/fileio/poly_mesh_io.cpp"
        "bindings/easy3d/fileio/surface_mesh_io.cpp"
        "bindings/easy3d/fileio/translator.cpp"

        "bindings/easy3d/kdtree/kdtree_search_eth.cpp"
        "bindings/easy3d/kdtree/kdtree_search_ann.cpp"
        "bindings/easy3d/kdtree/kdtree_search_flann.cpp"
        "bindings/easy3d/kdtree/kdtree_search_nanoflann.cpp"

        "bindings/easy3d/renderer/camera.cpp"
        "bindings/easy3d/renderer/drawables.cpp"
        "bindings/easy3d/renderer/renderer.cpp"
        "bindings/easy3d/renderer/state.cpp"

        "bindings/easy3d/util/dialog.cpp"
        "bindings/easy3d/util/file_system.cpp"
        "bindings/easy3d/util/initializer.cpp"
        "bindings/easy3d/util/logging.cpp"
        "bindings/easy3d/util/resource.cpp"
        "bindings/easy3d/util/setting.cpp"
        "bindings/easy3d/util/stop_watch.cpp"
        "bindings/easy3d/util/string.cpp"
        "bindings/easy3d/util/version.cpp"

        "bindings/easy3d/viewer/multi_viewer.cpp"
        "bindings/easy3d/viewer/offscreen.cpp"
        "bindings/easy3d/viewer/viewer.cpp"
        )

#if(Easy3D_HAS_CGAL)
#    list(APPEND BINDING_SOURCES "bindings/unused/algo_ext/surfacer.cpp")
#endif()
#
#if(Easy3D_HAS_FFMPEG)
#    list(APPEND BINDING_SOURCES "bindings/unused/video/video_encoder.cpp")
#endif()

pybind11_add_module(${PROJECT_NAME} MODULE ${BINDING_SOURCES})
target_link_libraries(${PROJECT_NAME} PRIVATE
        easy3d::algo
        easy3d::core
        easy3d::fileio
        easy3d::gui
        easy3d::kdtree
        easy3d::renderer
        easy3d::util
        easy3d::viewer
        )
#if (Easy3D_HAS_CGAL)
#    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_CGAL)
#    target_link_libraries(${PROJECT_NAME} PRIVATE easy3d::algo_ext)
#endif ()
#if (Easy3D_HAS_FFMPEG)
#    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_FFMPEG)
#    target_link_libraries(${PROJECT_NAME} PRIVATE easy3d::video)
#endif ()

target_compile_definitions(${PROJECT_NAME} PRIVATE PYBIND11_SIMPLE_GIL_SAFE_ABI) # to use the Python Stable ABI

set(MODULE_NAME "PyEasy3D") # Sets the base name of the output file
target_compile_definitions(${PROJECT_NAME} PUBLIC "PyEasy3D_NAME=${MODULE_NAME}")

# Platform-specific suffix for Python extensions
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(MODULE_SUFFIX ".pyd")
else()
    set(MODULE_SUFFIX ".so")
endif()
# Set the target properties with the computed suffix
set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME ${MODULE_NAME}  # Sets the base name of the output file
        PREFIX ""                   # Avoid prefixes like 'lib'
        SUFFIX "${MODULE_SUFFIX}"   # Use the computed suffix
        FOLDER "python"
        )

     

# The following commands are actually not necessary for generating bindings.
# They are here to make it easier for the installation of the generated Python module.

# Ensure the directory exists
file(MAKE_DIRECTORY ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d)

# Generate the __init__.py file for the Python package, which imports the compiled module
file(WRITE ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d/__init__.py
        "# Alias for the ${MODULE_NAME} module\n"
        "try:\n"
        "    from .${MODULE_NAME} import *\n"
        "except ImportError:\n"
        "    from ${MODULE_NAME} import *\n"
        )

# Path to the compiled module
set(COMPILED_MODULE "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${MODULE_NAME}${MODULE_SUFFIX}")

# Post-build command to copy the compiled module to the Python package directory.
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying the compiled module ${MODULE_NAME}${MODULE_SUFFIX} to ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d/..."
        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${PROJECT_NAME}> ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d/
        COMMAND_EXPAND_LISTS
        )

# Post-build command to copy the dependencies of the compiled module to the Python package directory.
if (MSVC) #MSVC on Windows. TARGET_RUNTIME_DLLS is only available on Windows.
    # TARGET_RUNTIME_DLLS: Automatically finds all DLL dependencies of the target.
    # TARGET_FILE_DIR: Refers to the output directory of the target file.
    # COMMAND_EXPAND_LISTS: Ensures that the DLL list expands correctly in the cmake -E copy command.
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Copying the runtime DLLs of ${MODULE_NAME}${MODULE_SUFFIX} to ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d/..."
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d/
            # Liangliang: normally we need this:
            # COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${PROJECT_NAME}> $<TARGET_FILE_DIR:${PROJECT_NAME}>
            COMMAND_EXPAND_LISTS
            )
    if(PYTHON_RUNTIME_DLL)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Copying Python runtime DLL ${PYTHON_RUNTIME_DLL_NAME} to ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d/..."
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PYTHON_RUNTIME_DLL} ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d/
                COMMAND_EXPAND_LISTS
                )
    else ()
        message(WARNING "Python runtime DLL (${PYTHON_RUNTIME_DLL_NAME}) not found. This may be due to:\n"
                " - Python is installed but its runtime path is not in the PATH environment variable. You need to add it manually.\n"
                " - Python is not installed. You can download the installer from https://www.python.org/downloads/")
    endif()

else() #GCC on macOS/Linux. Other platforms may need to adjust this.
    # Liangliang: the following command should work on all Windows/macOS/Linux. Commented out because not needed for non-Windows.
#    add_custom_command(
#            TARGET ${PROJECT_NAME} POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E echo "Copying ${PROJECT_NAME} module and dependencies to Python package"
#    #        COMMAND ${CMAKE_COMMAND} -E make_directory ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d/   # Already created by "file(MAKE_DIRECTORY ...)"
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:easy3d::algo> ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d/
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:easy3d::core> ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d/
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:easy3d::fileio> ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d/
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:easy3d::gui> ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d/
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:easy3d::kdtree> ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d/
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:easy3d::renderer> ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d/
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:easy3d::util> ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d/
#            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:easy3d::viewer> ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d/
#    #        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:OtherDependency> ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d/
#    )
endif()


# Copy pyproject.toml to the Python package directory
set(CONF_Easy3D_VERSION ${Easy3D_VERSION_STRING})
configure_file(${CMAKE_CURRENT_LIST_DIR}/private/installation/pyproject.toml.in ${Easy3D_PYTHON_PACKAGE_DIR}/pyproject.toml @ONLY)
configure_file(${CMAKE_CURRENT_LIST_DIR}/ReadMe.md ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/python/ReadMe.md) # Copy the readme