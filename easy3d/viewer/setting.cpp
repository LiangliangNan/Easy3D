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

#include <easy3d/viewer/setting.h>
#include <easy3d/util/file_system.h>


namespace easy3d {

    // the default parameters used for initialization of both ui and rendering
    namespace setting {

        // ui
        vec4 background_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);

        // lighting
        vec4 light_position = vec4(0.27f, 0.27f, 0.92f, 0.0f);
        bool light_distinct_back_color = false;
        vec4 light_back_color = vec4(0.8f, 0.4f, 0.4f, 1.0f);
        bool light_two_sides = false;

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
        vec4 point_cloud_points_color = vec4(85 / 255.0f, 170 / 255.0f, 1.0f, 1.0f);
        float point_cloud_point_size = 2.0f;
        bool point_cloud_impostors = false;

        // surface mesh - surface
        bool surface_mesh_phong_shading = false;
        bool surface_mesh_show_faces = true;
        vec4 surface_mesh_faces_color = vec4(0.8f, 0.88f, 1.0f, 1.0f);
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


        // resource directory (containing color maps, shaders, textures, etc.)
        std::string resource_directory() {
            std::string parent = file_system::executable_directory();
            std::string dir = parent + "/resources";
            if (file_system::is_directory(dir))
                return dir;
            else {
                // For macOS, if reached here, we may need to move "up" three times, because
                // macOS puts the executable file in an application bundle, e.g.,
                // "PolyFit.app/⁨Contents/⁨MacOS⁩/PolyFit". Some IDEs may also put the 'ext' in
                // Debug/Release subfolder, so we may to try four times up at most.
                parent = file_system::parent_directory(parent);
                std::string dir = parent + "/resources";
                if (file_system::is_directory(dir))
                    return dir;
                else {
                    for (int i = 0; i < 4; ++i) {
                        parent = file_system::parent_directory(parent);
                        std::string dir = parent + "/resources";
                        if (file_system::is_directory(dir))
                            return dir;
                    }
                }
                // if still could not find it, return the current working directory
                return file_system::current_working_directory();
            }
        }

    } // namespace setting

} // namespace easy3d

