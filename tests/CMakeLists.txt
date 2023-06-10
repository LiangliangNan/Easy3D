add_executable(Tests
        test_timer.cpp
        test_signal.cpp
        test_console_style.cpp
        test_kdtree.cpp
        graph.cpp
        linear_solvers.cpp
        main.cpp
        multithread.cpp
        point_cloud.cpp
        point_cloud_algorithms.cpp
        polyhedral_mesh.cpp
        spline.cpp
        surface_mesh.cpp
        surface_mesh_algorithms.cpp
        # ui, viewer, and camera
        visualization_viewer_imgui/main.cpp
        visualization_viewer_imgui/viewer.h
        visualization_viewer_imgui/viewer.cpp
        visualization_multi_view/main.cpp
        visualization_camera_interpolation/main.cpp
        visualization_camera_interpolation/viewer.h
        visualization_camera_interpolation/viewer.cpp
        visualization_real_camera/main.cpp
        visualization_real_camera/viewer.h
        visualization_real_camera/viewer.cpp
        visualization_real_camera/read_bundler_file.cpp
        # drawable objects and basic rendering
        visualization_drawables/main.cpp
        visualization_imposters/main.cpp
        visualization_scalar_field/main.cpp
        visualization_vector_field/main.cpp
        visualization_texture/main.cpp
        visualization_image/main.cpp
        visualization_image/viewer.h
        visualization_image/viewer.cpp
        visualization_tessellator/main.cpp
        visualization_convex_partition/main.cpp
        visualization_cross_section/main.cpp
        visualization_cross_section/viewer.h
        visualization_cross_section/viewer.cpp
        visualization_texture_mesh/main.cpp
        visualization_texture_mesh/viewer.h
        visualization_texture_mesh/viewer.cpp
        visualization_text_rendering/main.cpp
        visualization_text_rendering/viewer.h
        visualization_text_rendering/viewer.cpp
        visualization_text_mesher/main.cpp
        visualization_animation/main.cpp
        # user interaction
        visualization_model_picker/main.cpp
        visualization_model_picker/viewer.h
        visualization_model_picker/viewer.cpp
        visualization_face_picker/main.cpp
        visualization_face_picker/viewer.h
        visualization_face_picker/viewer.cpp
        visualization_point_selection/main.cpp
        visualization_point_selection/viewer.h
        visualization_point_selection/viewer.cpp
        # rendering techniques
        visualization_ambient_occlusion/main.cpp
        visualization_ambient_occlusion/viewer.h
        visualization_ambient_occlusion/viewer.cpp
        visualization_hard_shadow/main.cpp
        visualization_hard_shadow/viewer.h
        visualization_hard_shadow/viewer.cpp
        visualization_soft_shadow/main.cpp
        visualization_soft_shadow/viewer.h
        visualization_soft_shadow/viewer.cpp
        visualization_transparency/main.cpp
        visualization_transparency/viewer.h
        visualization_transparency/viewer.cpp
        visualization_eye_dome_lighting/main.cpp
        visualization_eye_dome_lighting/viewer.h
        visualization_eye_dome_lighting/viewer.cpp
        visualization_depth_maps/main.cpp
        visualization_depth_maps/viewer.h
        visualization_depth_maps/viewer.cpp
        )

set_target_properties(Tests PROPERTIES FOLDER "tests")

target_include_directories(Tests PRIVATE ${Easy3D_INCLUDE_DIR})


target_link_libraries(Tests 3rd_imgui easy3d::util easy3d::core easy3d::fileio easy3d::gui easy3d::kdtree easy3d::renderer easy3d::viewer easy3d::algo)
if (Easy3D_HAS_CGAL)
    target_link_libraries(Tests easy3d::algo_ext)
endif ()