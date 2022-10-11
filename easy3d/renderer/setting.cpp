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

#include <easy3d/renderer/setting.h>
#include <easy3d/util/file_system.h>

#include <fstream>


namespace easy3d {

    // the default parameters used for initialization of both ui and rendering
    namespace setting {

        std::string setting_file_name = "";

        void initialize(const std::string& file_name) {
            setting_file_name = file_name;
            if (file_system::is_file(setting_file_name))
                if (load(setting_file_name))
                    return;
            save(setting_file_name);
        }

        void save() {
            if (!setting_file_name.empty())
                save(setting_file_name);
        }

        // ui: background color of the viewer
        vec4 background_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

        // highlight: color for highlighted/selected primitives
        vec4 highlight_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);

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
        bool point_cloud_show_vertices = true;
        vec4 point_cloud_points_color = vec4(0.33f, 0.67f, 1.0f, 1.0f);
        float point_cloud_points_size = 3.0f;
        bool point_cloud_impostors = false;

        // surface mesh - surface
        bool surface_mesh_phong_shading = false;
        bool surface_mesh_show_faces = true;
        vec4 surface_mesh_faces_color = vec4(1.0f, 0.8f, 0.4f, 1.0f);
        float surface_mesh_opacity = 0.6f;

        // surface mesh - vertices
        bool surface_mesh_show_vertices = false;
        vec4 surface_mesh_vertices_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
        bool surface_mesh_vertices_imposters = true;
        float surface_mesh_vertices_point_size = 5.0f;
		// graph - vertices
		bool graph_show_vertices = true;
		vec4 graph_vertices_color = vec4(0.8f, 0.0f, 0.5f, 1.0f);
		bool graph_vertices_imposters = true;
		float graph_vertices_point_size = 15;
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

        // surface mesh - edges
        bool graph_show_edges = true;
        vec4 graph_edges_color = vec4(1.0f, 0.67f, 0.5f, 1.0f);
        bool graph_edges_imposters = true;
        float graph_edges_line_width = 3.0f;

		// polyhedral mesh - surface
		bool poly_mesh_show_faces = true;
		vec4 poly_mesh_faces_color = vec4(1.0f, 0.8f, 0.4f, 1.0f);
		// polyhedral mesh - vertices
		bool poly_mesh_show_vertices = false;
		vec4 poly_mesh_vertices_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
		bool poly_mesh_vertices_imposters = true;
		float poly_mesh_vertices_point_size = 5.0f;
		// polyhedral mesh - edges
		bool poly_mesh_show_edges = false;
		vec4 poly_mesh_edges_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
		bool poly_mesh_edges_imposters = false;
		float poly_mesh_edges_line_width = 1.0f;

        // clipping plane
        ClippingPlane* clipping_plane = nullptr;
        vec4 clipping_plane_color = vec4(1.0f, 0.0f, 0.0f, 0.2f);


        // Save the rendering parameters to a file.
        bool save(const std::string& file_name) {
            std::ofstream output(file_name);
            if (!output.is_open()) {
                LOG(WARNING) << "could not open file: " << file_name;
                return false;
            }

#define SAVE(var)                                \
    output << #var << ": " << var << std::endl;  \
    if (output.fail())                           \
        return false;

			// background color of the viewer
			SAVE(background_color)
			// highlight: color for highlighted/selected primitives
			SAVE(highlight_color)

			// lighting
			SAVE(light_position)  // light position defined in camera coordinate system
			// material
			SAVE(material_ambient)
			SAVE(material_specular)
			SAVE(material_shininess)    // specular power
			output << std::endl;

			// drawable lighting
			SAVE(triangles_drawable_two_side_lighting)
			SAVE(points_drawable_two_side_lighting)
			SAVE(lines_drawable_two_side_lighting)
			// enable/disable drawable distinct backside color
			SAVE(triangles_drawable_distinct_backside_color)
			SAVE(points_drawable_distinct_backside_color)
			SAVE(lines_drawable_distinct_backside_color)
			// drawable backside color
			SAVE(triangles_drawable_backside_color)
			SAVE(points_drawable_backside_color)
			SAVE(lines_drawable_backside_color)
			output << std::endl;

			// effect
			SAVE(effect_ssao_radius)
			SAVE(effect_ssao_intensity)
			SAVE(effect_ssao_bias)
			SAVE(effect_ssao_sharpness)
			SAVE(effect_edl_enabled)
			SAVE(effect_transparency_enabled)
			SAVE(effect_shadow_enabled)
			SAVE(shadow_light_distance)
			SAVE(shadow_softness)
			SAVE(shadow_darkness)
			output << std::endl;

			// point cloud
			SAVE(point_cloud_show_vertices)
			SAVE(point_cloud_points_color)
			SAVE(point_cloud_impostors)
			SAVE(point_cloud_points_size)
			output << std::endl;

			// surface mesh - surface
			SAVE(surface_mesh_phong_shading)
			SAVE(surface_mesh_show_faces)
			SAVE(surface_mesh_faces_color)
			SAVE(surface_mesh_opacity)
			// surface mesh - vertices
			SAVE(surface_mesh_show_vertices)
			SAVE(surface_mesh_vertices_color)
			SAVE(surface_mesh_vertices_imposters)
			SAVE(surface_mesh_vertices_point_size)
			// surface mesh - edges
			SAVE(surface_mesh_show_edges)
			SAVE(surface_mesh_edges_color)
			SAVE(surface_mesh_edges_imposters)
			SAVE(surface_mesh_edges_line_width)
			// surface mesh - borders
			SAVE(surface_mesh_show_borders)
			SAVE(surface_mesh_borders_color)
			SAVE(surface_mesh_borders_imposters)
			SAVE(surface_mesh_borders_line_width)
			output << std::endl;

			// graph: vertices
			SAVE(graph_show_vertices)
			SAVE(graph_vertices_color)
			SAVE(graph_vertices_imposters)
			SAVE(graph_vertices_point_size)
			// graph: edges
			SAVE(graph_show_edges)
			SAVE(graph_edges_color)
			SAVE(graph_edges_imposters)
			SAVE(graph_edges_line_width)				
			output << std::endl;

			// polyhedral mesh - surface
			SAVE(poly_mesh_show_faces)
			SAVE(poly_mesh_faces_color)
			// polyhedral mesh - vertices
			SAVE(poly_mesh_show_vertices)
			SAVE(poly_mesh_vertices_color)
			SAVE(poly_mesh_vertices_imposters)
			SAVE(poly_mesh_vertices_point_size)
			// polyhedral mesh - edges
			SAVE(poly_mesh_show_edges)
			SAVE(poly_mesh_edges_color)
			SAVE(poly_mesh_edges_imposters)
			SAVE(poly_mesh_edges_line_width)
			output << std::endl;

			// clipping plane
			SAVE(clipping_plane_color)

            return true;
        }

