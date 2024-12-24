#include <map>
#include <algorithm>
#include <stdexcept>

#include <pybind11/pybind11.h>


void bind_easy3d_core_box(pybind11::module_ &m);
void bind_easy3d_core_constant(pybind11::module_ &m);
void bind_easy3d_core_graph(pybind11::module_ &m);
void bind_easy3d_core_line(pybind11::module_ &m);
void bind_easy3d_core_mat(pybind11::module_ &m);
void bind_easy3d_core_model(pybind11::module_ &m);
void bind_easy3d_core_plane(pybind11::module_ &m);
void bind_easy3d_core_point_cloud(pybind11::module_ &m);
void bind_easy3d_core_poly_mesh(pybind11::module_ &m);
void bind_easy3d_core_property(pybind11::module_ &m);
void bind_easy3d_core_random(pybind11::module_ &m);
void bind_easy3d_core_signal(pybind11::module_ &m);
void bind_easy3d_core_surface_mesh(pybind11::module_ &m);
void bind_easy3d_core_surface_mesh_builder(pybind11::module_ &m);
void bind_easy3d_core_types(pybind11::module_ &m);
void bind_easy3d_core_vec(pybind11::module_ &m);

void bind_easy3d_algo_collider(pybind11::module_ &m);
void bind_easy3d_algo_delaunay(pybind11::module_ &m);
void bind_easy3d_algo_extrusion(pybind11::module_ &m);
void bind_easy3d_algo_gaussian_noise(pybind11::module_ &m);
void bind_easy3d_algo_point_cloud_normals(pybind11::module_ &m);
void bind_easy3d_algo_point_cloud_poisson_reconstruction(pybind11::module_ &m);
void bind_easy3d_algo_point_cloud_ransac(pybind11::module_ &m);
void bind_easy3d_algo_point_cloud_simplification(pybind11::module_ &m);
void bind_easy3d_algo_polygon_partition(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_components(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_curvature(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_enumerator(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_factory(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_fairing(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_features(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_geodesic(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_geometry(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_hole_filling(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_parameterization(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_polygonization(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_remeshing(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_sampler(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_simplification(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_smoothing(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_stitching(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_subdivision(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_tetrahedralization(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_topology(pybind11::module_ &m);
void bind_easy3d_algo_surface_mesh_triangulation(pybind11::module_ &m);
void bind_easy3d_algo_tessellator(pybind11::module_ &m);
void bind_easy3d_algo_text_mesher(pybind11::module_ &m);
void bind_easy3d_algo_triangle_mesh_kdtree(pybind11::module_ &m);
#ifdef HAS_CGAL
void bind_easy3d_algo_ext_surfacer(pybind11::module_ &m);
#endif

void bind_easy3d_fileio_graph_io(pybind11::module_ &m);
void bind_easy3d_fileio_image_io(pybind11::module_ &m);
void bind_easy3d_fileio_point_cloud_io(pybind11::module_ &m);
void bind_easy3d_fileio_poly_mesh_io(pybind11::module_ &m);
void bind_easy3d_fileio_surface_mesh_io(pybind11::module_ &m);
void bind_easy3d_fileio_translator(pybind11::module_ &m);

//void bind_easy3d_gui_picker(pybind11::module_ &m);
//void bind_easy3d_gui_picker_model(pybind11::module_ &m);
//void bind_easy3d_gui_picker_point_cloud(pybind11::module_ &m);
//void bind_easy3d_gui_picker_surface_mesh(pybind11::module_ &m);

void bind_easy3d_kdtree_kdtree_search_ann(pybind11::module_ &m);
void bind_easy3d_kdtree_kdtree_search_eth(pybind11::module_ &m);
void bind_easy3d_kdtree_kdtree_search_flann(pybind11::module_ &m);
void bind_easy3d_kdtree_kdtree_search_nanoflann(pybind11::module_ &m);

//void bind_easy3d_renderer_constraint(pybind11::module_ &m);
void bind_easy3d_renderer_camera(pybind11::module_ &m);
//void bind_easy3d_renderer_buffer(pybind11::module_ &m);
//void bind_easy3d_renderer_clipping_plane(pybind11::module_ &m);
//void bind_easy3d_renderer_drawable_lines(pybind11::module_ &m);
//void bind_easy3d_renderer_framebuffer_object(pybind11::module_ &m);
//void bind_easy3d_renderer_frustum(pybind11::module_ &m);
//void bind_easy3d_renderer_manipulated_frame(pybind11::module_ &m);
//void bind_easy3d_renderer_opengl_error(pybind11::module_ &m);
//void bind_easy3d_renderer_opengl_timer(pybind11::module_ &m);
//void bind_easy3d_renderer_read_pixel(pybind11::module_ &m);
//void bind_easy3d_renderer_renderer(pybind11::module_ &m);
//void bind_easy3d_renderer_shader_manager(pybind11::module_ &m);
//void bind_easy3d_renderer_shape(pybind11::module_ &m);
//void bind_easy3d_renderer_shape_1(pybind11::module_ &m);
//void bind_easy3d_renderer_soft_shadow(pybind11::module_ &m);
//void bind_easy3d_renderer_texture_manager(pybind11::module_ &m);
//void bind_easy3d_renderer_transform(pybind11::module_ &m);
//void bind_easy3d_renderer_vertex_array_object(pybind11::module_ &m);

void bind_easy3d_util_console_style(pybind11::module_ &m);
void bind_easy3d_util_dialog(pybind11::module_ &m);
void bind_easy3d_util_file_system(pybind11::module_ &m);
void bind_easy3d_util_initializer(pybind11::module_ &m);
void bind_easy3d_util_line_stream(pybind11::module_ &m);
void bind_easy3d_util_logging(pybind11::module_ &m);
void bind_easy3d_util_progress(pybind11::module_ &m);
void bind_easy3d_util_resource(pybind11::module_ &m);
void bind_easy3d_util_setting(pybind11::module_ &m);
void bind_easy3d_util_stop_watch(pybind11::module_ &m);
void bind_easy3d_util_string(pybind11::module_ &m);
void bind_easy3d_util_timer(pybind11::module_ &m);
void bind_easy3d_util_tokenizer(pybind11::module_ &m);
void bind_easy3d_util_version(pybind11::module_ &m);

void bind_easy3d_viewer_viewer(pybind11::module_ &m);
void bind_easy3d_viewer_multi_viewer(pybind11::module_ &m);
void bind_easy3d_viewer_offscreen(pybind11::module_ &m);

#ifdef HAS_FFMPEG
void bind_easy3d_video_video_encoder(pybind11::module_ &m);
#endif

// Submodule declarations
void bind_core(pybind11::module_ &m) {
    bind_easy3d_core_box(m);
    bind_easy3d_core_constant(m);
    bind_easy3d_core_line(m);
    bind_easy3d_core_mat(m);
    bind_easy3d_core_plane(m);
    bind_easy3d_core_property(m);
    bind_easy3d_core_random(m);
    bind_easy3d_core_signal(m);
    bind_easy3d_core_types(m);
    bind_easy3d_core_vec(m);

    bind_easy3d_core_model(m);
    bind_easy3d_core_graph(m);
    bind_easy3d_core_surface_mesh(m);
    bind_easy3d_core_point_cloud(m);
    bind_easy3d_core_poly_mesh(m);

    bind_easy3d_core_surface_mesh_builder(m);
}

void bind_util(pybind11::module_ &m) {
    bind_easy3d_util_console_style(m);
    bind_easy3d_util_dialog(m);
    bind_easy3d_util_file_system(m);
    bind_easy3d_util_initializer(m);
    bind_easy3d_util_line_stream(m);
    bind_easy3d_util_logging(m);
    bind_easy3d_util_progress(m);
    bind_easy3d_util_resource(m);
    bind_easy3d_util_setting(m);
    bind_easy3d_util_stop_watch(m);
    bind_easy3d_util_string(m);
    bind_easy3d_util_timer(m);
    bind_easy3d_util_tokenizer(m);
    bind_easy3d_util_version(m);
}

void bind_algo(pybind11::module_ &m) {
    bind_easy3d_algo_collider(m);
    bind_easy3d_algo_delaunay(m);
    bind_easy3d_algo_extrusion(m);
    bind_easy3d_algo_gaussian_noise(m);
    bind_easy3d_algo_point_cloud_normals(m);
    bind_easy3d_algo_point_cloud_poisson_reconstruction(m);
    bind_easy3d_algo_point_cloud_ransac(m);
    bind_easy3d_algo_point_cloud_simplification(m);
    bind_easy3d_algo_polygon_partition(m);
    bind_easy3d_algo_surface_mesh_components(m);
    bind_easy3d_algo_surface_mesh_curvature(m);
    bind_easy3d_algo_surface_mesh_enumerator(m);
    bind_easy3d_algo_surface_mesh_factory(m);
    bind_easy3d_algo_surface_mesh_fairing(m);
    bind_easy3d_algo_surface_mesh_features(m);
    bind_easy3d_algo_surface_mesh_geodesic(m);
    bind_easy3d_algo_surface_mesh_geometry(m);
    bind_easy3d_algo_surface_mesh_hole_filling(m);
    bind_easy3d_algo_surface_mesh_parameterization(m);
    bind_easy3d_algo_surface_mesh_polygonization(m);
    bind_easy3d_algo_surface_mesh_remeshing(m);
    bind_easy3d_algo_surface_mesh_sampler(m);
    bind_easy3d_algo_surface_mesh_simplification(m);
    bind_easy3d_algo_surface_mesh_smoothing(m);
    bind_easy3d_algo_surface_mesh_stitching(m);
    bind_easy3d_algo_surface_mesh_subdivision(m);
    bind_easy3d_algo_surface_mesh_tetrahedralization(m);
    bind_easy3d_algo_surface_mesh_topology(m);
    bind_easy3d_algo_surface_mesh_triangulation(m);
    bind_easy3d_algo_tessellator(m);
    bind_easy3d_algo_text_mesher(m);
    bind_easy3d_algo_triangle_mesh_kdtree(m);
}

#ifdef HAS_FFMPEG
void bind_algo_ext(pybind11::module_ &m) {
    bind_easy3d_algo_ext_surfacer(m);
}
#endif

void bind_fileio(pybind11::module_ &m) {
    bind_easy3d_fileio_graph_io(m);
    bind_easy3d_fileio_image_io(m);
    bind_easy3d_fileio_point_cloud_io(m);
    bind_easy3d_fileio_poly_mesh_io(m);
    bind_easy3d_fileio_surface_mesh_io(m);
    bind_easy3d_fileio_translator(m);
}

void bind_kdtree(pybind11::module_ &m) {
    bind_easy3d_kdtree_kdtree_search_ann(m);
    bind_easy3d_kdtree_kdtree_search_eth(m);
    bind_easy3d_kdtree_kdtree_search_flann(m);
    bind_easy3d_kdtree_kdtree_search_nanoflann(m);
}

void bind_renderer(pybind11::module_ &m) {
    bind_easy3d_renderer_camera(m);
}

void bind_viewer(pybind11::module_ &m) {
    bind_easy3d_viewer_viewer(m);
    bind_easy3d_viewer_multi_viewer(m);
    bind_easy3d_viewer_offscreen(m);
}

#ifdef HAS_FFMPEG
void bind_video(pybind11::module_ &m) {
    bind_easy3d_video_video_encoder(m);
}
#endif


PYBIND11_MODULE(PyEasy3D, root_module) {
    root_module.doc() = "Python bindings for Easy3D";

#define EASY3D_FLAT_BINDING
#ifdef EASY3D_FLAT_BINDING
    /**
     * Use a single easy3d module where all classes and functions are exposed directly at the module level.
     * This allows Python users to interact with the library in a straightforward way. In this way, The API
     * is more familiar to users switching between C++ and Python, which reduces the need to navigate submodules.
     * For example:
     *      C++ usage:    easy3d::PointCloud(), easy3d::initialize()
     *      Python usage: easy3d.PointCloud(), easy3d.initialize()
     * @code
     *      import easy3d
     *
     *      easy3d.initialize(True)
     *      file_name = easy3d.directory() + "/data/bunny.bin"
     *      stopwatch = easy3d.StopWatch()
     *      pc = easy3d.PointCloudIO.load(file_name)
     *
     *      easy3d.PointCloudNormals.estimate(pc, 16, False)
     *      easy3d.PointCloudNormals.reorient(pc, 16)
     *      viewer = easy3d.Viewer("Easy3D Viewer")
     */
    bind_core(root_module);
    bind_util(root_module);
    bind_algo(root_module);

#ifdef HAS_CGAL
    bind_algo_ext(root_module);
#endif

    bind_fileio(root_module);
    bind_kdtree(root_module);
    bind_renderer(root_module);
    bind_viewer(root_module);

#ifdef HAS_FFMPEG
    bind_video(root_module);
#endif

#else
    /**
     * Organize the bindings into submodules (core, algo, util ...) to reflect the C++ library structure.
     * This way, users need to navigate easy3d.core, easy3d.util, and easy3d.algo to access related functionalities.
     * @usage
     *       import easy3d
     *       from easy3d import util
     *       from easy3d import fileio
     *       from easy3d import algo
     *       from easy3d import viewer
     *
     *       util.initialize(True)
     *       file_name = util.directory() + "/data/bunny.bin"
     *       stopwatch = util.StopWatch()
     *       pc = fileio.PointCloudIO.load(file_name)
     *       algo.PointCloudNormals.estimate(pc, 16, False)
     *       algo.PointCloudNormals.reorient(pc, 16)
     *       viewer = viewer.Viewer("Easy3D Viewer")
     */
    pybind11::module_ core     = root_module.def_submodule("core",     "Core data structures and operations");
    pybind11::module_ util     = root_module.def_submodule("util",     "Utility functions");
    pybind11::module_ algo     = root_module.def_submodule("algo",     "Algorithms for geometry processing");
#ifdef HAS_CGAL
    pybind11::module_ algo_ext = root_module.def_submodule("algo_ext", "Extended surface mesh processing algorithms");
#endif
    pybind11::module_ fileio   = root_module.def_submodule("fileio",   "Functionalities for reading/writing files");
    pybind11::module_ kdtree   = root_module.def_submodule("kdtree",   "A collection of kd-tree implementations");
    pybind11::module_ renderer = root_module.def_submodule("renderer", "Functionalities and algorithms for visualization");
    pybind11::module_ viewer   = root_module.def_submodule("viewer",   "Visualization using viewers");
#ifdef HAS_FFMPEG
    pybind11::module_ video    = root_module.def_submodule("video",    "Functionalities that encode an image sequence into a video");
#endif
    // If you want to make the submodules available directly as part of the parent module (without requiring explicit
    // import), you can add each submodule as an attribute of the main module. Then the useage can be:
    //          import easy3d
    //          easy3d.util.initialize(True)
    // root_module.attr("util") = algo;
    // ...

    bind_core(core);
    bind_util(util);
    bind_algo(algo);
#ifdef HAS_CGAL
    bind_algo_ext(algo_ext);
#endif
    bind_fileio(fileio);
    bind_kdtree(kdtree);
    bind_renderer(renderer);
    bind_viewer(viewer);
#ifdef HAS_FFMPEG
    bind_video(video);
#endif
#endif

    // Unused bindings (they are really not needed):

    // bind_easy3d_gui_picker(m);
    // bind_easy3d_gui_picker_model(m);
    // bind_easy3d_gui_picker_point_cloud(m);
    // bind_easy3d_gui_picker_surface_mesh(m);

    // bind_easy3d_renderer_constraint(m);
    // bind_easy3d_renderer_buffer(m);
    // bind_easy3d_renderer_clipping_plane(m);
    // bind_easy3d_renderer_drawable_lines(m);
    // bind_easy3d_renderer_framebuffer_object(m);
    // bind_easy3d_renderer_frustum(m);
    // bind_easy3d_renderer_manipulated_frame(m);
    // bind_easy3d_renderer_opengl_error(m);
    // bind_easy3d_renderer_opengl_timer(m);
    // bind_easy3d_renderer_read_pixel(m);
    // bind_easy3d_renderer_renderer(m);
    // bind_easy3d_renderer_shader_manager(m);
    // bind_easy3d_renderer_shape(m);
    // bind_easy3d_renderer_shape_1(m);
    // bind_easy3d_renderer_soft_shadow(m);
    // bind_easy3d_renderer_texture_manager(m);
    // bind_easy3d_renderer_transform(m);
    // bind_easy3d_renderer_vertex_array_object(m);

    // bind_3rd_party_easyloggingpp_easylogging(m);
}
