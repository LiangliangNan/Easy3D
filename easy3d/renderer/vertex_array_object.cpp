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

#include <easy3d/renderer/vertex_array_object.h>

#include <cassert>

#include <easy3d/renderer/opengl_error.h>
#include <easy3d/renderer/opengl_util.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    bool VertexArrayObject::is_supported() {
        return OpenglUtil::is_supported("GL_VERSION_2_1") ||
            OpenglUtil::is_supported("GL_ARB_vertex_array_object") ||
            OpenglUtil::has_extension("GL_EXT_vertex_array_object");
    }


	VertexArrayObject::VertexArrayObject()
		: id_(0)
	{
        if (!is_supported()) {
            LOG(ERROR) << "vertex array object not supported on this platform";
            return;
        }

        // Liangliang: it is a bad idea to initialize OpenGL stuff in the constructors
        //			   because the OpenGL context may not exist. In Easy3D, I always
        //             follow the "create when needed" rule.
        // glGenVertexArrays(1, &id_);	easy3d_debug_log_gl_error
        // if (id_ == 0) {
        //     LOG(ERROR) << "generating VertexArrayObject failed";
		//}
	}


	VertexArrayObject::~VertexArrayObject() {
		if (id_ != 0) {
			glDeleteVertexArrays(1, &id_);	easy3d_debug_log_gl_error
			id_ = 0;
		}
	}


	void VertexArrayObject::bind() {
		if (id_ == 0) {
			glGenVertexArrays(1, &id_);	easy3d_debug_log_gl_error
			if (id_ == 0) {
				LOG(ERROR) << "failed generating VertexArrayObject";
				return;
			}
		}

		glBindVertexArray(id_);		easy3d_debug_log_gl_error
	}


    void VertexArrayObject::release() const {
		assert(id_ != 0);
		glBindVertexArray(0);		easy3d_debug_log_gl_error
	}


    void VertexArrayObject::release_buffer(GLuint& buffer) {
        if (buffer != 0) {
			glBindVertexArray(0);			easy3d_debug_log_gl_error
            glDeleteBuffers(1, &buffer);	easy3d_debug_log_gl_error
            buffer = 0;
		}
	}


    bool VertexArrayObject::create_array_buffer(GLuint& buffer, GLuint index, const void* data, std::size_t size, std::size_t dim, bool dynamic) {
        release_buffer(buffer);
		bind();
        glGenBuffers(1, &buffer);                       easy3d_debug_log_gl_error
        LOG_IF(buffer == 0, ERROR) << "failed creating array buffer";
        glBindBuffer(GL_ARRAY_BUFFER, buffer);			easy3d_debug_log_gl_error
        glBufferData(GL_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);		easy3d_debug_log_gl_error
        glEnableVertexAttribArray(index);               easy3d_debug_log_gl_error
        glVertexAttribPointer(index, int(dim), GL_FLOAT, GL_FALSE, 0, nullptr);		easy3d_debug_log_gl_error
        if (glGetError() != GL_NO_ERROR) {
            glBindBuffer(GL_ARRAY_BUFFER, 0);           easy3d_debug_log_gl_error
            glDeleteBuffers(1, &buffer);                easy3d_debug_log_gl_error
            buffer = 0;
            LOG(ERROR) << "failed creating array buffer";
		}
        glBindBuffer(GL_ARRAY_BUFFER, 0);               easy3d_debug_log_gl_error
        release();
        return (glGetError() == GL_NO_ERROR && buffer != 0);
    }


    bool VertexArrayObject::create_element_buffer(GLuint &buffer, const void *data, std::size_t size, bool dynamic) {
        release_buffer(buffer);
		bind();
        glGenBuffers(1, &buffer);                                           easy3d_debug_log_gl_error
        LOG_IF(buffer == 0, ERROR) << "failed creating element array buffer";
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);                      easy3d_debug_log_gl_error
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<GLsizeiptr>(size), data, dynamic ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);		easy3d_debug_log_gl_error
        if (glGetError() != GL_NO_ERROR) {
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);                       easy3d_debug_log_gl_error
            glDeleteBuffers(1, &buffer);                                    easy3d_debug_log_gl_error
            buffer = 0;
            LOG(ERROR) << "failed creating element array buffer";
		}
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);	easy3d_debug_log_gl_error
        release();
        return (glGetError() == GL_NO_ERROR && buffer != 0);
	}


    bool VertexArrayObject::create_storage_buffer(GLuint& buffer, GLuint index, const void* data, std::size_t size) {
        if (!OpenglUtil::is_supported("GL_ARB_shader_storage_buffer_object")) {
            LOG(ERROR) << "shader storage buffer object not supported on this platform";
            return false;
        }
        release_buffer(buffer);
		bind();
        glGenBuffers(1, &buffer);                                                   easy3d_debug_log_gl_error
        LOG_IF(buffer == 0, ERROR) << "failed creating shader storage buffer";
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);                             easy3d_debug_log_gl_error
        glBufferData(GL_SHADER_STORAGE_BUFFER, static_cast<GLsizeiptr>(size), data, GL_DYNAMIC_DRAW);		easy3d_debug_log_gl_error
        glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, buffer);                   easy3d_debug_log_gl_error
        if (glGetError() != GL_NO_ERROR) {
            glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);                              easy3d_debug_log_gl_error
            glDeleteBuffers(1, &buffer);                                            easy3d_debug_log_gl_error
            buffer = 0;
            LOG(ERROR) << "failed creating shader storage buffer";
		}
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);                                  easy3d_debug_log_gl_error
        release();                                                                  easy3d_debug_log_gl_error
        return (glGetError() == GL_NO_ERROR && buffer != 0);
	}


    bool VertexArrayObject::update_storage_buffer(GLuint& buffer, GLintptr offset, GLsizeiptr size, const void* data) {
		bind();
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, buffer);                     easy3d_debug_log_gl_error
		glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);		easy3d_debug_log_gl_error
        glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);                          easy3d_debug_log_gl_error
        release();                                                          easy3d_debug_log_gl_error
        return (glGetError() == GL_NO_ERROR);
	}


    void* VertexArrayObject::map_buffer(GLenum target, GLuint buffer, GLenum access) {
		// Liangliang: should work, but haven't tested yet.
        glBindBuffer(target, buffer);                   easy3d_debug_log_gl_error
		// Liangliang: Finally, I found this is the place to call glMemoryBarrier()
		// we don't have shader storage buffer
//		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);	easy3d_debug_log_gl_error
        void* ptr = glMapBuffer(target, access);        easy3d_debug_log_gl_error
        glBindBuffer(target, 0);                        easy3d_debug_log_gl_error
		return ptr;
	}


    void VertexArrayObject::unmap_buffer(GLenum target, GLuint buffer) {
		// Liangliang: should work, but haven't tested yet.
        glBindBuffer(target, buffer);	easy3d_debug_log_gl_error
        glUnmapBuffer(target);          easy3d_debug_log_gl_error
        glBindBuffer(target, 0);        easy3d_debug_log_gl_error
	}


    void VertexArrayObject::get_buffer_data(GLenum target, GLuint buffer, GLintptr offset, GLsizeiptr size, void* data) {
        glBindBuffer(target, buffer);                   easy3d_debug_log_gl_error
		// Liangliang: Finally, I found this is the place to call glMemoryBarrier()
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);	easy3d_debug_log_gl_error
		glGetBufferSubData(target, offset, size, data);	easy3d_debug_log_gl_error
        glBindBuffer(target, 0);                        easy3d_debug_log_gl_error
	}
}
