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

#ifndef EASY_RENDERER_VERTEX_ARRAY_OBJECT_H
#define EASY_RENDERER_VERTEX_ARRAY_OBJECT_H

#include <easy3d/renderer/opengl.h>

#include <string>


namespace easy3d {

	/**
	 * \brief A thin wrapper around an OpenGL Vertex Array Object (VAO).
	 *
	 * \class VertexArrayObject easy3d/renderer/vertex_array_object.h
	 *
	 * \details A VAO is an OpenGL container object that encapsulates the state needed to specify
	 * per-vertex attribute data to the OpenGL pipeline. To put it another way, a VAO
	 * remembers the states of buffer objects (i.e., vertex buffers) and their associated
	 * state (e.g. vertex attribute divisors). This allows a very easy and efficient method
	 * of switching between OpenGL buffer states for rendering different "objects" in a scene.
	 *
	 * \note This implementation is more than the standard VAO. It also provides functions
	 *       for creating and managing other buffer objects (i.e., shader storage buffer).
	 */
	class VertexArrayObject
	{
	public:
        static bool is_supported();

		VertexArrayObject();
		~VertexArrayObject();

		GLuint id() const { return id_; }

		void bind();
        void release() const;

		//------------------------- buffer management -------------------

        /**
         * @brief Creates an OpenGL array buffer and upload data to the buffer.
         * @param handle The name of the buffer object.
         * @param index  The index of the generic vertex attribute to be enabled.
         * @param data   The pointer to the data.
         * @param size   The size of the data in bytes.
         * @param dim    The number of components per generic vertex attribute. Must be 1, 2, 3, 4.
         * @param dynamic The expected usage pattern is GL_STATIC_DRAW or GL_DYNAMIC_DRAW.
         * @return OpenGL error code.
         */
        bool create_array_buffer(GLuint& buffer, GLuint index, const void* data, std::size_t size, std::size_t dim, bool dynamic = false);
        bool create_element_buffer(GLuint& buffer, const void* data, std::size_t size, bool dynamic = false);

        // @param index: the index of the binding point.
        bool create_storage_buffer(GLuint& buffer, GLuint index, const void* data, std::size_t size);
        bool update_storage_buffer(GLuint& buffer, GLintptr offset, GLsizeiptr size, const void* data);

		/// Frees the GPU memory of the buffer specified by 'handle'
        static void release_buffer(GLuint& buffer);

		// ------------------------- read/write buffer--------------------

		/**
		 * Returns a subset of a buffer object's data store.
		 * \param target: can be GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_SHADER_STORAGE_BUFFER, etc.
		 * \param handle: the name of the buffer object.
		 * \param offset: the offset into the buffer object's data store from which data will be returned,
		 *      measured in bytes.
		 * \param size:   the size in bytes of the data store region being returned.
		 * \param data:   a pointer to the location where buffer object data is returned.
		 */
        static void get_buffer_data(GLenum target, GLuint buffer, GLintptr offset, GLsizeiptr size, void* data);

		// \param target: can be GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_SHADER_STORAGE_BUFFER, etc.
		// \param handle: the name of the buffer object for the mapping.
		// \param access: must be GL_READ_ONLY, GL_WRITE_ONLY, or GL_READ_WRITE.
        static void* map_buffer(GLenum target, GLuint buffer, GLenum access);
        static void  unmap_buffer(GLenum target, GLuint buffer);

	private:
		GLuint	id_;

    private:
        //copying disabled
        VertexArrayObject(const VertexArrayObject&);
        VertexArrayObject& operator=(const VertexArrayObject&);
	};

}


#endif  // EASY_RENDERER_VERTEX_ARRAY_OBJECT_H
