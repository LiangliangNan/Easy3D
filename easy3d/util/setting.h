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

#ifndef EASY3D_UTIL_SETTING_H
#define EASY3D_UTIL_SETTING_H

#include <easy3d/core/types.h>
#include <easy3d/util/export.h>


namespace easy3d {

    /**
     * \brief Default parameters used for initializing rendering and UI
     * \namespace easy3d::setting
     */
    namespace setting {

        /**
         * \brief Initializes the default setting for rendering.
         * \details By calling this function with a valid file name, a setting file with an extension ".ini" will be
         *    created on the first startup of the program. This setting file stores the default rendering parameters.
         *    Users can modify this file to change the default rendering parameters, then the changes will be
         *    effective for the future.
		 * @param setting_file A string specifying the name of setting file. Three different values are accepted:
         *    - ""(an empty string): indicating that no setting file will be created.
		 *    - "default": create a setting file with a title in the form "AppName.ini" next to the executable file.
		 *    - any non-empty string: (if it is a valid path) a setting file with the same name will be created and
         *      rendering parameters will be written to this file.
         */
        void initialize(const std::string &setting_file = "default");

        /// Save the setting (i.e., rendering parameters) to a file.
        bool save(const std::string &filename);

        /// Load the setting (i.e., rendering parameters) from a file.
        bool load(const std::string &filename);

        /// @brief Background color of the viewer.
        /// @details This variable defines the background color used in the viewer. It is represented as a 4D vector (RGBA).
        extern EASY3D_UTIL_EXPORT vec4 background_color;
        /// @brief Default highlight color for highlighted/selected primitives.
        /// @details This variable defines the color used to highlight or select primitives (e.g., points, lines, faces).
        extern EASY3D_UTIL_EXPORT vec4 highlight_color;

        /// @brief Default light position defined in the camera coordinate system.
        /// @details This variable defines the position of the light source in the camera coordinate system.
        extern EASY3D_UTIL_EXPORT vec4 light_position;

        // Material properties
        /// @brief Default ambient color of the material.
        /// @details This variable defines the ambient color of the material, which represents the color of the material under ambient lighting.
        extern EASY3D_UTIL_EXPORT vec4 material_ambient;
        /// @brief Default specular color of the material.
        /// @details This variable defines the specular color of the material, which represents the color of the material's highlights.
        extern EASY3D_UTIL_EXPORT vec4 material_specular;
        /// @brief Default shininess of the material.
        /// @details This variable defines the shininess (specular power) of the material, which controls the size and intensity of specular highlights.
        extern EASY3D_UTIL_EXPORT float material_shininess;

        // Effects
        /// @brief Radius of the SSAO (Screen Space Ambient Occlusion) effect.
        /// @details This variable controls the radius of the SSAO effect, which determines the area around each pixel where occlusion is calculated.
        extern EASY3D_UTIL_EXPORT float effect_ssao_radius;
        /// @brief Intensity of the SSAO (Screen Space Ambient Occlusion) effect.
        /// @details This variable controls the intensity of the SSAO effect, which determines how dark the occluded areas appear.
        extern EASY3D_UTIL_EXPORT float effect_ssao_intensity;
        /// @brief Bias of the SSAO (Screen Space Ambient Occlusion) effect.
        /// @details This variable controls the bias of the SSAO effect, which helps reduce artifacts by offsetting the occlusion calculation.
        extern EASY3D_UTIL_EXPORT float effect_ssao_bias;
        /// @brief Sharpness of the SSAO (Screen Space Ambient Occlusion) effect.
        /// @details This variable controls the sharpness of the SSAO effect, which determines how sharp or smooth the occlusion edges appear.
        extern EASY3D_UTIL_EXPORT float effect_ssao_sharpness;

