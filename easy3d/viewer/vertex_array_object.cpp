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


#include <easy3d/viewer/vertex_array_object.h>

#include <cassert>

#include <easy3d/viewer/opengl_error.h>
#include <easy3d/viewer/opengl_info.h>


namespace easy3d {


    bool VertexArrayObject::is_supported() {
        return OpenglInfo::is_supported("GL_VERSION_2_1") ||
            OpenglInfo::is_supported("GL_ARB_vertex_array_object") ||
            OpenglInfo::has_entension("GL_EXT_vertex_array_object");
    }


	VertexArrayObject::VertexArrayObject()
		: id_(0)
	{
        if (!is_supported()) {
            std::cerr << "vertex array object not supported on this platform" << std::endl;
            return;
        }

        // Liangliang: it is a bad idea to initialize OpenGL stuff in the constructors
        //			   because the OpenGL context may not exist. In Easy3D, I alway
        //             follow the "create when needed" rule.
        // glGenVertexArrays(1, &id_);	easy3d_debug_gl_error;
        // if (id_ == 0) {
        //     std::cerr << "generating VertexArrayObject failed" << std::endl;
		//}
	}


	VertexArrayObject::~VertexArrayObject() {
		if (id_ != 0) {
			glDeleteVertexArrays(1, &id_);	easy3d_debug_gl_error;
			id_ = 0;
		}
	}


	void VertexArrayObject::bind() {
		if (id_ == 0) {
			glGenVertexArrays(1, &id_);	easy3d_debug_gl_error;
			if (id_ == 0) {
				std::cerr << "generating VertexArrayObject failed" << std::endl;
				return;
			}
		}

		glBindVertexArray(id_);		easy3d_debug_gl_error;
	}


    void VertexArrayObject::release() {
		assert(id_ != 0);
		glBindVertexArray(0);		easy3d_debug_gl_error;
	}


	void VertexArrayObject::release_buffer(GLuint& handle) {
		if (handle != 0) {
			glBindVertexArray(0);			easy3d_debug_gl_error;
			glDeleteBuffers(1, &handle);	easy3d_debug_gl_error;
			handle = 0;
		}
	}


	GLint VertexArrayObject::create_array_buffer(GLuint& handle, GLuint index, const void* data, std::size_t datasize, GLenum data_type, std::size_t vectordim) {
		release_buffer(handle);
		bind();

		glGenBuffers(1, &handle);			easy3d_debug_gl_error;
		glBindBuffer(GL_ARRAY_BUFFER, handle);			easy3d_debug_gl_error;
		glBufferData(GL_ARRAY_BUFFER, datasize, data, GL_STATIC_DRAW);		easy3d_debug_gl_error;
		glEnableVertexAttribArray(index);
        glVertexAttribPointer(index, int(vectordim), data_type, GL_FALSE, 0, nullptr);		easy3d_debug_gl_error;
		GLint err = glGetError();
		if (err != GL_NO_ERROR) {
			glBindBuffer(GL_ARRAY_BUFFER, 0);		easy3d_debug_gl_error;
			glDeleteBuffers(1, &handle);			easy3d_debug_gl_error;
			handle = 0;
		}
		glBindBuffer(GL_ARRAY_BUFFER, 0);			easy3d_debug_gl_error;

        release();

		return err;
	}


	GLint VertexArrayObject::create_index_buffer(GLuint& handle, const void* data, std::size_t datasize) {
		release_buffer(handle);
		bind();

		glGenBuffers(1, &handle);					easy3d_debug_gl_error;
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);		easy3d_debug_gl_error;
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, datasize, data, GL_STATIC_DRAW);		easy3d_debug_gl_error;
		GLint err = glGetError();	easy3d_debug_gl_error;
		if (err != GL_NO_ERROR) {
			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);		easy3d_debug_gl_error;
			glDeleteBuffers(1, &handle);		easy3d_debug_gl_error;
			handle = 0;
		}

		// Otherwise, you have to bind the buffer before drawing
        //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);	easy3d_debug_gl_error;

        release();

		return err;
	}


	GLint VertexArrayObject::create_storage_buffer(GLuint& handle, GLuint index, const void* data, std::size_t datasize) {
        if (!OpenglInfo::is_supported("GL_ARB_shader_storage_buffer_object")) {
            std::cerr << "shader storage buffer object not supported on this platform" << std::endl;
			return -1;
        }

		release_buffer(handle);
		bind();

		glGenBuffers(1, &handle);				easy3d_debug_gl_error;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);			easy3d_debug_gl_error;
		glBufferData(GL_SHADER_STORAGE_BUFFER, datasize, data, GL_DYNAMIC_DRAW);		easy3d_debug_gl_error;
		GLint err = glGetError();					easy3d_debug_gl_error;
		glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, handle);		easy3d_debug_gl_error;
		if (err != GL_NO_ERROR) {
			glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);			easy3d_debug_gl_error;
			glDeleteBuffers(1, &handle);			easy3d_debug_gl_error;
			handle = 0;
		}
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);		easy3d_debug_gl_error;
		easy3d_debug_gl_error;

        release();

		return err;
	}


	GLint VertexArrayObject::update_storage_buffer(GLuint& handle, GLintptr offset, GLsizeiptr size, const void* data) {
		bind();

		glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);		easy3d_debug_gl_error;
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);		easy3d_debug_gl_error;
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);		easy3d_debug_gl_error;

        release();

		return glGetError();	easy3d_debug_gl_error;
	}


	void* VertexArrayObject::map_buffer(GLenum target, GLuint handle, GLenum access) {
		// Liangliang: should work, but haven't tested yet.
		glBindBuffer(target, handle);	easy3d_debug_gl_error;

		// Liangliang: Finally, I found this is the place to call glMemoryBarrier()
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);	easy3d_debug_gl_error;

		void* ptr = glMapBuffer(handle, access);	easy3d_debug_gl_error;
		glBindBuffer(target, 0);	easy3d_debug_gl_error;
		return ptr;
	}


	void VertexArrayObject::unmap_buffer(GLenum target, GLuint handle) {
		// Liangliang: should work, but haven't tested yet.
		glBindBuffer(target, handle);	easy3d_debug_gl_error;
		glUnmapBuffer(handle);	easy3d_debug_gl_error;
		glBindBuffer(target, 0);	easy3d_debug_gl_error;
	}


	void* VertexArrayObject::map_named_buffer(GLuint handle, GLenum access) {
		// Liangliang: Finally, I found this is the place to call glMemoryBarrier()
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);	easy3d_debug_gl_error;

		return glMapNamedBuffer(handle, access);
	}


	void VertexArrayObject::unmap_named_buffer(GLuint handle) {
		glUnmapNamedBuffer(handle);	easy3d_debug_gl_error;
	}


	void VertexArrayObject::get_buffer_data(GLenum target, GLuint handle, GLintptr offset, GLsizeiptr size, void* data) {
		glBindBuffer(target, handle);	easy3d_debug_gl_error;

		// Liangliang: Finally, I found this is the place to call glMemoryBarrier()
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);	easy3d_debug_gl_error;

		glGetBufferSubData(target, offset, size, data);	easy3d_debug_gl_error;
		glBindBuffer(target, 0);	easy3d_debug_gl_error;
	}


	void VertexArrayObject::get_named_buffer_data(GLuint handle, GLintptr offset, GLsizeiptr size, void* data) {
		// Liangliang: Finally, I found this is the place to call glMemoryBarrier()
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);	easy3d_debug_gl_error;

		glGetNamedBufferSubData(handle, offset, size, data);	easy3d_debug_gl_error;
	}

}
