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

set(EASY3D_MODULE_NAME "PyEasy3D") # Sets the base name of the output file
target_compile_definitions(${PROJECT_NAME} PUBLIC "PyEasy3D_NAME=${EASY3D_MODULE_NAME}")

if (MINGW)
    target_compile_options(${PROJECT_NAME} PRIVATE -O3)   # solves "file too big" and "too many sections (85427)" with MinGW
endif ()

# Platform-specific suffix for Python extensions
if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
    set(EASY3D_MODULE_SUFFIX ".pyd")
    set_target_properties(${PROJECT_NAME} PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}) # put it together with exes/dlls
else()
    set(EASY3D_MODULE_SUFFIX ".so")
endif()

# Set the target properties with the computed suffix
set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_NAME ${EASY3D_MODULE_NAME}  # Sets the base name of the output file
        SUFFIX "${EASY3D_MODULE_SUFFIX}"   # Use the computed suffix
        FOLDER "python"
        )

# RPATH settings for macOS and Linux. This ensures the library looks for dependencies relative to the location of the module.
# Attention: I am not sure if this may has some impact on the subsequent deployment process.
if (APPLE)
    # For macOS, @loader_path ensures the library looks for dependencies relative to the location of the module.
    set_target_properties(${PROJECT_NAME} PROPERTIES BUILD_RPATH "@loader_path" INSTALL_RPATH "@loader_path")
elseif (UNIX)
    # For Linux, $ORIGIN ensures the library looks for dependencies relative to the location of the module.
    set_target_properties(${PROJECT_NAME} PROPERTIES BUILD_RPATH "$ORIGIN"      INSTALL_RPATH "$ORIGIN")
endif ()



# ----------------------------------------------------------------------------------------------------------------------
# The code below is actually not necessary for generating bindings.
# They are here to make building the whl file easier, for the generated Python module.
# ----------------------------------------------------------------------------------------------------------------------

