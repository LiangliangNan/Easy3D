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

#include <fstream>

#include <easy3d/util/file_system.h>
#include <3rd_party/json/json.hpp>


namespace easy3d {

    // the default parameters used for initialization of both ui and rendering
    namespace setting {

        // ui: background color of the viewer
        vec4 background_color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
        // highlight: color for highlighted/selected primitives
        vec4 highlight_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
        // lighting
        vec4 light_position = vec4(0.27f, 0.27f, 0.92f, 0.0f);

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

        // points drawable
        bool points_drawable_two_side_lighting = true;
        bool points_drawable_distinct_backside_color = false;
        vec4 points_drawable_backside_color = vec4(0.8f, 0.4f, 0.4f, 1.0f);
        // lines drawable
        bool lines_drawable_two_side_lighting = false;
        bool lines_drawable_distinct_backside_color = false;
        vec4 lines_drawable_backside_color = vec4(0.8f, 0.4f, 0.4f, 1.0f);
        // triangles drawable
        bool triangles_drawable_two_side_lighting = true;
        bool triangles_drawable_distinct_backside_color = true;
        vec4 triangles_drawable_backside_color = vec4(0.8f, 0.4f, 0.4f, 1.0f);

        // point cloud
        bool point_cloud_vertices_visible = true;
        vec4 point_cloud_vertices_color = vec4(0.33f, 0.67f, 1.0f, 1.0f);
        bool point_cloud_vertices_impostors = false;
        float point_cloud_vertices_size = 3.0f;

        // surface mesh - surface
        bool surface_mesh_faces_phong_shading = false;
        bool surface_mesh_faces_visible = true;
        vec4 surface_mesh_faces_color = vec4(1.0f, 0.8f, 0.4f, 1.0f);
        float surface_mesh_faces_opacity = 0.6f;
        // surface mesh - vertices
        bool surface_mesh_vertices_visible = false;
        vec4 surface_mesh_vertices_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
        bool surface_mesh_vertices_imposters = true;
        float surface_mesh_vertices_size = 5.0f;
        // surface mesh - edges
        bool surface_mesh_edges_visible = false;
        vec4 surface_mesh_edges_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        bool surface_mesh_edges_imposters = false;
        float surface_mesh_edges_size = 1.0f;
        // surface mesh - borders
        bool surface_mesh_borders_visible = false;
        vec4 surface_mesh_borders_color = vec4(1.0f, 0.0f, 0.0f, 1.0f);
        bool surface_mesh_borders_imposters = true;
        float surface_mesh_borders_size = 2.0f;

        // graph - vertices
        bool graph_vertices_visible = true;
        vec4 graph_vertices_color = vec4(0.8f, 0.0f, 0.5f, 1.0f);
        bool graph_vertices_imposters = true;
        float graph_vertices_size = 15;
        // graph: edges
        bool graph_edges_visible = true;
        vec4 graph_edges_color = vec4(1.0f, 0.67f, 0.5f, 1.0f);
        bool graph_edges_imposters = true;
        float graph_edges_size = 3.0f;

        // polyhedral mesh - surface
        bool poly_mesh_faces_visible = true;
        vec4 poly_mesh_faces_color = vec4(1.0f, 0.8f, 0.4f, 1.0f);
        // polyhedral mesh - vertices
        bool poly_mesh_vertices_visible = false;
        vec4 poly_mesh_vertices_color = vec4(0.0f, 1.0f, 0.0f, 1.0f);
        bool poly_mesh_vertices_imposters = true;
        float poly_mesh_vertices_size = 5.0f;
        // polyhedral mesh - edges
        bool poly_mesh_edges_visible = false;
        vec4 poly_mesh_edges_color = vec4(0.0f, 0.0f, 0.0f, 1.0f);
        bool poly_mesh_edges_imposters = false;
        float poly_mesh_edges_size = 1.0f;

        // clipping plane
        ClippingPlane *clipping_plane = nullptr;
        vec4 clipping_plane_color = vec4(1.0f, 0.0f, 0.0f, 0.2f);