        /// @brief Whether EDL (Eye-Dome Lighting) effect is enabled.
        /// @details This variable enables or disables the EDL effect, which enhances depth perception by adding shading to the scene.
        extern EASY3D_UTIL_EXPORT bool effect_edl_enabled;
        /// @brief Whether transparency effect is enabled.
        /// @details This variable enables or disables the transparency effect, which allows objects to be rendered with transparency.
        extern EASY3D_UTIL_EXPORT bool effect_transparency_enabled;
        /// @brief Whether shadow effect is enabled.
        /// @details This variable enables or disables the shadow effect, which adds shadows to the scene based on the light source.
        extern EASY3D_UTIL_EXPORT bool effect_shadow_enabled;

        /// @brief Distance of the light source to the scene (w.r.t. the scene radius).
        /// @details This variable controls the distance of the light to the scene, which affects the size and softness of shadows.
        extern EASY3D_UTIL_EXPORT float effect_shadow_light_distance;
        /// @brief Softness of the shadow effect.
        /// @details This variable controls the softness of the shadow effect, which determines how sharp or blurry the shadows appear.
        extern EASY3D_UTIL_EXPORT float effect_shadow_softness;
        /// @brief Darkness of the shadow effect.
        /// @details This variable controls the darkness of the shadow effect, which determines how dark the shadows appear.
        extern EASY3D_UTIL_EXPORT float effect_shadow_darkness;

        // Points drawable settings
        /// @brief Whether two-side lighting is enabled for points drawable.
        /// @details This variable enables or disables two-side lighting for points, which ensures both sides of the points are illuminated.
        extern EASY3D_UTIL_EXPORT bool points_drawable_two_side_lighting;
        /// @brief Whether a distinct backside color is used for points drawable.
        /// @details This variable enables or disables the use of a distinct color for the backside of points.
        extern EASY3D_UTIL_EXPORT bool points_drawable_distinct_backside_color;
        /// @brief Backside color for points drawable.
        /// @details This variable defines the color used for the backside of points when `points_drawable_distinct_backside_color` is enabled.
        extern EASY3D_UTIL_EXPORT vec4 points_drawable_backside_color;

        // Lines drawable settings
        /// @brief Whether two-side lighting is enabled for lines drawable.
        /// @details This variable enables or disables two-side lighting for lines, which ensures both sides of the lines are illuminated.
        extern EASY3D_UTIL_EXPORT bool lines_drawable_two_side_lighting;
        /// @brief Whether a distinct backside color is used for lines drawable.
        /// @details This variable enables or disables the use of a distinct color for the backside of lines.
        extern EASY3D_UTIL_EXPORT bool lines_drawable_distinct_backside_color;
        /// @brief Backside color for lines drawable.
        /// @details This variable defines the color used for the backside of lines when `lines_drawable_distinct_backside_color` is enabled.
        extern EASY3D_UTIL_EXPORT vec4 lines_drawable_backside_color;

        // Triangles drawable settings
        /// @brief Whether two-side lighting is enabled for triangles drawable.
        /// @details This variable enables or disables two-side lighting for triangles, which ensures both sides of the triangles are illuminated.
        extern EASY3D_UTIL_EXPORT bool triangles_drawable_two_side_lighting;
        /// @brief Whether a distinct backside color is used for triangles drawable.
        /// @details This variable enables or disables the use of a distinct color for the backside of triangles.
        extern EASY3D_UTIL_EXPORT bool triangles_drawable_distinct_backside_color;
        /// @brief Backside color for triangles drawable.
        /// @details This variable defines the color used for the backside of triangles when `triangles_drawable_distinct_backside_color` is enabled.
        extern EASY3D_UTIL_EXPORT vec4 triangles_drawable_backside_color;

        // Point cloud settings
        /// @brief Whether point cloud vertices are visible.
        /// @details This variable enables or disables the visibility of point cloud vertices.
        extern EASY3D_UTIL_EXPORT bool point_cloud_vertices_visible;
        /// @brief Color of point cloud vertices.
        /// @details This variable defines the color used for rendering point cloud vertices.
        extern EASY3D_UTIL_EXPORT vec4 point_cloud_vertices_color;
        /// @brief Whether point cloud vertices are rendered as impostors.
        /// @details This variable enables or disables the use of impostors (e.g., spheres) for rendering point cloud vertices.
        extern EASY3D_UTIL_EXPORT bool point_cloud_vertices_impostors;
        /// @brief Size of point cloud vertices.
        /// @details This variable controls the size of point cloud vertices when rendered.
        extern EASY3D_UTIL_EXPORT float point_cloud_vertices_size;

