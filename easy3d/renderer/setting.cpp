/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include <easy3d/renderer/setting.h>


namespace easy3d {

    // the default parameters used for initialization of both ui and rendering
    namespace setting {

        // ui
        vec4 background_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

        // lighting
        vec4 light_position = vec4(0.27f, 0.27f, 0.92f, 0.0f);

        // drawable lighting
        bool triangles_drawable_two_side_lighting = true;
        bool points_drawable_two_side_lighting = true;
        bool lines_drawable_two_side_lighting = false;
        // drawable distinct backside color
        bool triangles_drawable_distinct_backside_color = true;
        bool points_drawable_distinct_backside_color = false;
        bool lines_drawable_distinct_backside_color = false;
        // drawable backside color
        vec4 triangles_drawable_backside_color = vec4(0.8f, 0.4f, 0.4f, 1.0f);
        vec4 points_drawable_backside_color = vec4(0.8f, 0.4f, 0.4f, 1.0f);
        vec4 lines_drawable_backside_color = vec4(0.8f, 0.4f, 0.4f, 1.0f);

        // material
        vec4 material_ambient = vec4(0.05f, 0.05f, 0.05f, 1.0f);
        vec4 material_specular = vec4(0.4f, 0.4f, 0.4f, 1.0f);
        float material_shininess = 64.0f;

        // effect
        int effect_ssao_algorithm = 0; // disabled
        float effect_ssao_radius = 2.0f;
        float effect_ssao_intensity = 1.5f;
        float effect_ssao_bias = 0.1f;
        float effect_ssao_sharpness = 40.0f;
        bool effect_edl_enabled = false;
        bool effect_transparency_enabled = false;
        bool effect_shadow_enabled = false;
        float shadow_light_distance = 50.0f;
        float shadow_softness = 0.5f;
        float shadow_darkness = 0.6f;

        // point cloud
        bool point_cloud_use_color_property = true;
        bool point_cloud_show_points = true;
        vec4 point_cloud_points_color = vec4(0.33f, 0.67f, 1.0f, 1.0f);
        float point_cloud_point_size = 2.0f;
        bool point_cloud_impostors = false;

        // surface mesh - surface
        bool surface_mesh_phong_shading = false;
        bool surface_mesh_show_faces = true;
        vec4 surface_mesh_faces_color = vec4(1.0f, 0.8f, 0.4f, 1.0f);
        bool surface_mesh_use_color_property = true;
        float surface_mesh_opacity = 0.6f;

        // surface mesh - vertices
        bool surface_mesh_show_vertices = false;
        vec4 surface_mesh_vertices_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
        bool surface_mesh_vertices_imposters = true;
        float surface_mesh_vertices_point_size = 5.0f;

        // surface mesh - edges
        bool surface_mesh_show_edges = false;
        vec4 surface_mesh_edges_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        bool surface_mesh_edges_imposters = false;
        float surface_mesh_edges_line_width = 1.0f;

        // surface mesh - borders
        bool surface_mesh_show_borders = false;
        vec4 surface_mesh_borders_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
        bool surface_mesh_borders_imposters = true;
        float surface_mesh_borders_line_width = 2.0f;

        // graph - vertices
        bool graph_show_vertices = true;
        vec4 graph_vertices_color = vec4(0.8f, 0.0f, 0.5f, 1.0f);
        float   graph_vertices_point_size = 15;

        // surface mesh - edges
        bool graph_show_edges = true;
        vec4 graph_edges_color = vec4(1.0f, 0.67f, 0.5f, 1.0f);
        float   graph_edges_line_width = 3.0f;

        // clipping plane
        ClippingPlane* clipping_plane = nullptr;
        vec4 clipping_plane_color = vec4(1.0f, 0.0f, 0.0f, 0.2f);

    } // namespace setting

} // namespace easy3d

