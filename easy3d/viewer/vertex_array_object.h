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


#ifndef EASY_VERTEX_ARRAY_OBJECT_H
#define EASY_VERTEX_ARRAY_OBJECT_H

#include <easy3d/viewer/opengl.h>

#include <string>


namespace easy3d {

	/********************************************************************************************
	*
	* The VertexArrayObject class is a thin wrapper around an OpenGL Vertex Array Object (VAO).
	* A VAO is an OpenGL container object that encapsulates the state needed to specify
	* per-vertex attribute data to the OpenGL pipeline. To put it another way, a VAO
	* remembers the states of buffer objects (i.e., vertex buffers) and their associated
	* state (e.g. vertex attribute divisors). This allows a very easy and efficient method
	* of switching between OpenGL buffer states for rendering different "objects" in a scene.
	*
	* NOTE: this implementation is more than the standard VAO. It also provides functions
	*       for creating and managing other buffer objects (i.e., shader storage buffer).
	*
	*	Version:	 1.0
	*	created:	 Dec. 14, 2015
	*	author:		 Liangliang Nan
	*	contact:     liangliang.nan@gmail.com
	*
	*********************************************************************************************/


	class VertexArrayObject
	{
	public:
        static bool is_supported();

		VertexArrayObject();
		~VertexArrayObject();

		GLuint id() const { return id_; }

		void bind();
        void release();

		//------------------------- buffer management -------------------

		// @param index: the index of the generic vertex attribute.
		GLint create_array_buffer(GLuint& handle, GLuint index, const void* data, std::size_t datasize, GLenum data_type, std::size_t vectordim);
		GLint create_index_buffer(GLuint& handle, const void* data, std::size_t datasize);

		// @param index: the index of the binding point.
		GLint create_storage_buffer(GLuint& handle, GLuint index, const void* data, std::size_t datasize);
		GLint update_storage_buffer(GLuint& handle, GLintptr offset, GLsizeiptr size, const void* data);

		// free the GPU memory of the buffer specified by 'handle'
		static void release_buffer(GLuint& handle);

		// ------------------------- read/write buffer--------------------

		// returns a subset of a buffer object's data store
		// \param target: can be GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_SHADER_STORAGE_BUFFER, etc.
		// \param handle: the name of the buffer object.
		// \param offset: the offset into the buffer object's data store from which data will be returned, 
		//				  measured in bytes.
		// \param size:   the size in bytes of the data store region being returned.
		// \param data:   a pointer to the location where buffer object data is returned.
		void get_buffer_data(GLenum target, GLuint handle, GLintptr offset, GLsizeiptr size, void* data);

		// starting from OpenGL 4.5, you don't need to provide the target. 
		// It internally uses glGetNamedBufferSubData().
		void get_named_buffer_data(GLuint handle, GLintptr offset, GLsizeiptr size, void* data);


		// \param target: can be GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_SHADER_STORAGE_BUFFER, etc.
		// \param handle: the name of the buffer object for the mapping.
		// \param access: must be GL_READ_ONLY, GL_WRITE_ONLY, or GL_READ_WRITE.
		static void* map_buffer(GLenum target, GLuint handle, GLenum access);
		static void  unmap_buffer(GLenum target, GLuint handle);

		// starting from OpenGL 4.5, you don't need to provide the target. 
		// It internally uses glMapNamedBuffer() and glUnmapNamedBuffer.
		static void* map_named_buffer(GLuint handle, GLenum access);
		static void  unmap_named_buffer(GLuint handle);

	private:
		GLuint	id_;

    private:
        //copying disabled
        VertexArrayObject(const VertexArrayObject&);
        VertexArrayObject& operator=(const VertexArrayObject&);
	};

}


#endif  // EASY_VERTEX_ARRAY_OBJECT_H