        // Surface mesh settings
        /// @brief Whether Phong shading is enabled for surface mesh faces.
        /// @details This variable enables or disables Phong shading for surface mesh faces, which provides smoother lighting.
        extern EASY3D_UTIL_EXPORT bool surface_mesh_faces_phong_shading;
        /// @brief Whether surface mesh faces are visible.
        /// @details This variable enables or disables the visibility of surface mesh faces.
        extern EASY3D_UTIL_EXPORT bool surface_mesh_faces_visible;
        /// @brief Color of surface mesh faces.
        /// @details This variable defines the color used for rendering surface mesh faces.
        extern EASY3D_UTIL_EXPORT vec4 surface_mesh_faces_color;
        /// @brief Opacity of surface mesh faces.
        /// @details This variable controls the opacity of surface mesh faces, where 1.0 is fully opaque and 0.0 is fully transparent.
        extern EASY3D_UTIL_EXPORT float surface_mesh_faces_opacity;
        /// @brief Whether surface mesh vertices are visible.
        /// @details This variable enables or disables the visibility of surface mesh vertices.
        extern EASY3D_UTIL_EXPORT bool surface_mesh_vertices_visible;
        /// @brief Color of surface mesh vertices.
        /// @details This variable defines the color used for rendering surface mesh vertices.
        extern EASY3D_UTIL_EXPORT vec4 surface_mesh_vertices_color;
        /// @brief Whether surface mesh vertices are rendered as impostors.
        /// @details This variable enables or disables the use of impostors (e.g., spheres) for rendering surface mesh vertices.
        extern EASY3D_UTIL_EXPORT bool surface_mesh_vertices_imposters;
        /// @brief Size of surface mesh vertices.
        /// @details This variable controls the size of surface mesh vertices when rendered.
        extern EASY3D_UTIL_EXPORT float surface_mesh_vertices_size;
        /// @brief Whether surface mesh edges are visible.
        /// @details This variable enables or disables the visibility of surface mesh edges.
        extern EASY3D_UTIL_EXPORT bool surface_mesh_edges_visible;
        /// @brief Color of surface mesh edges.
        /// @details This variable defines the color used for rendering surface mesh edges.
        extern EASY3D_UTIL_EXPORT vec4 surface_mesh_edges_color;
        /// @brief Whether surface mesh edges are rendered as impostors.
        /// @details This variable enables or disables the use of impostors (e.g., cylinders) for rendering surface mesh edges.
        extern EASY3D_UTIL_EXPORT bool surface_mesh_edges_imposters;
        /// @brief Size of surface mesh edges.
        /// @details This variable controls the size of surface mesh edges when rendered.
        extern EASY3D_UTIL_EXPORT float surface_mesh_edges_size;
        /// @brief Whether surface mesh borders are visible.
        /// @details This variable enables or disables the visibility of surface mesh borders.
        extern EASY3D_UTIL_EXPORT bool surface_mesh_borders_visible;
        /// @brief Color of surface mesh borders.
        /// @details This variable defines the color used for rendering surface mesh borders.
        extern EASY3D_UTIL_EXPORT vec4 surface_mesh_borders_color;
        /// @brief Whether surface mesh borders are rendered as impostors.
        /// @details This variable enables or disables the use of impostors (e.g., cylinders) for rendering surface mesh borders.
        extern EASY3D_UTIL_EXPORT bool surface_mesh_borders_imposters;
        /// @brief Size of surface mesh borders.
        /// @details This variable controls the size of surface mesh borders when rendered.
        extern EASY3D_UTIL_EXPORT float surface_mesh_borders_size;

