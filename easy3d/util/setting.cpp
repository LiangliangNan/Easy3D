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

#include <easy3d/util/setting.h>


namespace easy3d {

    // the default parameters used for initialization of both ui and rendering
    namespace setting {

        // ui: background color of the viewer
        vec4 background_color = vec4(0.9f, 0.9f, 1.0f, 1.0f);
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
        float effect_shadow_light_distance = 50.0f;
        float effect_shadow_softness = 0.5f;
        float effect_shadow_darkness = 0.6f;

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
        vec4 clipping_plane_color = vec4(1.0f, 0.0f, 0.0f, 0.2f);

    }   // namespace setting
}   // namespace easy3d




#include <fstream>
#include <easy3d/util/file_system.h>
#include <3rd_party/json/json.hpp>


namespace easy3d {
    //\cond

    // The following two functions are for vectors to be handled as json's built-in types.

    template<typename Vec>
    void to_json(nlohmann::json &j, const Vec &v) {
        j = std::vector<typename Vec::FT>((const typename Vec::FT *) (v), v + v.size());
    }

    template<typename Vec>
    void from_json(const nlohmann::json &j, Vec &v) {
        assert(v.size() == j.size());
        for (int i = 0; i < v.size(); ++i) v[i] = j.at(i);
    }
    //\endcond

    namespace setting {

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
                setting_file_name = file_system::convert_to_native_style(setting_file_name);
            }

            if (!setting_file_name.empty()) {
                if (file_system::is_file(setting_file_name))
                    if (load(setting_file_name)) {
                        LOG(INFO) << "setting loaded: " << setting_file_name;
                        return;
                    }
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

#define ENCODE(group, var)   settings[group][#var] = var;

            nlohmann::json settings;

            // background color of the viewer
            ENCODE("global", background_color)
            // highlight: color for highlighted/selected primitives
            ENCODE("global", highlight_color)
            // lighting
            ENCODE("global", light_position)  // light position defined in camera coordinate system
            // material
            ENCODE("global", material_ambient)
            ENCODE("global", material_specular)
            ENCODE("global", material_shininess)    // specular power

            // effect
            ENCODE("effect", effect_ssao_radius)
            ENCODE("effect", effect_ssao_intensity)
            ENCODE("effect", effect_ssao_bias)
            ENCODE("effect", effect_ssao_sharpness)
            ENCODE("effect", effect_shadow_light_distance)
            ENCODE("effect", effect_shadow_softness)
            ENCODE("effect", effect_shadow_darkness)

            // points drawable
            ENCODE("points drawable", points_drawable_two_side_lighting)
            ENCODE("points drawable", points_drawable_distinct_backside_color)
            ENCODE("points drawable", points_drawable_backside_color)
            // lines drawable
            ENCODE("lines drawable", lines_drawable_two_side_lighting)
            ENCODE("lines drawable", lines_drawable_distinct_backside_color)
            ENCODE("lines drawable", lines_drawable_backside_color)
            // triangles drawable
            ENCODE("triangles drawable", triangles_drawable_two_side_lighting)
            ENCODE("triangles drawable", triangles_drawable_distinct_backside_color)
            ENCODE("triangles drawable", triangles_drawable_backside_color)

            // point cloud
            ENCODE("point cloud", point_cloud_vertices_visible)
            ENCODE("point cloud", point_cloud_vertices_color)
            ENCODE("point cloud", point_cloud_vertices_impostors)
            ENCODE("point cloud", point_cloud_vertices_size)

            // surface mesh - surface
            ENCODE("surface mesh", surface_mesh_faces_phong_shading)
            ENCODE("surface mesh", surface_mesh_faces_visible)
            ENCODE("surface mesh", surface_mesh_faces_color)
            ENCODE("surface mesh", surface_mesh_faces_opacity)
            // surface mesh - vertices
            ENCODE("surface mesh", surface_mesh_vertices_visible)
            ENCODE("surface mesh", surface_mesh_vertices_color)
            ENCODE("surface mesh", surface_mesh_vertices_imposters)
            ENCODE("surface mesh", surface_mesh_vertices_size)
            // surface mesh - edges
            ENCODE("surface mesh", surface_mesh_edges_visible)
            ENCODE("surface mesh", surface_mesh_edges_color)
            ENCODE("surface mesh", surface_mesh_edges_imposters)
            ENCODE("surface mesh", surface_mesh_edges_size)
            // surface mesh - borders
            ENCODE("surface mesh", surface_mesh_borders_visible)
            ENCODE("surface mesh", surface_mesh_borders_color)
            ENCODE("surface mesh", surface_mesh_borders_imposters)
            ENCODE("surface mesh", surface_mesh_borders_size)

            // graph: vertices
            ENCODE("graph", graph_vertices_visible)
            ENCODE("graph", graph_vertices_color)
            ENCODE("graph", graph_vertices_imposters)
            ENCODE("graph", graph_vertices_size)
            // graph: edges
            ENCODE("graph", graph_edges_visible)
            ENCODE("graph", graph_edges_color)
            ENCODE("graph", graph_edges_imposters)
            ENCODE("graph", graph_edges_size)

            // polyhedral mesh - surface
            ENCODE("polyhedral mesh", poly_mesh_faces_visible)
            ENCODE("polyhedral mesh", poly_mesh_faces_color)
            // polyhedral mesh - vertices
            ENCODE("polyhedral mesh", poly_mesh_vertices_visible)
            ENCODE("polyhedral mesh", poly_mesh_vertices_color)
            ENCODE("polyhedral mesh", poly_mesh_vertices_imposters)
            ENCODE("polyhedral mesh", poly_mesh_vertices_size)
            // polyhedral mesh - edges
            ENCODE("polyhedral mesh", poly_mesh_edges_visible)
            ENCODE("polyhedral mesh", poly_mesh_edges_color)
            ENCODE("polyhedral mesh", poly_mesh_edges_imposters)
            ENCODE("polyhedral mesh", poly_mesh_edges_size)

            // clipping plane
            ENCODE("clipping plane", clipping_plane_color)

            output << std::setw(4) << settings;
            LOG(INFO) << "setting file created: " << file_name;

            return true;
        }

#define DECODE(key, var) {                                                           \
            auto pos = settings.find(key);                                           \
            if (pos != settings.end()) {                                             \
                auto val_pos = pos->find(#var);                                      \
                if (val_pos != pos->end())  (var) = *val_pos;                          \
                else LOG(WARNING) << "value does not exist for variable: " << #var;  \
            }                                                                        \
            else LOG(WARNING) << "value does not exist for variable: " << #var;      \
        }