        void initialize(const std::string &setting_file) {
            std::string setting_file_name = setting_file;
            if (setting_file_name == "default") {
                const std::string app_path = file_system::executable();
                std::string file_path = app_path;
#ifdef __APPLE__
                // macOS may put the executable file in an application bundle, e.g., "PolyFit.app/Contents/MacOS/PolyFit"
                std::string::size_type pos = file_path.find(".app");
                if (pos != std::string::npos)
                    file_path = file_path.substr(0, pos);
#endif
                file_path = file_system::parent_directory(file_path);
                setting_file_name = file_path + "/" + file_system::base_name(app_path) + ".ini";
            }

            if (!setting_file_name.empty()) {
                if (file_system::is_file(setting_file_name))
                    if (load(setting_file_name))
                        return;
                save(setting_file_name);
            }
        }

        // Save the rendering parameters to a file.
        bool save(const std::string &file_name) {
            std::ofstream output(file_name);
            if (!output.is_open()) {
                LOG(WARNING) << "could not open file: " << file_name;
                return false;
            }

#define SAVE_VAR(group, var)   settings[group][#var] = var;
#define SAVE_VEC(group, var)   settings[group][#var] = {var[0], var[1], var[2], var[3]};

            using json = nlohmann::json;
            json settings;

            // background color of the viewer
            SAVE_VEC("global", background_color)
            // highlight: color for highlighted/selected primitives
            SAVE_VEC("global", highlight_color)
            // lighting
            SAVE_VEC("global", light_position)  // light position defined in camera coordinate system
            // material
            SAVE_VEC("global", material_ambient)
            SAVE_VEC("global", material_specular)
            SAVE_VAR("global", material_shininess)    // specular power

            // effect
            SAVE_VAR("effect", effect_ssao_radius)
            SAVE_VAR("effect", effect_ssao_intensity)
            SAVE_VAR("effect", effect_ssao_bias)
            SAVE_VAR("effect", effect_ssao_sharpness)
            SAVE_VAR("effect", shadow_light_distance)
            SAVE_VAR("effect", shadow_softness)
            SAVE_VAR("effect", shadow_darkness)

            // points drawable
            SAVE_VAR("points drawable", points_drawable_two_side_lighting)
            SAVE_VAR("points drawable", points_drawable_distinct_backside_color)
            SAVE_VEC("points drawable", points_drawable_backside_color)
            // lines drawable
            SAVE_VAR("lines drawable", lines_drawable_two_side_lighting)
            SAVE_VAR("lines drawable", lines_drawable_distinct_backside_color)
            SAVE_VEC("lines drawable", lines_drawable_backside_color)
            // triangles drawable
            SAVE_VAR("triangles drawable", triangles_drawable_two_side_lighting)
            SAVE_VAR("triangles drawable", triangles_drawable_distinct_backside_color)
            SAVE_VEC("triangles drawable", triangles_drawable_backside_color)

            // point cloud
            SAVE_VAR("point cloud", point_cloud_vertices_visible)
            SAVE_VEC("point cloud", point_cloud_vertices_color)
            SAVE_VAR("point cloud", point_cloud_vertices_impostors)
            SAVE_VAR("point cloud", point_cloud_vertices_size)

            // surface mesh - surface
            SAVE_VAR("surface mesh", surface_mesh_faces_phong_shading)
            SAVE_VAR("surface mesh", surface_mesh_faces_visible)
            SAVE_VEC("surface mesh", surface_mesh_faces_color)
            SAVE_VAR("surface mesh", surface_mesh_faces_opacity)
            // surface mesh - vertices
            SAVE_VAR("surface mesh", surface_mesh_vertices_visible)
            SAVE_VEC("surface mesh", surface_mesh_vertices_color)
            SAVE_VAR("surface mesh", surface_mesh_vertices_imposters)
            SAVE_VAR("surface mesh", surface_mesh_vertices_size)
            // surface mesh - edges
            SAVE_VAR("surface mesh", surface_mesh_edges_visible)
            SAVE_VEC("surface mesh", surface_mesh_edges_color)
            SAVE_VAR("surface mesh", surface_mesh_edges_imposters)
            SAVE_VAR("surface mesh", surface_mesh_edges_size)
            // surface mesh - borders
            SAVE_VAR("surface mesh", surface_mesh_borders_visible)
            SAVE_VEC("surface mesh", surface_mesh_borders_color)
            SAVE_VAR("surface mesh", surface_mesh_borders_imposters)
            SAVE_VAR("surface mesh", surface_mesh_borders_size)

            // graph: vertices
            SAVE_VAR("graph", graph_vertices_visible)
            SAVE_VEC("graph", graph_vertices_color)
            SAVE_VAR("graph", graph_vertices_imposters)
            SAVE_VAR("graph", graph_vertices_size)
            // graph: edges
            SAVE_VAR("graph", graph_edges_visible)
            SAVE_VEC("graph", graph_edges_color)
            SAVE_VAR("graph", graph_edges_imposters)
            SAVE_VAR("graph", graph_edges_size)

            // polyhedral mesh - surface
            SAVE_VAR("polyhedral mesh", poly_mesh_faces_visible)
            SAVE_VEC("polyhedral mesh", poly_mesh_faces_color)
            // polyhedral mesh - vertices
            SAVE_VAR("polyhedral mesh", poly_mesh_vertices_visible)
            SAVE_VEC("polyhedral mesh", poly_mesh_vertices_color)
            SAVE_VAR("polyhedral mesh", poly_mesh_vertices_imposters)
            SAVE_VAR("polyhedral mesh", poly_mesh_vertices_size)
            // polyhedral mesh - edges
            SAVE_VAR("polyhedral mesh", poly_mesh_edges_visible)
            SAVE_VEC("polyhedral mesh", poly_mesh_edges_color)
            SAVE_VAR("polyhedral mesh", poly_mesh_edges_imposters)
            SAVE_VAR("polyhedral mesh", poly_mesh_edges_size)

            // clipping plane
            SAVE_VEC("clipping plane", clipping_plane_color)

            output << std::setw(4) << settings;

            return true;
        }

