cmake_minimum_required(VERSION 3.1)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})


set(${PROJECT_NAME}_HEADERS
        ambient_occlusion.h
        average_color_blending.h
        camera.h
        clipping_plane.h
        constraint.h
        drawable.h
        drawable_lines.h
        drawable_points.h
        drawable_triangles.h
        dual_depth_peeling.h
        eye_dome_lighting.h
        frame.h
        framebuffer_object.h
        frustum.h
        key_frame_interpolator.h
        manipulated_camera_frame.h
        manipulated_frame.h
        model.h
        opengl.h
        opengl_error.h
        opengl_info.h
        primitives.h
        read_pixel.h
        renderer.h
        setting.h
        shader_manager.h
        shader_program.h
        shadow.h
        soft_shadow.h
        opengl_timer.h
        tessellator.h
        texture.h
        transform.h
        transform_decompose.h
        transparency.h
        vertex_array_object.h
        viewer.h
        )

set(${PROJECT_NAME}_SOURCES
        ambient_occlusion.cpp
        average_color_blending.cpp
        camera.cpp
        clipping_plane.cpp
        constraint.cpp
        drawable.cpp
        drawable_lines.cpp
        drawable_points.cpp
        drawable_triangles.cpp
        dual_depth_peeling.cpp
        eye_dome_lighting.cpp
        frame.cpp
        framebuffer_object.cpp
        frustum.cpp
        key_frame_interpolator.cpp
        manipulated_camera_frame.cpp
        manipulated_frame.cpp
        model.cpp
        opengl_error.cpp
        opengl_info.cpp
        primitives.cpp
        read_pixel.cpp
        renderer.cpp
        setting.cpp
        shader_manager.cpp
        shader_program.cpp
        shadow.cpp
        soft_shadow.cpp
        tessellator.cpp
        texture.cpp
        opengl_timer.cpp
        transform.cpp
        transform_decompose.cpp
        vertex_array_object.cpp
        viewer.cpp
        )


