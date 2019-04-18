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


#include <easy3d/viewer/drawable.h>

#include <cassert>

#include <easy3d/viewer/opengl.h>
#include <easy3d/viewer/vertex_array_object.h>
#include <easy3d/viewer/shader_program.h>
#include <easy3d/viewer/opengl_error.h>



namespace easy3d {


    Drawable::Drawable(const std::string& name, const Model* model)
        : name_(name)
        , model_(model)
        , visible_(true)
        , per_vertex_color_(false)
        , default_color_(1.0f, 0.0f, 0.0f)
        , num_vertices_(0)
		, num_indices_(0)
		, vertex_buffer_(0)
        , color_buffer_(0)
        , normal_buffer_(0)
		, texcoord_buffer_(0)
        , index_buffer_(0)
        , storage_buffer_(0)
		, current_storage_buffer_size_(0)
        , selection_buffer_(0)
        , current_selection_buffer_size_(0)
        , highlight_id_(-1)
    {
		vao_ = new VertexArrayObject;
	}


	Drawable::~Drawable() {
		if (vao_) {
			delete vao_;
			vao_ = nullptr;
		}

		release_vertex_buffer();
		release_color_buffer();
		release_normal_buffer();
		release_texcoord_buffer();
		release_index_buffer();
		release_storage_buffer();
		release_selection_buffer();
	}


	void Drawable::release_vertex_buffer() {
		if (vertex_buffer_ != 0)
			VertexArrayObject::release_buffer(vertex_buffer_);
	}


	void Drawable::release_color_buffer() {
		if (color_buffer_ != 0)
			VertexArrayObject::release_buffer(color_buffer_);
	}


	void Drawable::release_normal_buffer() {
		if (normal_buffer_ != 0)
			VertexArrayObject::release_buffer(normal_buffer_);
	}


	void Drawable::release_texcoord_buffer() {
		if (texcoord_buffer_ != 0)
			VertexArrayObject::release_buffer(texcoord_buffer_);
	}


	void Drawable::release_index_buffer() {
		if (index_buffer_ != 0)
			VertexArrayObject::release_buffer(index_buffer_);
	}


	void Drawable::release_storage_buffer() {
		if (storage_buffer_ != 0)
			VertexArrayObject::release_buffer(storage_buffer_);
	}


	void Drawable::release_selection_buffer() {
		if (selection_buffer_ != 0)
			VertexArrayObject::release_buffer(selection_buffer_);
	}


	void Drawable::update_vertex_buffer(const float* vertices, std::size_t count, int dim) {
		assert(vao_);

		int status = vao_->create_array_buffer(vertex_buffer_, ShaderProgram::POSITION, vertices, count * sizeof(float) * dim, GL_FLOAT, dim);
		if (status != GL_NO_ERROR) {
			num_vertices_ = 0;
			std::cerr << "failed creating vertex buffer" << std::endl;
		}
		else {
			num_vertices_ = count;
		}
	}


	void Drawable::update_color_buffer(const float* colors, std::size_t count, int dim) {
		assert(vao_);

		int status = vao_->create_array_buffer(color_buffer_, ShaderProgram::COLOR, colors, count * sizeof(float) * dim, GL_FLOAT, dim);
		if (status != GL_NO_ERROR)
			std::cerr << "failed updating color buffer" << std::endl;
	}


	void Drawable::update_normal_buffer(const float* normals, std::size_t count, int dim) {
		assert(vao_);
		int status = vao_->create_array_buffer(normal_buffer_, ShaderProgram::NORMAL, normals, count * sizeof(float) * dim, GL_FLOAT, dim);
		if (status != GL_NO_ERROR)
			std::cerr << "failed updating normal buffer" << std::endl;
	}


	void Drawable::update_texcoord_buffer(const float* texcoords, std::size_t count, int dim) {
		assert(vao_);

		int status = vao_->create_array_buffer(texcoord_buffer_, ShaderProgram::TEXCOORD, texcoords, count * sizeof(float) * dim, GL_FLOAT, dim);
		if (status != GL_NO_ERROR)
			std::cerr << "failed updating texcoord buffer" << std::endl;
	}


	void Drawable::update_index_buffer(const unsigned int* indices, std::size_t count) {
		assert(vao_);

		int status = vao_->create_index_buffer(index_buffer_, indices, count * sizeof(unsigned int));
		if (status != GL_NO_ERROR) {
			num_indices_ = 0;
		}
		else {
			num_indices_ = count;
		}
	}


	void Drawable::update_storage_buffer(const void* data, std::size_t datasize, unsigned int index /* = 1*/) {
		assert(vao_);

		if (storage_buffer_ == 0 || datasize != current_storage_buffer_size_) {
			int status = vao_->create_storage_buffer(storage_buffer_, index, data, datasize);
			if (status != GL_NO_ERROR)
				std::cerr << "failed creating storage buffer" << std::endl;
		}
		else {
			int status = vao_->update_storage_buffer(storage_buffer_, 0, datasize, data);
			if (status != GL_NO_ERROR)
				std::cerr << "failed updating storage buffer" << std::endl;
		}
	}


