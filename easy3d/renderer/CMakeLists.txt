set(module renderer)
set(private_dependencies)
set(public_dependencies easy3d::core easy3d::util easy3d::fileio easy3d::algo)

set(${module}_headers
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
        manipulator.h
        manipulated_camera_frame.h
        manipulated_frame.h
        opengl.h
        opengl_error.h
        opengl_util.h
        opengl_timer.h
        shape.h
        read_pixel.h
        buffer.h
        renderer.h
        shader_manager.h
        shader_program.h
        shadow.h
        soft_shadow.h
        state.h
        texture.h
        texture_manager.h
        text_renderer.h
        transform.h
        transform_decompose.h
        transparency.h
        vertex_array_object.h
        )

set(${module}_sources
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
        manipulator.cpp
        manipulated_camera_frame.cpp
        manipulated_frame.cpp
        opengl.cpp
        opengl_error.cpp
        opengl_util.cpp
        opengl_timer.cpp
        shape.cpp
        read_pixel.cpp
        buffer.cpp
        renderer.cpp
        shader_manager.cpp
        shader_program.cpp
        shadow.cpp
        soft_shadow.cpp
        state.cpp
        texture.cpp
        texture_manager.cpp
        text_renderer.cpp
        transform.cpp
        transform_decompose.cpp
        vertex_array_object.cpp
        )


set(${module}_resources
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
        ../../resources/shaders/surface/surface.vert
        ../../resources/shaders/surface/surface.frag
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
        ../../resources/shaders/transparency/dual_depth_peeling_peel_rendering.vert
        ../../resources/shaders/transparency/dual_depth_peeling_peel_rendering.frag
        #selection
        ../../resources/shaders/selection/selection_single_primitive.vert
        ../../resources/shaders/selection/selection_single_primitive.frag
        ../../resources/shaders/selection/selection_pointcloud_rect.vert
        ../../resources/shaders/selection/selection_pointcloud_rect.frag
        ../../resources/shaders/selection/selection_pointcloud_lasso.vert
        ../../resources/shaders/selection/selection_pointcloud_lasso.frag
        )

add_module(${module} "${${module}_headers}" "${${module}_sources}" "${private_dependencies}" "${public_dependencies}")
target_include_directories(easy3d_${module} PRIVATE ${Easy3D_THIRD_PARTY}/glew/include)
target_compile_definitions(easy3d_${module} PRIVATE GLEW_BUILD PUBLIC GLEW_NO_GLU)


if(WIN32 OR MSVC OR APPLE)
    find_package(OpenGL COMPONENTS OpenGL REQUIRED)
    message(STATUS "OpenGL library: ${OPENGL_gl_LIBRARY}")
    target_link_libraries(easy3d_${module} PUBLIC ${OPENGL_gl_LIBRARY})
else() # == if(UNIX AND NOT APPLE)
    set (OpenGL_GL_PREFERENCE GLVND)
    find_package(OpenGL COMPONENTS OpenGL GLX REQUIRED)
    message(STATUS "OpenGL libraries: ${OPENGL_opengl_LIBRARY} ${OPENGL_glx_LIBRARY}")
    target_link_libraries(easy3d_${module} PUBLIC ${OPENGL_opengl_LIBRARY} ${OPENGL_glx_LIBRARY})
endif()


install_module(${module})
# Install glew.h to the installation directory
install(FILES "${Easy3D_THIRD_PARTY}/glew/include/GL/glew.h"
        DESTINATION "${Easy3D_INSTALL_INCLUDE_DIR}/3rd_party/glew/include/GL")