        // Load the rendering parameters from a file.
        bool load(const std::string &file_name) {
            std::ifstream input(file_name);
            if (!input.is_open()) {
                LOG(WARNING) << "could not open file: " << file_name;
                return false;
            }

            // read a JSON file
            nlohmann::json settings;
            input >> settings;

            // background color of the viewer
            DECODE("global", background_color)
            // highlight: color for highlighted/selected primitives
            DECODE("global", highlight_color)
            // lighting
            DECODE("global", light_position)  // light position defined in camera coordinate system
            // material
            DECODE("global", material_ambient)
            DECODE("global", material_specular)
            DECODE("global", material_shininess)    // specular power

            // effect
            DECODE("effect", effect_ssao_radius)
            DECODE("effect", effect_ssao_intensity)
            DECODE("effect", effect_ssao_bias)
            DECODE("effect", effect_ssao_sharpness)
            DECODE("effect", effect_shadow_light_distance)
            DECODE("effect", effect_shadow_softness)
            DECODE("effect", effect_shadow_darkness)

            // points drawable
            DECODE("points drawable", points_drawable_two_side_lighting)
            DECODE("points drawable", points_drawable_distinct_backside_color)
            DECODE("points drawable", points_drawable_backside_color)
            // lines drawable
            DECODE("lines drawable", lines_drawable_two_side_lighting)
            DECODE("lines drawable", lines_drawable_distinct_backside_color)
            DECODE("lines drawable", lines_drawable_backside_color)
            // triangles drawable
            DECODE("triangles drawable", triangles_drawable_two_side_lighting)
            DECODE("triangles drawable", triangles_drawable_distinct_backside_color)
            DECODE("triangles drawable", triangles_drawable_backside_color)

            // point cloud
            DECODE("point cloud", point_cloud_vertices_visible)
            DECODE("point cloud", point_cloud_vertices_color)
            DECODE("point cloud", point_cloud_vertices_impostors)
            DECODE("point cloud", point_cloud_vertices_size)

            // surface mesh - surface
            DECODE("surface mesh", surface_mesh_faces_phong_shading)
            DECODE("surface mesh", surface_mesh_faces_visible)
            DECODE("surface mesh", surface_mesh_faces_color)
            DECODE("surface mesh", surface_mesh_faces_opacity)
            // surface mesh - vertices
            DECODE("surface mesh", surface_mesh_vertices_visible)
            DECODE("surface mesh", surface_mesh_vertices_color)
            DECODE("surface mesh", surface_mesh_vertices_imposters)
            DECODE("surface mesh", surface_mesh_vertices_size)
            // surface mesh - edges
            DECODE("surface mesh", surface_mesh_edges_visible)
            DECODE("surface mesh", surface_mesh_edges_color)
            DECODE("surface mesh", surface_mesh_edges_imposters)
            DECODE("surface mesh", surface_mesh_edges_size)
            // surface mesh - borders
            DECODE("surface mesh", surface_mesh_borders_visible)
            DECODE("surface mesh", surface_mesh_borders_color)
            DECODE("surface mesh", surface_mesh_borders_imposters)
            DECODE("surface mesh", surface_mesh_borders_size)

            // graph: vertices
            DECODE("graph", graph_vertices_visible)
            DECODE("graph", graph_vertices_color)
            DECODE("graph", graph_vertices_imposters)
            DECODE("graph", graph_vertices_size)
            // graph: edges
            DECODE("graph", graph_edges_visible)
            DECODE("graph", graph_edges_color)
            DECODE("graph", graph_edges_imposters)
            DECODE("graph", graph_edges_size)

            // polyhedral mesh - surface
            DECODE("polyhedral mesh", poly_mesh_faces_visible)
            DECODE("polyhedral mesh", poly_mesh_faces_color)
            // polyhedral mesh - vertices
            DECODE("polyhedral mesh", poly_mesh_vertices_visible)
            DECODE("polyhedral mesh", poly_mesh_vertices_color)
            DECODE("polyhedral mesh", poly_mesh_vertices_imposters)
            DECODE("polyhedral mesh", poly_mesh_vertices_size)
            // polyhedral mesh - edges
            DECODE("polyhedral mesh", poly_mesh_edges_visible)
            DECODE("polyhedral mesh", poly_mesh_edges_color)
            DECODE("polyhedral mesh", poly_mesh_edges_imposters)
            DECODE("polyhedral mesh", poly_mesh_edges_size)

            // clipping plane
            DECODE("clipping plane", clipping_plane_color)

            return true;
        }

    } // namespace setting

} // namespace easy3d

