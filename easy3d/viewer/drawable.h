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

#ifndef EASY3D_DRAWABLE_H
#define EASY3D_DRAWABLE_H

#include <string>
#include <vector>

#include <easy3d/core/types.h>

/********************************************************************************************
* A Drawable is a general abstraction for "something that can be drawn."
* Drawable is the base class for drawable objects (i.e., points, line segments, triangles).
* A drawable manages the rendering of an objects. When data (vertex coordinates/colors/normals,
* selection, etc.) changes, it controls the upload of the data to GPU.
*********************************************************************************************/

namespace easy3d {

    class Model;
    class Camera;
    class Texture;
    class VertexArrayObject;


    struct Material {
        Material();
        Material(const vec3 &ambi, const vec3 &spec, float shin);
        vec3 ambient;
        //vec3    diffuse; // we have per face/point/line color!
        vec3 specular;
        float shininess;   // specular power
    };


    struct ColorScheme {
        // where is the color from?
        enum Source { UNIFORM_COLOR, COLOR_PROPERTY, TEXTURE, SCALAR_FIELD };
        // where is the necessary property stored?
        enum Location { FACE, VERTEX, EDGE, HALFEDGE };

        ColorScheme(Source src = UNIFORM_COLOR, Location loc = VERTEX) : source(src), location(loc),
                                                                         name("uniform color"),
                                                                         clamp_value(true),
                                                                         dummy_lower(0.05f), dummy_upper(0.05f) {}
        Source source;
        Location location;
        std::string name;   // the name of the property
        bool clamp_value;
        float dummy_lower;  // the percentage of values to be clamped (used by rendering scalar field)
        float dummy_upper;  // the percentage of values to be clamped (used by rendering scalar field)
    };


    class Drawable {
    public:
        // three types of drawables (the values are the same as GL_POINTS, GL_LINES, and GL_TRIANGLES).
        enum Type {
            DT_POINTS = 0x0000,
            DT_LINES = 0x0001,
            DT_TRIANGLES = 0x0004
        };

    public:
        // a drawable can be stand-alone or attached to a model
        Drawable(const std::string &name = "unknown", Model *model = nullptr);
        virtual ~Drawable();

        /// Returns the type of the drawable.
        virtual Type type() const = 0;

        const std::string &name() const { return name_; }
        void set_name(const std::string n) { name_ = n; }

        /// the model to which the drawable is attached to (can be NULL).
        Model *model() { return model_; }
        const Model *model() const { return model_; }
        void set_model(Model *m) { model_ = m; }

        const Box3 &bounding_box() const { return bbox_; }

        /// print statistics (e.g., num vertices, memory usage) of the buffers to an output stream (e.g., std::cout).
        void buffer_stats(std::ostream &output) const;

        // ---------------------- buffer access ------------------------------

        unsigned int vertex_buffer() const { return vertex_buffer_; }
        unsigned int color_buffer() const { return color_buffer_; }
        unsigned int normal_buffer() const { return normal_buffer_; }
        unsigned int texcoord_buffer() const { return texcoord_buffer_; }
        unsigned int index_buffer() const { return index_buffer_; }
        unsigned int storage_buffer() const { return storage_buffer_; }
        unsigned int selection_buffer() const { return selection_buffer_; }

        // ------------------- buffer creation/update ------------------------

        /// To render non-standard model drawables (e.g., visualized the locked vertices of a mesh), a user can provide
        /// a customized function for update the OpenGL buffers.
        /// @related Standard drawables can be easily updated by calling the "renderer::update_buffer(...)" functions.
        void set_update_func(std::function<void(Model*, Drawable*)> func) { update_func_ = func; }

        /// Update all the OpenGL buffers of this drawable.
        /// By default, the "renderer::update_buffer(...)" function is called. For non-standard drawables, a user needs
        /// to provide the update function by calling the "set_update_func(...)" function.
        void update();

        /// Creates/Updates a single buffer.
        void update_vertex_buffer(const std::vector<vec3> &vertices);
        void update_color_buffer(const std::vector<vec3> &colors);
        void update_normal_buffer(const std::vector<vec3> &normals);
        void update_texcoord_buffer(const std::vector<vec2> &texcoords);
        void update_index_buffer(const std::vector<unsigned int> &indices);

        /// selection buffer (internally based on a shader storage buffer)
        /// @param index: the index of the binding point.
        /// NOTE: the buffers should also be bound to this point in all shader code
        void update_selection_buffer(unsigned int index = 1);

        /// generic storage buffer
        /// @param index: the index of the binding point.
        /// NOTE: the buffers should also be bound to this point in all shader code
        void update_storage_buffer(const void *data, std::size_t datasize, unsigned int index = 0);

        /// Releases the index buffer if existing vertex data is sufficient (may require duplicating vertex data).
        void release_element_buffer();

        /// Sets the drawable modified when the rendering needs update.
        void set_modified(bool b = true) { modified_ = b; }
        bool is_modified() const { return modified_; }

        // ---------------------- get data from GPU -------------------------

        void fetch_selection_buffer();

        // -------------------------- rendering -----------------------------

        bool is_visible() const { return visible_; }
        void set_visible(bool v) { visible_ = v; }

