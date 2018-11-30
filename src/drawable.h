/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*
*	This file is part of easy3d: software for processing and rendering
*   meshes and point clouds.
*
*	easy3d is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	easy3d is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _MPL_OPENGL_DRAWABLE_H_
#define _MPL_OPENGL_DRAWABLE_H_


#include <model/math_types.h>

#include <string>
#include <vector>

/********************************************************************************************
*
* Drawable is the base class for drawable objects (e.g., surface, wireframe, vertices, etc.)
* that are capable of managing their rendering.
* When data (vertex position/color/normal, selection, etc.) changes, it controls the upload
* of these data to GPU.
*
*	Version:	 1.0
*	created:	 Dec. 14, 2015
*	author:		 Liangliang Nan
*	contact:     liangliang.nan@gmail.com
*
*********************************************************************************************/

namespace easy3d {


	class VertexArrayObject;

	// representation models
	enum DrawableType {
		DT_FACES = 0,	// a set of faces
		DT_LINES,		// a set of line segments
		DT_POINTS		// a set of points
	};

	class Drawable
	{
	public:
		Drawable();
		virtual ~Drawable();

		static std::string title() { return "Drawable"; }

		virtual DrawableType type() const = 0;

		const std::string name() const { return name_; }
		void set_name(const std::string n) { name_ = n; }

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
		void draw(bool with_storage_buffer = true) const;

		void set_highlight_id(int id) { highlight_id_ = id; }
		int  highlight_id() const { return highlight_id_; }

	protected:
		Box3   bounding_box_; // cached bounding box

		// vertex array object
		VertexArrayObject*	vao_;

		std::string			name_;

		std::size_t	 num_vertices_;
		std::size_t  num_indices_;	// draw_as_polygons() needs this

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


	class FacesDrawable : public Drawable {
	public:
		DrawableType type() const { return DT_FACES; }

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
	};


	class LinesDrawable : public Drawable {
	public:
		DrawableType type() const { return DT_LINES; }
	};


	class PointsDrawable : public Drawable {
	public:
		DrawableType type() const { return DT_POINTS; }
	};


}



#endif