# Post-build command to copy the compiled module and its dependencies to the Python package directory.
function(copy_runtime_dependencies target destination)
    if (WIN32)  # MSVC or MINGW
        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory ${destination}
                # -----------------------------------------------------------------------------------------------
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${target}> ${destination}
                # -----------------------------------------------------------------------------------------------
                COMMAND ${CMAKE_COMMAND} -E echo "Copying runtime DLLs of ${target} to ${destination}..."
                # -----------------------------------------------------------------------------------------------
                # Liangliang: both "dumpbin /DEPENDENTS" and "$<TARGET_RUNTIME_DLLS:${target}>" only resolve direct dependencies.
                # COMMAND dumpbin /DEPENDENTS $<TARGET_FILE:${target}> > ${destination}/runtime_dependencies.txt
                # COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_RUNTIME_DLLS:${target}> ${destination}
                # ----------
                # Using a Python script (copy_dependencies.py) to resolve transitive dependencies.
                # This script should also work for macOS and Linux!!!
                COMMAND ${PYTHON_EXECUTABLE} ${CMAKE_CURRENT_LIST_DIR}/helper/copy_dependencies.py $<TARGET_FILE:${target}>  ${destination}
                # -----------------------------------------------------------------------------------------------
                # Copy the Python runtime DLL
                COMMAND ${CMAKE_COMMAND} -E echo "Copying Python runtime DLL ${PYTHON_RUNTIME_DLL} to ${destination}..."
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PYTHON_RUNTIME_DLL} ${destination}
                # -----------------------------------------------------------------------------------------------
                COMMAND_EXPAND_LISTS
                )
    elseif (APPLE) # macOS
        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory ${destination}
                # -----------------------------------------------------------------------------------------------
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${target}> ${destination}
                # -----------------------------------------------------------------------------------------------
                COMMAND ${CMAKE_COMMAND} -E echo "Copying runtime dependencies of ${target} to ${destination}..."
                # Liangliang: It took me so much time to figure out how to find and copy the dependencies using `add_custom_command`.
                # Write the dependencies of the target to a file:
                #   (a) otool -L $<TARGET_FILE:${target}>: Lists the dynamic library dependencies of the target file (regardless of file existence).
                #   (b) awk '/@rpath|\\/|\\// {print $$1}': Extracts the first column (the library paths) from the output of otool.
                #   (c) sed 's|@rpath|$<TARGET_FILE_DIR:${target}>|g': Replaces @rpath with the target file directory.
                #   (d) grep -v "^$<TARGET_FILE:${target}>$": Removes the line containing the target file. That is the heading line of the otool
                #       output, e.g., "/.../Release/lib/PyPolyFit.so:", which is not shown when using the command in terminal.
                COMMAND otool -L $<TARGET_FILE:${target}> | awk '/@rpath|\\/|\\// {print $$1}' | sed 's|@rpath|$<TARGET_FILE_DIR:${target}>|g' | grep -v "^$<TARGET_FILE:${target}>$" > ${destination}/runtime_dependencies.txt
                # Copy the dependencies of the target to the destination directory:
                #   (e) xargs -I {} cp -v {} ${destination}: Copies each library file to the destination directory. If does not exist, it will show e.g., "cp: /usr/lib/libc++.1.dylib: No such file or directory".
                #   (f) || true: Ignores the error during the copy process. This ensures that the command does not cause the build to fail.
                COMMAND otool -L $<TARGET_FILE:${target}> | awk '/@rpath|\\/|\\// {print $$1}' | sed 's|@rpath|$<TARGET_FILE_DIR:${target}>|g' | grep -v "^$<TARGET_FILE:${target}>$" | xargs -I {} cp -v {} ${destination} || true
                #   (g) xargs -I {} sh -c '[ -e "{}" ] && cp -v "{}" ${destination}': Ensures only existing files are copied to the destination directory. Thus it avoids the "No such file or directory" message.
                #COMMAND otool -L $<TARGET_FILE:${target}> | awk '/@rpath|\\/|\\// {print $$1}' | sed 's|@rpath|$<TARGET_FILE_DIR:${target}>|g' | xargs -I {} sh -c '[ -e "{}" ] && cp -v "{}" ${destination}' || true
                # -----------------------------------------------------------------------------------------------
                # Copy the Python runtime library
                COMMAND ${CMAKE_COMMAND} -E echo "Copying Python runtime library ${PYTHON_LIBRARIES} to ${destination}..."
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PYTHON_LIBRARIES} ${destination}
                # -----------------------------------------------------------------------------------------------
                COMMAND_EXPAND_LISTS
                )
    elseif (UNIX) # Linux
        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory ${destination}
                # -----------------------------------------------------------------------------------------------
                COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${target}> ${destination}
                # -----------------------------------------------------------------------------------------------
                COMMAND ${CMAKE_COMMAND} -E echo "Copying runtime dependencies of ${target} to ${destination}..."
                # Liangliang: It took me so much time to figure out how to find and copy the dependencies using `add_custom_command`.
                COMMAND ldd $<TARGET_FILE:${target}> | awk '/=>/ {print $$3}' > ${destination}/runtime_dependencies.txt
                COMMAND ldd $<TARGET_FILE:${target}> | grep '=> /' | awk '{print $$3}' | xargs -I {} cp -v {} ${destination}
                # -----------------------------------------------------------------------------------------------
                # Copy the Python runtime library
                COMMAND ${CMAKE_COMMAND} -E echo "Copying Python runtime library ${PYTHON_LIBRARIES} to ${destination}..."
                COMMAND ${CMAKE_COMMAND} -E copy_if_different ${PYTHON_LIBRARIES} ${destination}
                # -----------------------------------------------------------------------------------------------
                COMMAND_EXPAND_LISTS
                )
    endif ()
endfunction()

# Use the above function to copy easy3d python module and its dependencies to the Python package directory.
copy_runtime_dependencies(${PROJECT_NAME} ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d)


# Configure and create pyproject.toml
configure_file(${CMAKE_CURRENT_LIST_DIR}/pyproject.toml.in ${Easy3D_PYTHON_PACKAGE_DIR}/pyproject.toml @ONLY)

# Copy the resource files
file(COPY ${Easy3D_ROOT}/resources DESTINATION ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d)
# Copy the readme file
file(COPY ${CMAKE_CURRENT_LIST_DIR}/README.md DESTINATION ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d)
# Copy the license files
file(COPY ${Easy3D_ROOT}/LICENSE DESTINATION ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d)

# Generate the __init__.py file for the Python package
file(WRITE ${Easy3D_PYTHON_PACKAGE_DIR}/easy3d/__init__.py
        "# Alias for the ${EASY3D_MODULE_NAME} module\n"
        "import os\n"
        "\n"
        "try:\n"
        "    from .${EASY3D_MODULE_NAME} import *\n"
        "except ImportError:\n"
        "    from ${EASY3D_MODULE_NAME} import *\n"
        "\n"
        "# Get the path of the current module (__init__.py)\n"
        "module_path = os.path.abspath(__file__)\n"
        "# The resources reside in the directory containing the module\n"
        "resource_dir = os.path.join(os.path.dirname(module_path), \"resources\")\n"
        "# Initialize the resource directory\n"
        "initialize_resource_directory(resource_dir)\n"
        )