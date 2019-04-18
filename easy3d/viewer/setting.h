/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work, 
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan. 
*           Easy3D: a lightweight, easy-to-use, and efficient C++ 
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


#ifndef EASY3D_OPENGL_DSETTING_H
#define EASY3D_OPENGL_DSETTING_H


#include <easy3d/core/types.h>


namespace easy3d {

    // the default parameters used for initialization of both ui and rendering
    namespace setting {

        // ui
        extern vec4		background_color;

        // lighting
        extern vec4     light_position;  // light poistion defined in camera coordinate system
        extern vec4		light_specular;
        extern vec4		light_ambient;
        extern float	light_shininess;	// specular power

        extern bool		light_surface_distinct_back_color;
        extern vec4		light_surface_back_color;
        extern bool		light_two_sides;

        // effect
        extern int		effect_ssao_algorithm;
        extern float    effect_ssao_radius;
        extern float    effect_ssao_intensity;
        extern float    effect_ssao_bias;
        extern float    effect_ssao_sharpness;
        extern bool		effect_edl_enabled;
        extern bool     effect_transparency_enabled;
        extern bool     effect_shadow_enabled;
        extern float    shadow_light_distance;
        extern float    shadow_softness;
        extern float    shadow_darkness;

        // point cloud
        extern bool    point_cloud_use_color_property;
        extern bool    point_cloud_show_points;
        extern vec3    point_cloud_default_color;
        extern float   point_cloud_point_size;
        extern bool    point_cloud_impostors;

        // surface mesh - surface
        extern bool    surface_mesh_phong_shading;
        extern bool    surface_mesh_show_faces;
        extern vec3    surface_mesh_faces_default_color;
        extern bool    surface_mesh_use_color_property;
        extern float   surface_mesh_opacity;

        // surface mesh - vertices
        extern bool    surface_mesh_show_vertices;
        extern vec3    surface_mesh_vertices_default_color;
        extern bool    surface_mesh_vertices_imposters;
        extern float   surface_mesh_vertices_point_size;

        // surface mesh - wireframe
        extern bool    surface_mesh_show_wireframe;
        extern vec3    surface_mesh_wireframe_default_color;
        extern bool    surface_mesh_wireframe_imposters;
        extern float   surface_mesh_wireframe_imposter_thickness;

        // surface mesh - borders
        extern bool    surface_mesh_show_borders;
        extern vec3    surface_mesh_borders_default_color;
        extern bool    surface_mesh_borders_imposters;
        extern float   surface_mesh_borders_imposter_thickness;

		// resource directory (containing color maps, shaders, textures, etc.)
		extern std::string resource_directory();

    } // namespace setting {

} // namespace easy3d


#endif // EASY3D_OPENGL_DSETTING_H
