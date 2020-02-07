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


	// representation models
	enum DrawableType {
		DT_POINTS = 0x0000,		// same as GL_POINTS
		DT_LINES = 0x0001,		// same as GL_LINES
		DT_TRIANGLES = 0x0004	// same as GL_TRIANGLES
	};


    struct Material {
        Material();
        Material(const vec3& ambi, const vec3& spec, float shin);
        vec3    ambient;
        //vec3    diffuse; // I may have per face/point/line color!
        vec3	specular;
        float	shininess;	// specular power
    };


    class Model;
    class Camera;
    class Texture;
    class VertexArrayObject;

	class Drawable
	{
	public:
        // a drawable can be stand-alone or attached to a model
        Drawable(const std::string& name = "unknown", const Model* model = nullptr);
		virtual ~Drawable();

		// accept  values are: GL_POINTS, GL_LINES, GL_TRIANGLES
		virtual DrawableType type() const = 0;

        const std::string& name() const { return name_; }
		void set_name(const std::string n) { name_ = n; }

        // the model to which the drawable is attached to (can be NULL).
        const Model* model() const { return model_; }
        void set_model(const Model* m) { model_ = m; }

        const Box3& bounding_box() const { return bbox_; }

        // ---------------------- buffer access ------------------------------

		unsigned int vertex_buffer() const { return vertex_buffer_; }
		unsigned int color_buffer() const { return color_buffer_; }
		unsigned int normal_buffer() const { return normal_buffer_; }
		unsigned int texcoord_buffer() const { return texcoord_buffer_; }
        unsigned int index_buffer() const { return index_buffer_; }
		unsigned int storage_buffer() const { return storage_buffer_; }
		unsigned int selection_buffer() const { return selection_buffer_; }

        // ------------------- buffer creation/update ------------------------

		void update_vertex_buffer(const std::vector<vec3>& vertices);
		void update_color_buffer(const std::vector<vec3>& colors);
		void update_normal_buffer(const std::vector<vec3>& normals);
		void update_texcoord_buffer(const std::vector<vec2>& texcoords);
		void update_index_buffer(const std::vector<unsigned int>& indices);

		// selection buffer (internally based on a shader storage buffer)
		// @param index: the index of the binding point.
		// NOTE: the buffers should also be bound to this point in all shader code
		void update_selection_buffer(unsigned int index = 1);
		// generic storage buffer
		// @param index: the index of the binding point.
		// NOTE: the buffers should also be bound to this point in all shader code
		void update_storage_buffer(const void* data, std::size_t datasize, unsigned int index = 0);

        void release_element_buffer();

        // print statistics, e.g., memory usage
        void drawable_stats() const;

        // ---------------------- get data from GPU -------------------------

        void fetch_selection_buffer();

        // -------------------------- rendering -----------------------------

		bool is_visible() const { return visible_; }
		void set_visible(bool v) { visible_ = v; }

		// if true, one of the color properties (e.g., "v:color", "f:color", or "e:color") will be active for rendering.
		bool per_vertex_color() const { return per_vertex_color_; }
		void set_per_vertex_color(bool b) { per_vertex_color_ = b; }

        // default_color will be ignored if per_vertex_color is true and given.
		const vec4 &default_color() const { return default_color_; }
		void set_default_color(const vec4 &c) { default_color_ = c; }

        bool lighting() const { return lighting_; }
        void set_lighting(bool l) { lighting_ = l; }

        bool lighting_two_sides() const { return lighting_two_sides_; }
        void set_lighting_two_sides(bool b) { lighting_two_sides_ = b; }

        // use a different color for the back side, valid only if two-side lighting is enabled
        bool distinct_back_color() const { return distinct_back_color_; }
        void set_distinct_back_color(bool b) { distinct_back_color_ = b; }

        // the back side color, valid only if two-side lighting is enabled and distinct back color is true
        const vec4 &back_color() const { return back_color_; }
        void set_back_color(const vec4 &c) { back_color_ = c; }

        Material &material() { return material_; }
        const Material &material() const { return material_; }
        void set_material(const Material &m) { material_ = m; }

		bool use_texture() const { return use_texture_; }
		void set_use_texture(bool b) { use_texture_ = b; }

        Texture *texture() const { return texture_; }
        void set_texture(Texture *tex) { texture_ = tex; }

		// How many time do you want to repeat the texture?
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
        void set_highlight_range(const std::pair<int, int>& range) { highlight_range_ = range; }
        const std::pair<int, int>& highlight_range() const { return highlight_range_; }

        // Rendering
        virtual void draw(const Camera *camera, bool with_storage_buffer = false) const = 0;

        // The internal draw of this drawable.
        // NOTE: this functions should be called when your shader program is in use,
        //		 i.e., between glUseProgram(id) and glUseProgram(0);
        void gl_draw(bool with_storage_buffer = false) const;

    protected:
        void clear();

        VertexArrayObject* vao() const { return vao_; }

	protected:
        std::string	 name_;
        const Model* model_;

        Box3 bbox_;

        bool visible_;
        bool per_vertex_color_;
        vec4 default_color_;

        bool lighting_;
        bool lighting_two_sides_;
        bool distinct_back_color_;
        vec4 back_color_;

        // highlight the primitives within the range [highlight_id_low_, highlight_id_high_]
        bool highlight_;
        std::pair<int, int> highlight_range_;

        Material    material_;

        // texture
        bool 		use_texture_;
        Texture*    texture_;
		// How many times do you want to repeat the texture?
		float texture_repeat_;
		// Control at a finer level: 100 fractional repeat == repeat.
		float texture_fractional_repeat_;

        // vertex array object
		VertexArrayObject*	vao_;

		std::size_t	 num_vertices_;
		std::size_t  num_indices_;

		unsigned int vertex_buffer_;
		unsigned int color_buffer_;
		unsigned int normal_buffer_;
		unsigned int texcoord_buffer_;
		unsigned int index_buffer_;

		unsigned int storage_buffer_;
		std::size_t	 current_storage_buffer_size_;

		unsigned int selection_buffer_;  // used for selection.
		std::size_t	 current_selection_buffer_size_; // in case the object is modified
	};

}


#endif  // EASY3D_DRAWABLE_H