        /**
         * A drawable can be colored in the following ways:
         *      - using one of the color properties (e.g., "v:color", "f:color", or "e:color");
         *      - textured using one of the texture coordinates (e.g., "v:texcoord", "h:texcoord").
         *      - textured using using the scalar field defined on the model.
         * The active color is a string denoting how this drawable is colored.
         * @return The string denoting the current color scheme.
         */
        ColorScheme &color_scheme() { return color_scheme_; }
        const ColorScheme &color_scheme() const { return color_scheme_; }

        /// if true, one of the color properties (e.g., "v:color", "f:color", or "e:color") will be active for rendering.
        bool per_vertex_color() const { return per_vertex_color_; }
        void set_per_vertex_color(bool b) { per_vertex_color_ = b; }

        /// default_color will be ignored if per_vertex_color is true and given.
        const vec4 &default_color() const { return default_color_; }
        void set_default_color(const vec4 &c) { default_color_ = c; }

        bool lighting() const { return lighting_; }
        void set_lighting(bool l) { lighting_ = l; }

        bool lighting_two_sides() const { return lighting_two_sides_; }
        void set_lighting_two_sides(bool b) { lighting_two_sides_ = b; }

        /// use a different color for the back side, valid only if two-side lighting is enabled
        bool distinct_back_color() const { return distinct_back_color_; }
        void set_distinct_back_color(bool b) { distinct_back_color_ = b; }

        /// the back side color, valid only if two-side lighting is enabled and distinct back color is true
        const vec4 &back_color() const { return back_color_; }
        void set_back_color(const vec4 &c) { back_color_ = c; }

        Material &material() { return material_; }
        const Material &material() const { return material_; }
        void set_material(const Material &m) { material_ = m; }

        bool use_texture() const { return use_texture_; }
        void set_use_texture(bool b) { use_texture_ = b; }

        /** Memory management of textures is the user's responsibility. */
        Texture *texture() const { return texture_; }
        void set_texture(Texture *tex) { texture_ = tex; }

        /// How many time do you want to repeat the texture?
        float texture_repeat() const { return texture_repeat_; };
        void set_texture_repeat(float r) { texture_repeat_ = r; };

        // Control at a finer level: 100 fractional repeat == repeat.
        float texture_fractional_repeat() const { return texture_fractional_repeat_; };
        void set_texture_fractional_repeat(float fr) { texture_fractional_repeat_ = fr; };

        /**
         * Highlight a subset of primitives of this drawable. Primitives with indices within the range
         * [highlight_id_low_, highlight_id_high_] will be highlighted.
         * @param range Specifies the min and max indices of the primitives to be highlighted. Providing [-1, -1] will
         *              un-highlight any previously highlighted primitives.
         * @attention For non-triangular surface meshes, all polygonal faces are internally triangulated to allow a
         *            unified rendering APIs. Thus for performance reasons, the selection of polygonal faces is also
         *            internally implemented by selecting triangle primitives using program shaders. This allows data
         *            uploaded to the GPU for the rendering purpose be shared for selection. Yeah, performance gain!
         */
        bool highlight() const { return highlight_; };
        void set_highlight(bool b) { highlight_ = b; }

        void set_highlight_range(const std::pair<int, int> &range) { highlight_range_ = range; }
        const std::pair<int, int> &highlight_range() const { return highlight_range_; }

        /// Rendering
        virtual void draw(const Camera *camera, bool with_storage_buffer = false) const = 0;

        /// The internal draw of this drawable.
        /// NOTE: this functions should be called when your shader program is in use,
        ///		 i.e., between glUseProgram(id) and glUseProgram(0);
        void gl_draw(bool with_storage_buffer = false) const;

    protected:
        void clear();

        VertexArrayObject *vao() const { return vao_; }

    protected:
        std::string name_;
        Model *model_;

        Box3 bbox_;
        bool visible_;

        ColorScheme color_scheme_;

        bool per_vertex_color_;
        vec4 default_color_;

        bool lighting_;
        bool lighting_two_sides_;
        bool distinct_back_color_;
        vec4 back_color_;

        // highlight the primitives within the range [highlight_id_low_, highlight_id_high_]
        bool highlight_;
        std::pair<int, int> highlight_range_;

        Material material_;

        // texture
        bool use_texture_;
        Texture *texture_;
        // How many times do you want to repeat the texture?
        float texture_repeat_;
        // Control at a finer level: 100 fractional repeat == repeat.
        float texture_fractional_repeat_;

        // vertex array object
        VertexArrayObject *vao_;

        std::size_t num_vertices_;
        std::size_t num_indices_;

        bool modified_;
        std::function<void(Model*, Drawable*)> update_func_;

        unsigned int vertex_buffer_;
        unsigned int color_buffer_;
        unsigned int normal_buffer_;
        unsigned int texcoord_buffer_;
        unsigned int index_buffer_;

        unsigned int storage_buffer_;
        std::size_t current_storage_buffer_size_;

        unsigned int selection_buffer_;  // used for selection.
        std::size_t current_selection_buffer_size_; // in case the object is modified
    };

}


#endif  // EASY3D_DRAWABLE_H
