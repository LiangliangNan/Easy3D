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

#ifndef EASY3D_RENDERER_STATE_H
#define EASY3D_RENDERER_STATE_H

#include <string>

#include <easy3d/core/types.h>


namespace easy3d {

    class Texture;

    /**
     * \brief Class representing the rendering state of a drawable.
     * \class State easy3d/renderer/state.h
     */
    class State {
    public:

        /**
         * \brief Available coloring methods.
         * \details A complete description of a color scheme contains the coloring method, and (if not uniform color)
         *      the location and name of a property. Possible properties for coloring a drawable:
         *          - color properties defined on vertices, faces, or edges. A color property is specified by its name,
         *            e.g., "v:color", "f:color", "e:color";
         *          - scalar fields defined on vertices, faces, or edges. A scalar field is specified by its name, e.g.,
         *            "v:curvature", "v:height", "f:segment_id";
         *          - texture coordinates defined on vertices or halfedges. A texture coordinates property is specified
         *            by its name, e.g., "v:texcoord", "h:texcoord".
         */
        enum Method {
            UNIFORM_COLOR,  ///< Uniformly colored
            COLOR_PROPERTY, ///< Using a color property
            SCALAR_FIELD,   ///< Using a scalar field
            TEXTURED        ///< Using texture(s)
        };

        /**
         * \brief The location of a coloring property.
         * \details It is denoted by the type of the geometric primitives on which the property is defined.
         */
        enum Location {
            VERTEX,     ///< Property defined on vertices
            FACE,       ///< Property defined on faces
            EDGE,       ///< Property defined on edges
            HALFEDGE    ///< Property defined on halfedges
        };

        /**
         * Material definition.
         */
        struct Material {
            Material() = default;
            Material(const vec4 &ambi, const vec4 &spec, float shin);

            vec4 ambient;       ///< Ambient color
            //vec3    diffuse; // we have per face/point/line color!
            vec4 specular;      ///< Specular color
            float shininess;    ///< Specular power
        };

    public:
        State();
        State(const State& s);
        /// assign \c rhs to \c *this. performs a deep copy of all member variables.
        State& operator=(const State& rhs);

        virtual ~State() = default;

        /**
         * \brief Checks if the drawable is visible.
         * \return \c true if visible, \c false otherwise.
         */
        bool is_visible() const { return visible_; }
        /**
         * \brief Sets the visibility of the drawable.
         * \param v \c true to make visible, \c false to hide.
         */
        void set_visible(bool v) { visible_ = v; }

        /**
         * \brief Checks if the drawable is selected.
         * \return \c true if selected, \c false otherwise.
         */
        bool is_selected() const { return selected_; }
        /**
         * \brief Sets the selection state of the drawable.
         * \param b \c true to select, \c false to deselect.
         */
        void set_selected(bool b) { selected_ = b; }

        /// \name Coloring
        //@{

        /**
         * \brief Constructs a uniform coloring scheme.
         * \param color The color.
         */
        void set_uniform_coloring(const vec4 &color);

        /**
         * \brief Constructs a scheme for rendering a drawable with per-element color.
         * \param color_location The location of the color property.
         * \param color_name The name of the color property.
         */
        void set_property_coloring(Location color_location, const std::string &color_name = "");

        /**
         * \brief Constructs a scheme for textured rendering.
         * \param texcoord_location The location of texture coordinate.
         * \param texcoord_name The name of the texture coordinate property.
         * \param texture The pointer to the texture. If \c nullptr, a default texture will be created from
         *      "textures/checkerboard.png". Client code is expected to provide a meaningful texture at a later stage.
         * \param repeat The repeat factor of the texture. Default value is 1.0.
         * \param repeat_fraction The fractional repeat factor of the texture. Default value is 0.0.
         */
        void set_texture_coloring(Location texcoord_location, const std::string &texcoord_name,
                                  const Texture *texture = nullptr, float repeat = 1.0f,
                                  float repeat_fraction = 0.0f);

        /**
         * \brief Constructs a scheme for rendering scalar fields.
         * \param scalar_location The location of the scalar field.
         * \param scalar_name The name of the scalar field.
         * \param texture The pointer to the texture. Default is \c nullptr, a scalar default texture will be created
         *      from "/colormaps/default.png". Client code can provide a more suitable texture at a later stage.
         * \param clamp_lower The percentage of values to be clamped at the lower side of the range. Default is 5%.
         * \param clamp_upper The percentage of values to be clamped at the upper side of the range. Default is 5%.
         */
        void set_scalar_coloring(Location scalar_location, const std::string &scalar_name,
                                 const Texture *texture = nullptr, float clamp_lower = 0.05f,
                                 float clamp_upper = 0.05f);

        /**
         * \brief Sets the coloring.
         * \details A generic version of the set_[method]_coloring() method.
         * \param method The coloring method.
         * \param location The location of the coloring property.
         * \param name The name of the coloring property.
         */
        void set_coloring(Method method, Location location, const std::string &name);