set(${PROJECT_NAME}_SHADERS
        # points
        ../../resources/shaders/points/points_plain_color.vert
        ../../resources/shaders/points/points_plain_color.frag
        ../../resources/shaders/points/points_plain_texture.vert
        ../../resources/shaders/points/points_plain_texture.frag
        ../../resources/shaders/points/points_spheres_geometry_color.vert
        ../../resources/shaders/points/points_spheres_geometry_color.geom
        ../../resources/shaders/points/points_spheres_geometry_color.frag
        ../../resources/shaders/points/points_spheres_geometry_texture.vert
        ../../resources/shaders/points/points_spheres_geometry_texture.geom
        ../../resources/shaders/points/points_spheres_geometry_texture.frag
        ../../resources/shaders/points/points_spheres_sprite_color.vert
        ../../resources/shaders/points/points_spheres_sprite_color.frag
        ../../resources/shaders/points/points_surfel_color.vert
        ../../resources/shaders/points/points_surfel_color.geom
        ../../resources/shaders/points/points_surfel_color.frag
        ../../resources/shaders/points/points_surfel_texture.vert
        ../../resources/shaders/points/points_surfel_texture.geom
        ../../resources/shaders/points/points_surfel_texture.frag
        # lines
        ../../resources/shaders/lines/lines_plain_color.vert
        ../../resources/shaders/lines/lines_plain_color.frag
        ../../resources/shaders/lines/lines_plain_texture.vert
        ../../resources/shaders/lines/lines_plain_texture.frag
        ../../resources/shaders/lines/lines_plain_color_width_control.vert
        ../../resources/shaders/lines/lines_plain_color_width_control.geom
        ../../resources/shaders/lines/lines_plain_color_width_control.frag
        ../../resources/shaders/lines/lines_plain_texture_width_control.vert
        ../../resources/shaders/lines/lines_plain_texture_width_control.geom
        ../../resources/shaders/lines/lines_plain_texture_width_control.frag
        ../../resources/shaders/lines/lines_cylinders_color.vert
        ../../resources/shaders/lines/lines_cylinders_color.geom
        ../../resources/shaders/lines/lines_cylinders_color.frag
        ../../resources/shaders/lines/lines_cylinders_texture.vert
        ../../resources/shaders/lines/lines_cylinders_texture.geom
        ../../resources/shaders/lines/lines_cylinders_texture.frag
        ../../resources/shaders/lines/lines_cones_color.vert
        ../../resources/shaders/lines/lines_cones_color.geom
        ../../resources/shaders/lines/lines_cones_color.frag
        ../../resources/shaders/lines/lines_cones_texture.vert
        ../../resources/shaders/lines/lines_cones_texture.geom
        ../../resources/shaders/lines/lines_cones_texture.frag
       # surface
        ../../resources/shaders/surface/surface_color.vert
        ../../resources/shaders/surface/surface_color.frag
        ../../resources/shaders/surface/surface_texture.vert
        ../../resources/shaders/surface/surface_texture.frag
        # EDL
        ../../resources/shaders/edl/edl_bilateral_filter.vert
        ../../resources/shaders/edl/edl_bilateral_filter.frag
        ../../resources/shaders/edl/edl_compose.vert
        ../../resources/shaders/edl/edl_compose.frag
        ../../resources/shaders/edl/edl_shade.vert
        ../../resources/shaders/edl/edl_shade.frag
        # screen space
        ../../resources/shaders/screen_space/screen_space_color.vert
        ../../resources/shaders/screen_space/screen_space_color.frag
        ../../resources/shaders/screen_space/screen_space_texture.frag
        ../../resources/shaders/screen_space/screen_space_texture.vert
        ../../resources/shaders/screen_space/screen_space_depth_texture.vert
        ../../resources/shaders/screen_space/screen_space_depth_texture.frag
        # shadow
        ../../resources/shaders/shadow/shadow_generate.vert
        ../../resources/shaders/shadow/shadow_generate.frag
        ../../resources/shaders/shadow/shadow_rendering.vert
        ../../resources/shaders/shadow/shadow_rendering.frag
        ../../resources/shaders/shadow/soft_shadow_generate.vert
        ../../resources/shaders/shadow/soft_shadow_generate.frag
        ../../resources/shaders/shadow/soft_shadow_rendering.vert
        ../../resources/shaders/shadow/soft_shadow_rendering.frag
        # ssao
        ../../resources/shaders/ssao/blur.vert
        ../../resources/shaders/ssao/blur.frag
        ../../resources/shaders/ssao/geometry_pass.vert
        ../../resources/shaders/ssao/geometry_pass.frag
        ../../resources/shaders/ssao/ssao.vert
        ../../resources/shaders/ssao/ssao.frag
        # transparency
        ../../resources/shaders/transparency/average_color_blend.vert
        ../../resources/shaders/transparency/average_color_blend.frag
        ../../resources/shaders/transparency/average_color_init.vert
        ../../resources/shaders/transparency/average_color_init.frag
        ../../resources/shaders/transparency/dual_depth_peeling_blend.vert
        ../../resources/shaders/transparency/dual_depth_peeling_blend.frag
        ../../resources/shaders/transparency/dual_depth_peeling_final.vert
        ../../resources/shaders/transparency/dual_depth_peeling_final.frag
        ../../resources/shaders/transparency/dual_depth_peeling_init.vert
        ../../resources/shaders/transparency/dual_depth_peeling_init.frag
        ../../resources/shaders/transparency/dual_depth_peeling_peel_color.vert
        ../../resources/shaders/transparency/dual_depth_peeling_peel_color.frag
        ../../resources/shaders/transparency/dual_depth_peeling_peel_texture.vert
        ../../resources/shaders/transparency/dual_depth_peeling_peel_texture.frag
        #selection
        ../../resources/shaders/selection/selection_single_primitive.vert
        ../../resources/shaders/selection/selection_single_primitive.frag
        ../../resources/shaders/selection/selection_pointcloud_rect.vert
        ../../resources/shaders/selection/selection_pointcloud_rect.frag
        ../../resources/shaders/selection/selection_pointcloud_lasso.vert
        ../../resources/shaders/selection/selection_pointcloud_lasso.frag
        )

add_library(${PROJECT_NAME} STATIC
        ${${PROJECT_NAME}_SOURCES}
        ${${PROJECT_NAME}_HEADERS}
        ${${PROJECT_NAME}_SHADERS}
        )

set_target_properties(${PROJECT_NAME} PROPERTIES FOLDER "easy3d")

target_include_directories(${PROJECT_NAME} PRIVATE ${EASY3D_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} glew glfw core util fileio)

target_compile_definitions(${PROJECT_NAME} PRIVATE GLEW_STATIC)

if (MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _CRT_SECURE_NO_DEPRECATE)
endif ()