        // Graph settings
        /// @brief Whether graph vertices are visible.
        /// @details This variable enables or disables the visibility of graph vertices.
        extern EASY3D_UTIL_EXPORT bool graph_vertices_visible;
        /// @brief Color of graph vertices.
        /// @details This variable defines the color used for rendering graph vertices.
        extern EASY3D_UTIL_EXPORT vec4 graph_vertices_color;
        /// @brief Whether graph vertices are rendered as impostors.
        /// @details This variable enables or disables the use of impostors (e.g., spheres) for rendering graph vertices.
        extern EASY3D_UTIL_EXPORT bool graph_vertices_imposters;
        /// @brief Size of graph vertices.
        /// @details This variable controls the size of graph vertices when rendered.
        extern EASY3D_UTIL_EXPORT float graph_vertices_size;
        /// @brief Whether graph edges are visible.
        /// @details This variable enables or disables the visibility of graph edges.
        extern EASY3D_UTIL_EXPORT bool graph_edges_visible;
        /// @brief Color of graph edges.
        /// @details This variable defines the color used for rendering graph edges.
        extern EASY3D_UTIL_EXPORT vec4 graph_edges_color;
        /// @brief Whether graph edges are rendered as impostors.
        /// @details This variable enables or disables the use of impostors (e.g., cylinders) for rendering graph edges.
        extern EASY3D_UTIL_EXPORT bool graph_edges_imposters;
        /// @brief Size of graph edges.
        /// @details This variable controls the size of graph edges when rendered.
        extern EASY3D_UTIL_EXPORT float graph_edges_size;

        // Polyhedral mesh settings
        /// @brief Whether polyhedral mesh faces are visible.
        /// @details This variable enables or disables the visibility of polyhedral mesh faces.
        extern EASY3D_UTIL_EXPORT bool poly_mesh_faces_visible;
        /// @brief Color of polyhedral mesh faces.
        /// @details This variable defines the color used for rendering polyhedral mesh faces.
        extern EASY3D_UTIL_EXPORT vec4 poly_mesh_faces_color;
        /// @brief Whether polyhedral mesh vertices are visible.
        /// @details This variable enables or disables the visibility of polyhedral mesh vertices.
        extern EASY3D_UTIL_EXPORT bool poly_mesh_vertices_visible;
        /// @brief Color of polyhedral mesh vertices.
        /// @details This variable defines the color used for rendering polyhedral mesh vertices.
        extern EASY3D_UTIL_EXPORT vec4 poly_mesh_vertices_color;
        /// @brief Whether polyhedral mesh vertices are rendered as impostors.
        /// @details This variable enables or disables the use of impostors (e.g., spheres) for rendering polyhedral mesh vertices.
        extern EASY3D_UTIL_EXPORT bool poly_mesh_vertices_imposters;
        /// @brief Size of polyhedral mesh vertices.
        /// @details This variable controls the size of polyhedral mesh vertices when rendered.
        extern EASY3D_UTIL_EXPORT float poly_mesh_vertices_size;
        /// @brief Whether polyhedral mesh edges are visible.
        /// @details This variable enables or disables the visibility of polyhedral mesh edges.
        extern EASY3D_UTIL_EXPORT bool poly_mesh_edges_visible;
        /// @brief Color of polyhedral mesh edges.
        /// @details This variable defines the color used for rendering polyhedral mesh edges.
        extern EASY3D_UTIL_EXPORT vec4 poly_mesh_edges_color;
        /// @brief Whether polyhedral mesh edges are rendered as impostors.
        /// @details This variable enables or disables the use of impostors (e.g., cylinders) for rendering polyhedral mesh edges.
        extern EASY3D_UTIL_EXPORT bool poly_mesh_edges_imposters;
        /// @brief Size of polyhedral mesh edges.
        /// @details This variable controls the size of polyhedral mesh edges when rendered.
        extern EASY3D_UTIL_EXPORT float poly_mesh_edges_size;

        // Clipping plane settings
        /// @brief Default color of the clipping plane.
        /// @details This variable defines the color used for rendering the clipping plane.
        extern EASY3D_UTIL_EXPORT vec4 clipping_plane_color;

    } // namespace setting

} // namespace easy3d


#endif // EASY3D_UTIL_SETTING_H