        /**
         * \brief Sets the coloring method.
         * \details It has the same effect as if set_coloring() is called without changing the location (2nd argument)
         *      and name (3rd argument) of the coloring property.
         * \param method The coloring method.
         */
        void set_coloring_method(Method method) { coloring_method_ = method; }
        /**
         * \brief Returns the coloring method being used for rendering.
         * \return The coloring method.
         */
        Method coloring_method() const { return coloring_method_; }

        /**
         * \brief Returns the color, which is effective only when the coloring method was set to UNIFORM_COLOR.
         * \return The color.
         */
        const vec4 &color() const { return color_; }
        /**
         * \brief Sets the color.
         * \param c The color.
         */
        void set_color(const vec4& c) { color_ = c; }

        /**
         * \brief Returns the location of the color property.
         * \return The location of the color property.
         */
        Location property_location() const { return property_location_; }

        /**
         * \brief Returns the name of the color attribute.
         * \return The name of the color attribute.
         */
        const std::string &property_name() const { return property_name_; }

        //@}

        /**
         * \brief Checks if lighting is enabled.
         * \return \c true if lighting is enabled, \c false otherwise.
         */
        bool lighting() const { return lighting_; }
        /**
         * \brief Enables or disables lighting.
         * \param l \c true to enable lighting, \c false to disable.
         */
        void set_lighting(bool l) { lighting_ = l; }

        /**
         * \brief Checks if double-sided lighting is enabled.
         * \return \c true if double-sided lighting is enabled, \c false otherwise.
         */
        bool lighting_two_sides() const { return lighting_two_sides_; }
        /**
         * \brief Enables or disables double-sided lighting.
         * \param b \c true to enable double-sided lighting, \c false to disable.
         */
        void set_lighting_two_sides(bool b) { lighting_two_sides_ = b; }

        /**
         * \brief Checks if a different color is used for rendering the backside of a drawable.
         * \details Effective only when two-sides lighting is enabled.
         * \return \c true if a different backside color is used, \c false otherwise.
         */
        bool distinct_back_color() const { return distinct_back_color_; }
        /**
         * \brief Enables or disables different backside color.
         * \details Effective only when two-sides lighting is enabled.
         * \param b \c true to enable different backside color, \c false to disable.
         */
        void set_distinct_back_color(bool b) { distinct_back_color_ = b; }

        /**
         * \brief Returns the backside color.
         * \details Effective only when two-sides lighting and distinct back color are enabled.
         * \return The backside color.
         */
        const vec4 &back_color() const { return back_color_; }
        /**
         * \brief Sets the backside color.
         * \details Effective only when two-sides lighting and distinct back color are enabled.
         * \param c The backside color.
         */
        void set_back_color(const vec4 &c) { back_color_ = c; }

        /**
         * \brief Returns the texture.
         * \details Memory management of textures is the user's responsibility.
         * \return The texture.
         */
        const Texture *texture() const { return texture_; }
        /**
         * \brief Sets the texture.
         * \param tex The texture.
         */
        void set_texture(const Texture *tex) { texture_ = tex; }

        /**
         * \brief Returns the repeat factor of the texture.
         * \return The repeat factor.
         */
        float texture_repeat() const { return texture_repeat_; }
        /**
         * \brief Sets the repeat factor of the texture.
         * \param r The repeat factor.
         */
        void set_texture_repeat(float r) { texture_repeat_ = r; }

        /**
         * \brief Returns the fractional repeat factor of the texture.
         * \details Controls the texture repeat at a finer level: 100 fractional repeat == 1 repeat.
         * \return The fractional repeat factor.
         */
        float texture_fractional_repeat() const { return texture_fractional_repeat_; }
        /**
         * \brief Sets the fractional repeat factor of the texture.
         * \details Controls the texture repeat at a finer level: 100 fractional repeat == 1 repeat.
         * \param fr The fractional repeat factor.
         */
        void set_texture_fractional_repeat(float fr) { texture_fractional_repeat_ = fr; }

        /**
         * \brief Checks if SSAO is enabled.
         * \return \c true if SSAO is enabled, \c false otherwise.
         */
        bool is_ssao_enabled() const { return ssao_enabled_; }
        /**
         * \brief Enables or disables SSAO.
         * \param b \c true to enable SSAO, \c false to disable.
         */
        void enable_ssao(bool b) { ssao_enabled_ = b; }
        /**
         * \brief Sets the SSAO texture.
         * \param tex The SSAO texture.
         */
        void set_ssao_texture(unsigned int tex) { ssao_texture_ = tex; }

        /**
         * \brief Checks if the value range of a scalar field is clamped.
         * \return \c true if the value range is clamped, \c false otherwise.
         */
        bool clamp_range() const { return clamp_range_; }
        /**
         * \brief Sets the clamping of the value range of a scalar field.
         * \param b \c true to clamp the value range, \c false to disable clamping.
         */
        void set_clamp_range(bool b) { clamp_range_ = b; }

