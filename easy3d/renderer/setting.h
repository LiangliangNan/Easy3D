/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
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
 ********************************************************************/

#ifndef EASY3D_RENDERER_SETTING_H
#define EASY3D_RENDERER_SETTING_H


#include <easy3d/core/types.h>


namespace easy3d {

    class ClippingPlane;

    /**
     * \brief Default parameters used for initializing rendering and UI
     * \namespace easy3d::setting
     */
    namespace setting {

        // ui
        extern vec4 background_color;

        // selection
        // color for selected primitives
        extern vec4 selected_color;

        // lighting
        extern vec4 light_position;  // light position defined in camera coordinate system
        // drawable lighting
        extern bool triangles_drawable_two_side_lighting;
        extern bool points_drawable_two_side_lighting;
        extern bool lines_drawable_two_side_lighting;
        // drawable distinct backside color
        extern bool triangles_drawable_distinct_backside_color;
        extern bool points_drawable_distinct_backside_color;
        extern bool lines_drawable_distinct_backside_color;
        // drawable backside color
        extern vec4 triangles_drawable_backside_color;
        extern vec4 points_drawable_backside_color;
        extern vec4 lines_drawable_backside_color;

        // material
        extern vec4 material_ambient;
        extern vec4 material_specular;
        extern float material_shininess;    // specular power

        // effect
        extern int effect_ssao_algorithm;
        extern float effect_ssao_radius;
        extern float effect_ssao_intensity;
        extern float effect_ssao_bias;
        extern float effect_ssao_sharpness;
        extern bool effect_edl_enabled;
        extern bool effect_transparency_enabled;
        extern bool effect_shadow_enabled;
        extern float shadow_light_distance;
        extern float shadow_softness;
        extern float shadow_darkness;

        // point cloud
        extern vec4 point_cloud_points_color;
        extern float point_cloud_point_size;
        extern bool point_cloud_impostors;
        extern bool point_cloud_two_side_lighting;
        extern vec4 point_cloud_back_side_color;

        // surface mesh - surface
        extern bool surface_mesh_phong_shading;
        extern bool surface_mesh_show_faces;
        extern vec4 surface_mesh_faces_color;
        extern bool surface_mesh_use_color_property;
        extern float surface_mesh_opacity;
        // surface mesh - vertices
        extern bool surface_mesh_show_vertices;
        extern vec4 surface_mesh_vertices_color;
        extern bool surface_mesh_vertices_imposters;
        extern float surface_mesh_vertices_point_size;
        // surface mesh - edges
        extern bool surface_mesh_show_edges;
        extern vec4 surface_mesh_edges_color;
        extern bool surface_mesh_edges_imposters;
        extern float surface_mesh_edges_line_width;
        // surface mesh - borders
        extern bool surface_mesh_show_borders;
        extern vec4 surface_mesh_borders_color;
        extern bool surface_mesh_borders_imposters;
        extern float surface_mesh_borders_line_width;

        // graph - vertices
        extern bool graph_show_vertices;
        extern vec4 graph_vertices_color;
        extern float graph_vertices_point_size;

        // surface mesh - edges
        extern bool graph_show_edges;
        extern vec4 graph_edges_color;
        extern float graph_edges_line_width;

        extern ClippingPlane* clipping_plane;
        extern vec4 clipping_plane_color;


    } // namespace setting

} // namespace easy3d


#endif // EASY3D_RENDERER_SETTING_H
