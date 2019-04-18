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


#include <easy3d/viewer/setting.h>
#include <easy3d/util/file.h>


namespace easy3d {

    // the default parameters used for initialization of both ui and rendering
    namespace setting {

        // ui
        vec4	 background_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

        // lighting
        vec4     light_position = vec4(0.27f, 0.27f, 0.92f, 0);
        vec4	 light_specular = vec4(0.4f, 0.4f, 0.4f, 1.0f);
        vec4     light_ambient = vec4(0.05f, 0.05f, 0.05f, 1.0f);
        float	 light_shininess = 64.0f;

        bool	 light_surface_distinct_back_color = false;
        vec4	 light_surface_back_color = vec4(0.67f, 0.67f, 1.0f, 1.0f);
        bool	 light_two_sides = false;

        // effect
        int		 effect_ssao_algorithm = 0; // disabled
        float    effect_ssao_radius = 2.0f;
        float    effect_ssao_intensity = 1.5f;
        float    effect_ssao_bias = 0.1f;
        float    effect_ssao_sharpness = 40.0f;
        bool	 effect_edl_enabled = false;
        bool	 effect_transparency_enabled = false;
        bool     effect_shadow_enabled = false;
        float    shadow_light_distance = 50.0f;
        float    shadow_softness = 0.5f;
        float    shadow_darkness = 0.6f;

        // point cloud
        bool    point_cloud_use_color_property = true;
        bool    point_cloud_show_points = true;
        vec3    point_cloud_default_color = vec3(85 / 255.0f, 170 / 255.0f, 1.0f);
        float   point_cloud_point_size = 2.0f;
        bool    point_cloud_impostors = false;

        // surface mesh - surface
        bool    surface_mesh_phong_shading = false;
        bool    surface_mesh_show_faces = true;
        vec3    surface_mesh_faces_default_color = vec3(0.8f, 0.88f, 1.0f);
        bool    surface_mesh_use_color_property = true;
        float   surface_mesh_opacity = 0.6f;

        // surface mesh - vertices
        bool    surface_mesh_show_vertices = false;
        vec3    surface_mesh_vertices_default_color = vec3(0.0f, 1.0f, 0.0f);
        bool    surface_mesh_vertices_imposters = true;
        float   surface_mesh_vertices_point_size = 5.0f;

        // surface mesh - wireframe
        bool    surface_mesh_show_wireframe = false;
        vec3    surface_mesh_wireframe_default_color = vec3(0.0f, 0.0f, 0.0f);
        bool    surface_mesh_wireframe_imposters = false;
        float   surface_mesh_wireframe_imposter_thickness = 1.0f;

        // surface mesh - borders
        bool    surface_mesh_show_borders = false;
        vec3    surface_mesh_borders_default_color = vec3(1.0f, 0.0f, 0.0f);
        bool    surface_mesh_borders_imposters = true;
        float   surface_mesh_borders_imposter_thickness = 3.0f;


		// resource directory (containing color maps, shaders, textures, etc.)
		std::string resource_directory() {
			std::string dir = "resource";
			if (file::is_directory(dir)) 
				return "./" + dir;
			else if (file::is_directory("../resource"))
				return "../resource";
			else if (file::is_directory("../../resource"))
				return "../../resource";
			else if (file::is_directory("../../../resource"))
				return "../../../resource";
			return dir;
		}

    } // namespace setting {

} // namespace easy3d