        // Load the rendering parameters from a file.
        bool load(const std::string& file_name) {
            std::ifstream input(file_name);
            if (!input.is_open()) {
                LOG(WARNING) << "could not open file: " << file_name;
                return false;
            }

#define LOAD(var)                                                                           \
    input >> dummy >> var;                                                                  \
    if (input.fail()) {                                                                     \
         LOG(WARNING) << "failed to read value for variable: " << #var;                     \
         return false;                                                                      \
    }                                                                                       \
    if (dummy.find(#var) == std::string::npos) {                                            \
         LOG(WARNING) << "key does not match variable name: " << dummy << " vs. " << #var;  \
         return false;                                                                      \
    }

            std::string dummy;


			// background color of the viewer
			LOAD(background_color)
			// highlight: color for highlighted/selected primitives
			LOAD(highlight_color)

			// lighting
			LOAD(light_position)  // light position defined in camera coordinate system
			// material
			LOAD(material_ambient)
			LOAD(material_specular)
			LOAD(material_shininess)    // specular power

			// drawable lighting
			LOAD(triangles_drawable_two_side_lighting)
			LOAD(points_drawable_two_side_lighting)
			LOAD(lines_drawable_two_side_lighting)
			// enable/disable drawable distinct backside color
			LOAD(triangles_drawable_distinct_backside_color)
			LOAD(points_drawable_distinct_backside_color)
			LOAD(lines_drawable_distinct_backside_color)
			// drawable backside color
			LOAD(triangles_drawable_backside_color)
			LOAD(points_drawable_backside_color)
			LOAD(lines_drawable_backside_color)

			// effect
			LOAD(effect_ssao_radius)
			LOAD(effect_ssao_intensity)
			LOAD(effect_ssao_bias)
			LOAD(effect_ssao_sharpness)
			LOAD(effect_edl_enabled)
			LOAD(effect_transparency_enabled)
			LOAD(effect_shadow_enabled)
			LOAD(shadow_light_distance)
			LOAD(shadow_softness)
			LOAD(shadow_darkness)

			// point cloud
			LOAD(point_cloud_show_vertices)
			LOAD(point_cloud_points_color)
			LOAD(point_cloud_impostors)
			LOAD(point_cloud_points_size)

			// surface mesh - surface
			LOAD(surface_mesh_phong_shading)
			LOAD(surface_mesh_show_faces)
			LOAD(surface_mesh_faces_color)
			LOAD(surface_mesh_opacity)
			// surface mesh - vertices
			LOAD(surface_mesh_show_vertices)
			LOAD(surface_mesh_vertices_color)
			LOAD(surface_mesh_vertices_imposters)
			LOAD(surface_mesh_vertices_point_size)
			// surface mesh - edges
			LOAD(surface_mesh_show_edges)
			LOAD(surface_mesh_edges_color)
			LOAD(surface_mesh_edges_imposters)
			LOAD(surface_mesh_edges_line_width)
			// surface mesh - borders
			LOAD(surface_mesh_show_borders)
			LOAD(surface_mesh_borders_color)
			LOAD(surface_mesh_borders_imposters)
			LOAD(surface_mesh_borders_line_width)

			// graph: vertices
			LOAD(graph_show_vertices)
			LOAD(graph_vertices_color)
			LOAD(graph_vertices_imposters)
			LOAD(graph_vertices_point_size)
			// graph: edges
			LOAD(graph_show_edges)
			LOAD(graph_edges_color)
			LOAD(graph_edges_imposters)
			LOAD(graph_edges_line_width)

			// polyhedral mesh - surface
			LOAD(poly_mesh_show_faces)
			LOAD(poly_mesh_faces_color)
			// polyhedral mesh - vertices
			LOAD(poly_mesh_show_vertices)
			LOAD(poly_mesh_vertices_color)
			LOAD(poly_mesh_vertices_imposters)
			LOAD(poly_mesh_vertices_point_size)
			// polyhedral mesh - edges
			LOAD(poly_mesh_show_edges)
			LOAD(poly_mesh_edges_color)
			LOAD(poly_mesh_edges_imposters)
			LOAD(poly_mesh_edges_line_width)

			// clipping plane
			LOAD(clipping_plane_color)

            return true;
        }

    } // namespace setting

} // namespace easy3d