        /**
         * \brief Returns the percentage of values clamped at the lower side of the range.
         * \details Value is in [0, 1] (i.e., 0% to 100%).
         * \return The percentage of values clamped at the lower side.
         */
        float clamp_lower() const { return clamp_lower_; }
        /**
         * \brief Sets the percentage of values clamped at the lower side of the range.
         * \details Value is in [0, 1] (i.e., 0% to 100%).
         * \param v The percentage of values clamped at the lower side.
         */
        void set_clamp_lower(float v) { clamp_lower_ = v; }

        /**
         * \brief Returns the percentage of values clamped at the upper side of the range.
         * \details Value is in [0, 1] (i.e., 0% to 100%).
         * \return The percentage of values clamped at the upper side.
         */
        float clamp_upper() const { return clamp_upper_; }
        /**
         * \brief Sets the percentage of values clamped at the upper side of the range.
         * \details Value is in [0, 1] (i.e., 0% to 100%).
         * \param v The percentage of values clamped at the upper side.
         */
        void set_clamp_upper(float v) { clamp_upper_ = v; }

        /**
         * \brief Returns the material.
         * \return The material.
         */
        Material &material() { return material_; }
        /**
         * \brief Returns the material (const version).
         * \return The material.
         */
        const Material &material() const { return material_; }
        /**
         * \brief Sets the material.
         * \param m The material.
         */
        void set_material(const Material &m) { material_ = m; }

        /**
         * \brief Controls the behavior for vertex clipping.
         * \details If plane_clip_discard_primitive() is \c false (default value, standard plane clip), the clip
         *      distances will be linearly interpolated across the primitive (e.g., line, triangle) and the portion
         *      of the primitive with interpolated distances less than 0.0 will be clipped.
         *      If plane_clip_discard_primitive() is \c true, a primitive will be completely discarded if one of its
         *      vertices has a negative clip distance.
         * \return \c true if the primitive is discarded, \c false otherwise.
         */
        bool plane_clip_discard_primitive() const { return plane_clip_discard_primitive_; }
        /**
         * \brief Sets the behavior for vertex clipping.
         * \details If plane_clip_discard_primitive() is \c false (default value, standard plane clip), the clip
         *      distances will be linearly interpolated across the primitive (e.g., line, triangle) and the portion
         *      of the primitive with interpolated distances less than 0.0 will be clipped.
         *      If plane_clip_discard_primitive() is \c true, a primitive will be completely discarded if one of its
         *      vertices has a negative clip distance.
         * \param b \c true to discard the primitive, \c false otherwise.
         */
        void set_plane_clip_discard_primitive(bool b) { plane_clip_discard_primitive_ = b; }

        /**
         * \brief Checks if a subset of primitives of this drawable is highlighted.
         * \return \c true if highlighted, \c false otherwise.
         */
        bool highlight() const { return highlight_; }
        /**
         * \brief Sets the highlight state of a subset of primitives of this drawable.
         * \param b \c true to highlight, \c false to un-highlight.
         */
        void set_highlight(bool b) { highlight_ = b; }
        /**
         * \brief Sets the range of primitives to be highlighted.
         * \details Primitives with indices within the range will be highlighted.
         * \param range Specifies the min and max indices of the primitives to be highlighted. Providing [-1, -1] will
         *              un-highlight any previously highlighted primitives.
         * \attention For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a
         *      unified rendering APIs. The range must be given on the triangulated faces! If you have face range,
         *      you should convert the face range to triangle range. For each face, its triangles are defined on
         *      SurfaceMesh::face_property<std::pair<int, int> >("f:triangle_range").
         */
        void set_highlight_range(const std::pair<int, int> &range) { highlight_range_ = range; }
        /**
         * \brief Returns the range of primitives to be highlighted.
         * \details Primitives with indices within the range will be highlighted.
         * \return The range of primitives to be highlighted.
         */
        const std::pair<int, int> &highlight_range() const { return highlight_range_; }

    protected:
        // Create a default texture.
        // Default texture file: "/textures/checkerboard.png";
        Texture* create_color_texture();

        // Create a default texture for rendering scalar fields.
        // Default texture file: "/colormaps/default.png";
        Texture* create_scalar_texture();

    protected:
        bool visible_;
        bool selected_;

        Method coloring_method_;
        vec4 color_; // valid when color method is UNIFORM_COLOR
        Location property_location_;
        std::string property_name_;

        bool lighting_;

        bool lighting_two_sides_;
        bool distinct_back_color_;
        vec4 back_color_;

        // texture
        const Texture *texture_;
        // How many times do you want to repeat the texture?
        float texture_repeat_;
        // Control at a finer level: 100 fractional repeat == 1 repeat.
        float texture_fractional_repeat_;

        bool ssao_enabled_;
        unsigned int ssao_texture_;

        bool clamp_range_;
        float clamp_lower_;
        float clamp_upper_;

        Material material_;

        // Clipping plane behavior.
        //  - true: completely discard a primitive if one of its vertices has a negative clip distance
        //  - false: linearly interpolated (standard plane clipping behavior)
        bool plane_clip_discard_primitive_;

        // highlight the primitives within the range [highlight_id_low_, highlight_id_high_]
        bool highlight_;
        std::pair<int, int> highlight_range_;
    };

}


#endif  // EASY3D_RENDERER_STATE_H
