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

#ifndef EASY3D_DRAWABLE_H
#define EASY3D_DRAWABLE_H

#include <string>
#include <vector>

#include <easy3d/core/types.h>

/********************************************************************************************
*
* Drawable is the base class for drawable objects (i.e., points, line segments, triangles).
* A drawable manages the rendering of an objects. When data (vertex position/color/normal,
* selection, etc.) changes, it controls the upload of the data to GPU.
*
*********************************************************************************************/

namespace easy3d {

    class Model;
	class VertexArrayObject;

	// representation models
	enum DrawableType {
		DT_POINTS = 0x0000,		// same as GL_POINTS
		DT_LINES = 0x0001,		// same as GL_LINES
		DT_TRIANGLES = 0x0004	// same as GL_TRIANGLES
	};

	class Drawable
	{
	public:
        // a drawable can be stand-alone or attached to a model
        Drawable(const std::string& name = "unknown", const Model* model = nullptr);
		virtual ~Drawable();

		// accept  values are: GL_POINTS, GL_LINES, GL_TRIANGLES
		virtual DrawableType type() const = 0;

		const std::string name() const { return name_; }
		void set_name(const std::string n) { name_ = n; }

        // the model to which the drawable is attached to
        const Model* model() const { return model_; }
        void set_model(const Model* m) { model_ = m; }

		bool is_visible() const { return visible_; }
		void set_visible(bool v) { visible_ = v; }

        bool per_vertex_color() const { return per_vertex_color_;}
		void set_per_vertex_color(bool b) { per_vertex_color_ = b; }

		// default_color will be ignored if per_vertex_color is true and given.
		const vec3& default_color() const { return default_color_; }
		void set_default_color(const vec3& c) { default_color_ = c; }

		VertexArrayObject* vao() { return vao_; }

		// ---------------------- buffer access ------------------------------

		unsigned int vertex_buffer() const { return vertex_buffer_; }
		unsigned int color_buffer() const { return color_buffer_; }
		unsigned int normal_buffer() const { return normal_buffer_; }
		unsigned int texcoord_buffer() const { return texcoord_buffer_; }
		unsigned int index_buffer() const { return index_buffer_; }
		unsigned int storage_buffer() const { return storage_buffer_; }
		unsigned int selection_buffer() const { return selection_buffer_; }

		void release_vertex_buffer();
		void release_color_buffer();
		void release_normal_buffer();
		void release_texcoord_buffer();
		void release_index_buffer();
		void release_storage_buffer();
		void release_selection_buffer();

		// ---------------------- buffer creation -----------------------------

		// dim: number of float components of a point, 3 by default
		void update_vertex_buffer(const float* vertices, std::size_t count, int dim);
		void update_vertex_buffer(const std::vector<vec3>& vertices);
	
		// dim: number of float components of a color, 3 by default
		void update_color_buffer(const float* colors, std::size_t count, int dim);
		void update_color_buffer(const std::vector<vec3>& colors);

		// dim: number of float components of a normal, 3 by default
		void update_normal_buffer(const float* normals, std::size_t count, int dim);
		void update_normal_buffer(const std::vector<vec3>& normals);

		// dim: number of float components of a texture coordinate, 2 by default
		void update_texcoord_buffer(const float* texcoords, std::size_t count, int dim);
		void update_texcoord_buffer(const std::vector<vec2>& texcoords);

		void update_index_buffer(const std::vector<unsigned int>& indices);
		void update_index_buffer(const unsigned int* indices, std::size_t count);

		// selection buffer (internally based on a shader storage buffer)
		// @param index: the index of the binding point.
		// NOTE: the buffers should also be bound to this point in all shader code
		void update_selection_buffer(unsigned int index = 1);
		// generic storage buffer
		// @param index: the index of the binding point.
		// NOTE: the buffers should also be bound to this point in all shader code
		void update_storage_buffer(const void* data, std::size_t datasize, unsigned int index = 0);