        // Load the rendering parameters from a file.
        bool load(const std::string &file_name) {
            std::ifstream input(file_name);
            if (!input.is_open()) {
                LOG(WARNING) << "could not open file: " << file_name;
                return false;
            }

            // read a JSON file
            using json = nlohmann::json;
            json settings;
            input >> settings;

#define LOAD_VAR(key, var)    {                                                     \
            auto pos = settings.find(key);                                          \
            if (pos != settings.end()) {                                            \
                auto val_pos = pos->find(#var);                                     \
                if (val_pos != pos->end()) {                                        \
                    if (val_pos->is_array())                                        \
                        LOG(WARNING) << "variable type mismatch: vector vs value";  \
                    else                                                            \
                        var = *val_pos;                                             \
                }                                                                   \
            }                                                                       \
            else                                                                    \
                LOG(WARNING) << "value does not exist for key: " << #var;           \
        }

#define LOAD_VEC(key, var)    {                                                     \
            auto pos = settings.find(key);                                          \
            if (pos != settings.end()) {                                            \
                auto val_pos = pos->find(#var);                                     \
                if (val_pos != pos->end()) {                                        \
                    if (val_pos->is_array()) {                                      \
                        for (int i = 0; i < val_pos->size(); ++i)                   \
                            var[i] = val_pos->at(i);                                \
                    }                                                               \
                    else                                                            \
                        LOG(ERROR) << "variable type mismatch: vector vs value";    \
                }                                                                   \
                else                                                                \
                    LOG(ERROR) << "value does not exist for key: " << #var;         \
            }}

            // background color of the viewer
            LOAD_VEC("global", background_color)
            // highlight: color for highlighted/selected primitives
            LOAD_VEC("global", highlight_color)
            // lighting
            LOAD_VEC("global", light_position)  // light position defined in camera coordinate system
            // material
            LOAD_VEC("global", material_ambient)
            LOAD_VEC("global", material_specular)
            LOAD_VAR("global", material_shininess)    // specular power

            // effect
            LOAD_VAR("effect", effect_ssao_radius)
            LOAD_VAR("effect", effect_ssao_intensity)
            LOAD_VAR("effect", effect_ssao_bias)
            LOAD_VAR("effect", effect_ssao_sharpness)
            LOAD_VAR("effect", shadow_light_distance)
            LOAD_VAR("effect", shadow_softness)
            LOAD_VAR("effect", shadow_darkness)

            // points drawable
            LOAD_VAR("points drawable", points_drawable_two_side_lighting)
            LOAD_VAR("points drawable", points_drawable_distinct_backside_color)
            LOAD_VEC("points drawable", points_drawable_backside_color)
            // lines drawable
            LOAD_VAR("lines drawable", lines_drawable_two_side_lighting)
            LOAD_VAR("lines drawable", lines_drawable_distinct_backside_color)
            LOAD_VEC("lines drawable", lines_drawable_backside_color)
            // triangles drawable
            LOAD_VAR("triangles drawable", triangles_drawable_two_side_lighting)
            LOAD_VAR("triangles drawable", triangles_drawable_distinct_backside_color)
            LOAD_VEC("triangles drawable", triangles_drawable_backside_color)

            // point cloud
            LOAD_VAR("point cloud", point_cloud_vertices_visible)
            LOAD_VEC("point cloud", point_cloud_vertices_color)
            LOAD_VAR("point cloud", point_cloud_vertices_impostors)
            LOAD_VAR("point cloud", point_cloud_vertices_size)

            // surface mesh - surface
            LOAD_VAR("surface mesh", surface_mesh_faces_phong_shading)
            LOAD_VAR("surface mesh", surface_mesh_faces_visible)
            LOAD_VEC("surface mesh", surface_mesh_faces_color)
            LOAD_VAR("surface mesh", surface_mesh_faces_opacity)
            // surface mesh - vertices
            LOAD_VAR("surface mesh", surface_mesh_vertices_visible)
            LOAD_VEC("surface mesh", surface_mesh_vertices_color)
            LOAD_VAR("surface mesh", surface_mesh_vertices_imposters)
            LOAD_VAR("surface mesh", surface_mesh_vertices_size)
            // surface mesh - edges
            LOAD_VAR("surface mesh", surface_mesh_edges_visible)
            LOAD_VEC("surface mesh", surface_mesh_edges_color)
            LOAD_VAR("surface mesh", surface_mesh_edges_imposters)
            LOAD_VAR("surface mesh", surface_mesh_edges_size)
            // surface mesh - borders
            LOAD_VAR("surface mesh", surface_mesh_borders_visible)
            LOAD_VEC("surface mesh", surface_mesh_borders_color)
            LOAD_VAR("surface mesh", surface_mesh_borders_imposters)
            LOAD_VAR("surface mesh", surface_mesh_borders_size)

            // graph: vertices
            LOAD_VAR("graph", graph_vertices_visible)
            LOAD_VEC("graph", graph_vertices_color)
            LOAD_VAR("graph", graph_vertices_imposters)
            LOAD_VAR("graph", graph_vertices_size)
            // graph: edges
            LOAD_VAR("graph", graph_edges_visible)
            LOAD_VEC("graph", graph_edges_color)
            LOAD_VAR("graph", graph_edges_imposters)
            LOAD_VAR("graph", graph_edges_size)

            // polyhedral mesh - surface
            LOAD_VAR("polyhedral mesh", poly_mesh_faces_visible)
            LOAD_VEC("polyhedral mesh", poly_mesh_faces_color)
            // polyhedral mesh - vertices
            LOAD_VAR("polyhedral mesh", poly_mesh_vertices_visible)
            LOAD_VEC("polyhedral mesh", poly_mesh_vertices_color)
            LOAD_VAR("polyhedral mesh", poly_mesh_vertices_imposters)
            LOAD_VAR("polyhedral mesh", poly_mesh_vertices_size)
            // polyhedral mesh - edges
            LOAD_VAR("polyhedral mesh", poly_mesh_edges_visible)
            LOAD_VEC("polyhedral mesh", poly_mesh_edges_color)
            LOAD_VAR("polyhedral mesh", poly_mesh_edges_imposters)
            LOAD_VAR("polyhedral mesh", poly_mesh_edges_size)

            // clipping plane
            LOAD_VEC("clipping plane", clipping_plane_color)

            return true;
        }

    } // namespace setting

} // namespace easy3d

