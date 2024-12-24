cmake_minimum_required(VERSION 3.12)
project(PyEasy3D)

set(CMAKE_CXX_STANDARD 11)

find_package(pybind11 REQUIRED)

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
        "bindings/easy3d/core/random.cpp"
        "bindings/easy3d/core/signal.cpp"
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

        "bindings/easy3d/util/console_style.cpp"
        "bindings/easy3d/util/dialog.cpp"
        "bindings/easy3d/util/file_system.cpp"
        "bindings/easy3d/util/initializer.cpp"
        "bindings/easy3d/util/line_stream.cpp"
        "bindings/easy3d/util/logging.cpp"
        "bindings/easy3d/util/progress.cpp"
        "bindings/easy3d/util/resource.cpp"
        "bindings/easy3d/util/setting.cpp"
        "bindings/easy3d/util/stop_watch.cpp"
        "bindings/easy3d/util/string.cpp"
        "bindings/easy3d/util/timer.cpp"
        "bindings/easy3d/util/tokenizer.cpp"
        "bindings/easy3d/util/version.cpp"

        "bindings/easy3d/viewer/multi_viewer.cpp"
        "bindings/easy3d/viewer/offscreen.cpp"
        "bindings/easy3d/viewer/viewer.cpp"

#        Seems never needed
#        "bindings/easy3d/gui/picker.cpp"
#        "bindings/easy3d/gui/picker_model.cpp"
#        "bindings/easy3d/gui/picker_point_cloud.cpp"
#        "bindings/easy3d/gui/picker_surface_mesh.cpp"
#        "bindings/easy3d/renderer/ambient_occlusion.cpp"
#        "bindings/easy3d/renderer/buffer.cpp"
#        "bindings/easy3d/renderer/clipping_plane.cpp"
#        "bindings/easy3d/renderer/constraint.cpp"
#        "bindings/easy3d/renderer/drawable_lines.cpp"
#        "bindings/easy3d/renderer/eye_dome_lighting.cpp"
#        "bindings/easy3d/renderer/framebuffer_object.cpp"
#        "bindings/easy3d/renderer/frustum.cpp"
#        "bindings/easy3d/renderer/key_frame_interpolator.cpp"
#        "bindings/easy3d/renderer/manipulated_frame.cpp"
#        "bindings/easy3d/renderer/opengl_error.cpp"
#        "bindings/easy3d/renderer/opengl_timer.cpp"
#        "bindings/easy3d/renderer/read_pixel.cpp"
#        "bindings/easy3d/renderer/renderer.cpp"
#        "bindings/easy3d/renderer/shader_manager.cpp"
#        "bindings/easy3d/renderer/shader_program.cpp"
#        "bindings/easy3d/renderer/shadow.cpp"
#        "bindings/easy3d/renderer/shape.cpp"
#        "bindings/easy3d/renderer/soft_shadow.cpp"
#        "bindings/easy3d/renderer/state.cpp"
#        "bindings/easy3d/renderer/text_renderer.cpp"
#        "bindings/easy3d/renderer/texture.cpp"
#        "bindings/easy3d/renderer/texture_manager.cpp"
#        "bindings/easy3d/renderer/transform.cpp"
#        "bindings/easy3d/renderer/transparency.cpp"
#        "bindings/easy3d/renderer/vertex_array_object.cpp"
        )

if(Easy3D_HAS_CGAL)
    list(APPEND BINDING_SOURCES "bindings/easy3d/algo_ext/surfacer.cpp")
endif()

if(Easy3D_HAS_FFMPEG)
    list(APPEND BINDING_SOURCES "bindings/easy3d/video/video_encoder.cpp")
endif()

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
if (Easy3D_HAS_CGAL)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_CGAL)
    target_link_libraries(${PROJECT_NAME} PRIVATE easy3d::algo_ext)
endif ()
if (Easy3D_HAS_FFMPEG)
    target_compile_definitions(${PROJECT_NAME} PRIVATE HAS_FFMPEG)
    target_link_libraries(${PROJECT_NAME} PRIVATE easy3d::video)
endif ()

target_compile_definitions(${PROJECT_NAME} PRIVATE PYBIND11_SIMPLE_GIL_SAFE_ABI) # to use the Python Stable ABI
# By default, the output .so will include the Python version. Use a CMake command to rename it to a version-independent name
set_target_properties(${PROJECT_NAME} PROPERTIES
        SUFFIX ".so"
        FOLDER "python"
        )


# The following commands are actually not necessary for generating bindings.
# They are here to make it easier for the installation of the generated Python module.

# Specify the output directory for the Python package
set(PYTHON_PACKAGE_DIR "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/python/easy3d")

# Ensure the directory exists
file(MAKE_DIRECTORY ${PYTHON_PACKAGE_DIR})

# Generate the __init__.py file
file(WRITE ${PYTHON_PACKAGE_DIR}/__init__.py
        "# Alias for the PyEasy3D module\n"
        "try:\n"
        "    from .PyEasy3D import *\n"
        "except ImportError:\n"
        "    from PyEasy3D import *\n"
        )

# Path to the compiled PyEasy3D module
set(COMPILED_MODULE "${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.so")

# Copy the compiled PyEasy3D module to the Python package directory
add_custom_command(
        TARGET PyEasy3D POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy ${COMPILED_MODULE} ${PYTHON_PACKAGE_DIR}/
        COMMENT "Copying PyEasy3D module to Python package"
)

# Copy setup.py to the Python package directory
set(CONF_Easy3D_VERSION ${Easy3D_VERSION_STRING})
configure_file(${CMAKE_CURRENT_LIST_DIR}/setup.py.in ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/python/setup.py @ONLY)