	void Drawable::update_selection_buffer(unsigned int index/* = 1*/) {
		assert(vao_);

        //if (selection_buffer_ == 0 || selections_.size() != current_selection_buffer_size_) {
	 //       int status = vao_->create_storage_buffer(selection_buffer_, index, selections_.data(), selections_.size() * sizeof(int));
		//	if (status != GL_NO_ERROR)
		//		std::cerr << "failed creating selection buffer" << std::endl;
		//	else {
		//		current_selection_buffer_size_ = selections_.size();
		//		if (t.elapsed_seconds() > 0.1) {
		//			std::cout << "selection buffer updated. time: " << t.time_string() << std::endl;
		//		}
		//	}
		//}
		//else {
	 //       int status = vao_->update_storage_buffer(selection_buffer_, 0, selections_.size() * sizeof(int), selections_.data());
		//	if (status != GL_NO_ERROR)
		//		std::cerr << "failed updating selection buffer" << std::endl;
		//	else {
		//		if (t.elapsed_seconds() > 0.1) {
		//			std::cout << "selection buffer updated. time: " << t.time_string() << std::endl;
		//		}
		//	}
		//}
	}


	void Drawable::update_vertex_buffer(const std::vector<vec3>& vertices) {
		update_vertex_buffer((const float*)vertices.data(), vertices.size(), 3);
	}


	void Drawable::update_color_buffer(const std::vector<vec3>& colors) {
		update_color_buffer((const float*)colors.data(), colors.size(), 3);
	}


	void Drawable::update_normal_buffer(const std::vector<vec3>& normals) {
		update_normal_buffer((const float*)normals.data(), normals.size(), 3);
	}


	void Drawable::update_texcoord_buffer(const std::vector<vec2>& texcoords) {
		update_texcoord_buffer((const float*)texcoords.data(), texcoords.size(), 2);
	}


	void Drawable::update_index_buffer(const std::vector<unsigned int>& indices) {
		update_index_buffer((const unsigned int*)indices.data(), indices.size());
	}


	void Drawable::fetch_selection_buffer() {
		//    vao_->get_buffer_data(GL_SHADER_STORAGE_BUFFER, selection_buffer_, 0, selections_.size() * sizeof(uint32_t), selections_.data());

			//starting from OpenGL 4.5, you can use the simpler version
			//vao_->get_named_buffer_data(selection_buffer_, 0, selections_.size() * sizeof(uint32_t), selections_.data());
	}


	void Drawable::draw(bool with_storage_buffer /* = false */) const
	{
		vao_->bind();

		if (type() == DT_POINTS)
            glPointSize(dynamic_cast<const PointsDrawable*>(this)->point_size());	easy3d_debug_gl_error;

		if (with_storage_buffer) {
            // Liangliang: I made stupid mistake here (confused by glBindBuffer() and glBindBufferBase())
			//glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssb);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, selection_buffer_);		easy3d_debug_gl_error;

            GLbitfield barriers = GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT | GL_SHADER_STORAGE_BARRIER_BIT;
            if (index_buffer_ != 0)
                barriers |= GL_ELEMENT_ARRAY_BARRIER_BIT;

            glMemoryBarrier(barriers);		easy3d_debug_gl_error;
		}

		// Primitives like lines and triangles can be drawn without the index buffer provided that 
		// all vertices are in order (e.g., f1_v1, f1_v2, f1_v3, f2_v1, f2_v2, f2_v2). This requires
		// the shared vertices be duplicated in the vertex buffer.
		if (index_buffer_) {
			// index buffer must be bound if using glDrawElements()
            glDrawElements(type(), GLsizei(num_indices_), GL_UNSIGNED_INT, nullptr);	easy3d_debug_gl_error;
		}
		else
            glDrawArrays(type(), 0, GLsizei(num_vertices_));	easy3d_debug_gl_error;

		if (with_storage_buffer) {
			// Liangliang: I made stupid mistake here (confused by glBindBuffer() and glBindBufferBase())
			//glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
            glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, 0);		easy3d_debug_gl_error;
		}

        vao_->release();	easy3d_debug_gl_error;
	}


    DrawableType PointsDrawable::type() const {
        return DT_POINTS;
    }


    DrawableType LinesDrawable::type() const {
        return DT_LINES;
    }


    DrawableType TrianglesDrawable::type() const {
        return DT_TRIANGLES;
    }


	void TrianglesDrawable::set_triangle_indices(const std::vector< std::vector<unsigned int> >& indices) {
		indices_ = indices;
		std::size_t count = 0;
		for (std::size_t i = 0; i < indices_.size(); ++i) {
			count += indices_[i].size();
		}
		//Selectable::set_num_primitives(count);
	}


	void TrianglesDrawable::set_selected(std::size_t face_idx, bool b) {
		// 	const std::vector<unsigned int>& indices = indices_[face_idx];
		// 	for (std::size_t i = 0; i < indices.size(); ++i) {
		// 		unsigned int id = indices[i];
		// 		Selectable::set_selected(id, b);
		// 	}
	}


	bool TrianglesDrawable::is_selected(std::size_t face_idx) const {
		// 	const std::vector<unsigned int>& indices = indices_[face_idx];
		// 	for (std::size_t i = 0; i < indices.size(); ++i) {
		// 		unsigned int id = indices[i];
		// 		if (Selectable::is_selected(id))
		// 			return false;
		// 	}
		return true;
	}


	int TrianglesDrawable::num_selected() const {
		int count = 0;
		for (std::size_t i = 0; i < indices_.size(); ++i) {
			if (is_selected(i))
				++count;
		}
		return count;
	}


	void TrianglesDrawable::get_highlighted_trangles_range(int& tri_min, int& tri_max) const {
		if (highlight_id_ >= 0 && highlight_id_ < indices_.size()) {
			tri_min = indices_[highlight_id_].front();
			tri_max = indices_[highlight_id_].back();
		}
		else {
			tri_min = -1;
			tri_max = -1;
		}
	}


}
