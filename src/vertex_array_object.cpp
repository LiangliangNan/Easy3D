#include "vertex_array_object.h"
#include "opengl_error.h"
#include "opengl_info.h"
#include <cassert>


VertexArrayObject::VertexArrayObject()
	: id_(0)
{
	// Liangliang: it is always a bad idea to initialize some OpenGL stuff in the constructors
	//			   because the OpenGL context may not exist or the current context is not the 
	//			   correct one. It is always correct "create when needed".
	//if (is_supported()) {
	//	glGenVertexArrays(1, &id_);	mpl_debug_gl_error;
	//	if (id_ == 0) {
	//		std::cerr << "generating VertexArrayObject failed" << std::endl;
	//	}
	//}
}


VertexArrayObject::~VertexArrayObject() {
	if (id_ != 0) {
		glDeleteVertexArrays(1, &id_);	mpl_debug_gl_error;
		id_ = 0;
	}
}


void VertexArrayObject::bind() {
	if (id_ == 0) {
		glGenVertexArrays(1, &id_);	mpl_debug_gl_error;
		if (id_ == 0) {
			std::cerr << "generating VertexArrayObject failed" << std::endl;
			return;
		}
	}

	glBindVertexArray(id_);		mpl_debug_gl_error;
}


void VertexArrayObject::unbind() {
	assert(id_ != 0);
	glBindVertexArray(0);		mpl_debug_gl_error;
}


void VertexArrayObject::release_buffer(GLuint& handle) {
	if (handle != 0) {
		glBindVertexArray(0);			mpl_debug_gl_error;
		glDeleteBuffers(1, &handle);	mpl_debug_gl_error;
		handle = 0;
	}
}


GLint VertexArrayObject::create_array_buffer(GLuint& handle, GLuint index, const void* data, std::size_t datasize, GLenum data_type, std::size_t vectordim) {
	release_buffer(handle);
	bind();

	glGenBuffers(1, &handle);			mpl_debug_gl_error;
	glBindBuffer(GL_ARRAY_BUFFER, handle);			mpl_debug_gl_error;
	glBufferData(GL_ARRAY_BUFFER, datasize, data, GL_STATIC_DRAW);		mpl_debug_gl_error;
	glVertexAttribPointer(index, int(vectordim), data_type, GL_FALSE, 0, NULL);		mpl_debug_gl_error;
	GLint err = glGetError();
	if (err != GL_NO_ERROR) {
		glBindBuffer(GL_ARRAY_BUFFER, 0);		mpl_debug_gl_error;
		glDeleteBuffers(1, &handle);			mpl_debug_gl_error;
		handle = 0;
	}
	glBindBuffer(GL_ARRAY_BUFFER, 0);			mpl_debug_gl_error;

	unbind();
	
	return err;
}


GLint VertexArrayObject::create_index_buffer(GLuint& handle, const void* data, std::size_t datasize) {
	release_buffer(handle);
	bind();

	glGenBuffers(1, &handle);					mpl_debug_gl_error;
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, handle);		mpl_debug_gl_error;
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, datasize, data, GL_STATIC_DRAW);		mpl_debug_gl_error;
	GLint err = glGetError();	mpl_debug_gl_error;
	if (err != GL_NO_ERROR) {
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);		mpl_debug_gl_error;
		glDeleteBuffers(1, &handle);		mpl_debug_gl_error;
		handle = 0;
	}
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);	mpl_debug_gl_error;
	mpl_debug_gl_error;

	unbind();

	return err;
}


GLint VertexArrayObject::create_storage_buffer(GLuint& handle, GLuint index, const void* data, std::size_t datasize) {
	if (!OpenglInfo::is_supported("GL_ARB_shader_storage_buffer_object"))
		return -1;

	release_buffer(handle);
	bind();

	glGenBuffers(1, &handle);				mpl_debug_gl_error;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);			mpl_debug_gl_error;
	glBufferData(GL_SHADER_STORAGE_BUFFER, datasize, data, GL_DYNAMIC_DRAW);		mpl_debug_gl_error;
	GLint err = glGetError();					mpl_debug_gl_error;
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, index, handle);		mpl_debug_gl_error;
	if (err != GL_NO_ERROR) {
		glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);			mpl_debug_gl_error;
		glDeleteBuffers(1, &handle);			mpl_debug_gl_error;
		handle = 0;
	}
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);		mpl_debug_gl_error;
	mpl_debug_gl_error;

	unbind();

	return err;
}


GLint VertexArrayObject::update_storage_buffer(GLuint& handle, GLintptr offset, GLsizeiptr size, const void* data) {
	bind();

	glBindBuffer(GL_SHADER_STORAGE_BUFFER, handle);		mpl_debug_gl_error;
	glBufferSubData(GL_SHADER_STORAGE_BUFFER, offset, size, data);		mpl_debug_gl_error;
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);		mpl_debug_gl_error;

	unbind();

	return glGetError();	mpl_debug_gl_error;
}


void* VertexArrayObject::map_buffer(GLenum target, GLuint handle, GLenum access) {
	// Liangliang: should work, but haven't tested yet.
	glBindBuffer(target, handle);	mpl_debug_gl_error;
	
	// Liangliang: Finally, I found this is the place to call glMemoryBarrier()
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);	mpl_debug_gl_error;
	
	void* ptr = glMapBuffer(handle, access);	mpl_debug_gl_error;
	glBindBuffer(target, 0);	mpl_debug_gl_error;
	return ptr;
}


void VertexArrayObject::unmap_buffer(GLenum target, GLuint handle) {
	// Liangliang: should work, but haven't tested yet.
	glBindBuffer(target, handle);	mpl_debug_gl_error;
	glUnmapBuffer(handle);	mpl_debug_gl_error;
	glBindBuffer(target, 0);	mpl_debug_gl_error;
}


void* VertexArrayObject::map_named_buffer(GLuint handle, GLenum access) {
	// Liangliang: Finally, I found this is the place to call glMemoryBarrier()
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);	mpl_debug_gl_error;
	
	return glMapNamedBuffer(handle, access);
}


void VertexArrayObject::unmap_named_buffer(GLuint handle) {
	glUnmapNamedBuffer(handle);	mpl_debug_gl_error;
}


void VertexArrayObject::get_buffer_data(GLenum target, GLuint handle, GLintptr offset, GLsizeiptr size, void* data) {
	glBindBuffer(target, handle);	mpl_debug_gl_error;
	
	// Liangliang: Finally, I found this is the place to call glMemoryBarrier()
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);	mpl_debug_gl_error;
	
	glGetBufferSubData(target, offset, size, data);	mpl_debug_gl_error;
	glBindBuffer(target, 0);	mpl_debug_gl_error;
}


void VertexArrayObject::get_named_buffer_data(GLuint handle, GLintptr offset, GLsizeiptr size, void* data) {
	// Liangliang: Finally, I found this is the place to call glMemoryBarrier()
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);	mpl_debug_gl_error;
	
	glGetNamedBufferSubData(handle, offset, size, data);	mpl_debug_gl_error;
}