		// ---------------------- get data from GPU -------------------------

		void fetch_selection_buffer();

		// -------------------------- rendering -----------------------------

		// Draw this drawable.
		// NOTE: this functions should be called when your shader program is in use, 
		//		 i.e., between glUseProgram(id) and glUseProgram(0);	
		void draw(bool with_storage_buffer = false) const;

		void set_highlight_id(int id) { highlight_id_ = id; }
		int  highlight_id() const { return highlight_id_; }

	protected:
		std::string	name_;
        const Model* model_;

		bool		visible_;
		bool		per_vertex_color_;
		vec3		default_color_;

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

		int	highlight_id_;
	};


    // The drawable for rendering a set of points, e.g., point clouds, vertices of a mesh
	class PointsDrawable : public Drawable {
	public:
        PointsDrawable(const std::string& name = "")
			: Drawable(name) 
			, point_size_(2.0f)
            , impostors_(false)
		{
			default_color_ = vec3(0.0f, 1.0f, 0.0f);
		}

        DrawableType type() const override ;

        // default_color will be ignored if per_vertex_color is true and given.
		float point_size() const { return point_size_; }
		void set_point_size(float s) { point_size_ = s; }

        bool impostors() const { return impostors_; }
        void set_impostors(bool b) { impostors_ = b; }

	private:
		float point_size_;
        bool  impostors_;
	};


    // The drawable for rendering a set of line segments, e.g., wireframe of a mesh, vector fields
	class LinesDrawable : public Drawable {
	public:
        LinesDrawable(const std::string& name = "")
            : Drawable(name)
            , impostors_(false)
            , impostor_thickness_(1.0f)
        {
			default_color_ = vec3(0.0f, 0.0f, 0.0f);
		}
        DrawableType type() const override;

        bool impostors() const { return impostors_; }
        void set_impostors(bool b) { impostors_ = b; }

        float impostor_thickness() const { return impostor_thickness_; }
        void set_impostor_thickness(float t) { impostor_thickness_ = t; }

    private:
        bool  impostors_;
        float impostor_thickness_;
    };


    // The drawable for rendering a set of triangles, e.g., the surface of a triangular mesh.
    // NOTE: it surports triangles only. To visualize general polygons, the vertex coordinates
    //       and properties (e.g., color, normal) should be provided as consequtive triplets
    //       in an array to be transfered to GPU. See update_vertex_buffer().
    class TrianglesDrawable : public Drawable {
	public:
        TrianglesDrawable(const std::string& name = "")
            : Drawable(name)
            , phong_shading_(false)
            , opacity_(0.6f)
        {
			default_color_ = vec3(0.4f, 0.8f, 0.8f);
		}
        DrawableType type() const override;

        bool phong_shading() const { return phong_shading_; }
        void set_phong_shading(bool b) { phong_shading_ = b; }

        float opacity() const { return opacity_; }
        void set_opacity(float a) { opacity_ = a; }

		// The selection of a polygonal face is internally implemented by selecting triangle
		// primitives using shaders. So I need a way to map back to the original polygons.
		// \param indices: indices[i] are the triangle indices of the i'th face.
		void set_triangle_indices(const std::vector< std::vector<unsigned int> >& indices);
		const std::vector< std::vector<unsigned int> >& triangle_indices() { return indices_; }

		// a face (i.e., polygon) is internally rendered as multiple triangles
		void get_highlighted_trangles_range(int& tri_min, int& tri_max) const;

		// set/query if a facet is selected.
		// NOTE: a face is selected if all its vertices are selected.
		inline void set_selected(std::size_t face_idx, bool b);
		inline bool is_selected(std::size_t face_idx) const;

		int  num_selected() const;

	private:
		std::vector< std::vector<unsigned int> > indices_;

        bool    phong_shading_;
        float   opacity_;
	};

}


#endif  // EASY3D_DRAWABLE_H
