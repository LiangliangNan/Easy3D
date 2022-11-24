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
     * @brief Class representing the rendering state of a drawable
     *
     * \class State easy3d/renderer/state.h
     */

    class State {
    public:

        /**
         * A complete description of a color scheme contains the coloring method, and (if not uniform color) the
         * location and name of a property.
         * Available coloring methods:
         *   - UNIFORM_COLOR: uniformly colored;
         *   - COLOR_PROPERTY: using a color property;
         *   - SCALAR_FIELD: using a scalar field;
         *   - TEXTURED: using texture(s).
         * Possible properties for coloring a drawable:
         *   - color properties defined on vertices, faces, or edges. A color property is specified by its name,
         *     e.g., "v:color", "f:color", "e:color";
         *   - scalar fields defined on vertices, faces, or edges. A scalar field is specified by its name, e.g.,
         *     "v:curvature", "v:height", "f:segment_id";
         *   - texture coordinates defined on vertices or halfedges. A texture coordinates property is specified by its
         *     name, e.g., "v:texcoord", "h:texcoord".
         */
        enum Method {
            UNIFORM_COLOR, COLOR_PROPERTY, SCALAR_FIELD, TEXTURED
        };

        /**
         * The location of a coloring property. It is denoted by the type of the geometric primitives on which the
         * property is defined.
         */
        enum Location {
            VERTEX, FACE, EDGE, HALFEDGE
        };

        /**
         * Material definition.
         */
        struct Material {
            Material() = default;
            Material(const vec4 &ambi, const vec4 &spec, float shin);

            vec4 ambient;
            //vec3    diffuse; // we have per face/point/line color!
            vec4 specular;
            float shininess;   // specular power
        };

    public:
        State();
        State(const State& s);
        /// assign \c rhs to \c *this. performs a deep copy of all member variables.
        State& operator=(const State& rhs);

        virtual ~State() = default;

        bool is_visible() const { return visible_; }
        void set_visible(bool v) { visible_ = v; }

        bool is_selected() const { return selected_; }
        void set_selected(bool b) { selected_ = b; }

        /// \name Coloring

        /**
         * Constructs a uniform coloring scheme.
         * @param color The color.
         */
        void set_uniform_coloring(const vec4 &color);

        /**
        * Constructs a scheme for rendering a drawable with per-element color.
        * @param color_location The location of the color property.
        * @param color_name The name of the color property.
        */
        void set_property_coloring(Location color_location, const std::string &color_name = "");

        /**
         * Constructs a scheme for textured rendering.
         * @param texcoord_location The location of texture coordinate.
         * @param texcoord_name The name of the texture coordinate property.
         * @param texture The pointer to the texture.
         * @param repeat The repeat factor of the texture. Default value is 1.0.
         * @param repeat_fraction The fractional repeat factor of the texture. Default value is 0.0.
         */
        void set_texture_coloring(Location texcoord_location, const std::string &texcoord_name,
                                  const Texture *texture = nullptr, float repeat = 1.0f,
                                  float repeat_fraction = 0.0f);

        /**
         * Constructs a scheme for rendering scalar fields.
         * @param scalar_location The location of the scalar field.
         * @param scalar_name The name of the scalar field.
         * @param texture The pointer to the texture.
         * @param clamp_lower The percentage of values to be clamped at the lower side of the range. Default is 5%.
         * @param clamp_upper The percentage of values to be clamped at the upper side of the range. Default is 5%.
         */
        void set_scalar_coloring(Location scalar_location, const std::string &scalar_name,
                                 const Texture *texture = nullptr, float clamp_lower = 0.05f,
                                 float clamp_upper = 0.05f);

        /**
         * Sets the coloring. A generic version of the set_[method]_coloring() method.
         * @param method The coloring method.
         * @param location The the location of the coloring property.
         * @param name The name of the coloring property.
         * @param texture The texture for the coloring.
         */
        void set_coloring(Method method, Location location, const std::string &name);

        /**
         * Sets the coloring method.
         * \details It has the same effect as if set_coloring() is called without changing the location (2nd argument)
         *      and name (3rd argument) of the coloring property. \see set_coloring()
         * @param method The coloring method.
         */
        void set_coloring_method(Method method) { coloring_method_ = method; }

        /** Returns the coloring method being used for rendering. */
        Method coloring_method() const { return coloring_method_; }

        /**
         * The color, which is effective only when the coloring method was set to UNIFORM_COLOR.
         * Call set_uniform_coloring() to change this color.
         */
        const vec4 &color() const { return color_; }
        void set_color(const vec4& c) { color_ = c; }

        /** The location of the color property. */
        Location property_location() const { return property_location_; }

        /** The name of the color attribute. */
        const std::string &property_name() const { return property_name_; }

        /// returns whether lighting is enabled.
        bool lighting() const { return lighting_; }
        /// enables/disables lighting.
        void set_lighting(bool l) { lighting_ = l; }

        /// returns whether double-sided lighting is enabled.
        bool lighting_two_sides() const { return lighting_two_sides_; }
        /// enables/disables double-sided lighting.
        void set_lighting_two_sides(bool b) { lighting_two_sides_ = b; }

        /// returns whether a different color is used for rendering the backside of a drawable.
        /// \note effective only when two-sides lighting is enabled
        bool distinct_back_color() const { return distinct_back_color_; }
        /// enables/disables different backside color.
        /// \note effective only when two-sides lighting is enabled
        void set_distinct_back_color(bool b) { distinct_back_color_ = b; }

        /// returns the backside color.
        /// \note effective only when two-sides lighting and distinct back color are enabled.
        /// \sa lighting_two_sides(), distinct_back_color()
        const vec4 &back_color() const { return back_color_; }
        /// sets the backside color.
        /// \note effective only when two-sides lighting and distinct back color are enabled.
        /// \sa set_lighting_two_sides(), set_distinct_back_color()
        void set_back_color(const vec4 &c) { back_color_ = c; }

        /** Memory management of textures is the user's responsibility. */
        const Texture *texture() const { return texture_; }
        void set_texture(Texture *tex) { texture_ = tex; }

        /** How many times do you want to repeat the texture? */
        float texture_repeat() const { return texture_repeat_; };
        void set_texture_repeat(float r) { texture_repeat_ = r; };

        /** Controls the texture repeat at a finer level: 100 fractional repeat == 1 repeat. */
        float texture_fractional_repeat() const { return texture_fractional_repeat_; };
        void set_texture_fractional_repeat(float fr) { texture_fractional_repeat_ = fr; };

        // \todo: reuse the texture() and set_texture()
        bool is_ssao_enabled() const { return ssao_enabled_; }
        void enable_ssao(bool b) { ssao_enabled_ = b; }
        void set_ssao_texture(unsigned int tex) { ssao_texture_ = tex; }

        /** Clamp the value range of a scalar field. */
        bool clamp_range() const { return clamp_range_; }
        void set_clamp_range(bool b) { clamp_range_ = b; }

        /** Clamp the lower side of the value range of a scalar field. Value is in [0, 1] (i.e., 0% to 100%).*/
        float clamp_lower() const { return clamp_lower_; }
        void set_clamp_lower(float v) { clamp_lower_ = v; }

        /** Clamp the upper side of the value range of a scalar field. Value is in [0, 1] (i.e., 0% to 100%).*/
        float clamp_upper() const { return clamp_upper_; }
        void set_clamp_upper(float v) { clamp_upper_ = v; }

        Material &material() { return material_; }
        const Material &material() const { return material_; }
        void set_material(const Material &m) { material_ = m; }

        /**
         * \brief Controls the behavior for vertex clipping.
         * \details If plane_clip_discard_primitive() is \c false (default value, standard plane clip), the clip
         *      distances will be linearly interpolated across the primitive (e.g., line, triangle) and the portion
         *      of the primitive with interpolated distances less than 0.0 will be clipped.
         *      If plane_clip_discard_primitive() is \c true, a primitive will be completely discarded if one of its
         *      vertices has a negative clip distance.
         */
        bool plane_clip_discard_primitive() const { return plane_clip_discard_primitive_; };
        void set_plane_clip_discard_primitive(bool b) { plane_clip_discard_primitive_ = b; };

        /**
         * Highlight a subset of primitives of this drawable. Primitives with indices within the range
         * [highlight_id_low_, highlight_id_high_] will be highlighted.
         * @param range Specifies the min and max indices of the primitives to be highlighted. Providing [-1, -1] will
         *              un-highlight any previously highlighted primitives.
         * @attention For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a
         *            unified rendering APIs. The range must be given on the triangulated faces! If you have face range,
         *            you should convert the face range to triangle range. For each face, its triangles are defined on
         *            SurfaceMesh::face_property<std::pair<int, int> >("f:triangle_range").
         */
        bool highlight() const { return highlight_; };
        void set_highlight(bool b) { highlight_ = b; }
        void set_highlight_range(const std::pair<int, int> &range) { highlight_range_ = range; }
        const std::pair<int, int> &highlight_range() const { return highlight_range_; }

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
        // Control at a finer level: 100 fractional repeat == repeat.
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
