#include <easy3d/renderer/vertex_array_object.h>
#include <sstream> // __str__

#include <functional>
#include <pybind11/pybind11.h>
#include <string>
#include <pybind11/stl.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_renderer_vertex_array_object(pybind11::module_& m)
{
	{ // easy3d::VertexArrayObject file:easy3d/renderer/vertex_array_object.h line:51
		pybind11::class_<easy3d::VertexArrayObject, std::shared_ptr<easy3d::VertexArrayObject>> cl(m, "VertexArrayObject", "A thin wrapper around an OpenGL Vertex Array Object (VAO).\n\n \n\n \n A VAO is an OpenGL container object that encapsulates the state needed to specify\n per-vertex attribute data to the OpenGL pipeline. To put it another way, a VAO\n remembers the states of buffer objects (i.e., vertex buffers) and their associated\n state (e.g. vertex attribute divisors). This allows a very easy and efficient method\n of switching between OpenGL buffer states for rendering different \"objects\" in a scene.\n\n \n This implementation is more than the standard VAO. It also provides functions\n       for creating and managing other buffer objects (i.e., shader storage buffer).");
		cl.def( pybind11::init( [](){ return new easy3d::VertexArrayObject(); } ) );
		cl.def_static("is_supported", (bool (*)()) &easy3d::VertexArrayObject::is_supported, "C++: easy3d::VertexArrayObject::is_supported() --> bool");
		cl.def("id", (unsigned int (easy3d::VertexArrayObject::*)() const) &easy3d::VertexArrayObject::id, "C++: easy3d::VertexArrayObject::id() const --> unsigned int");
		cl.def("bind", (void (easy3d::VertexArrayObject::*)()) &easy3d::VertexArrayObject::bind, "C++: easy3d::VertexArrayObject::bind() --> void");
		cl.def("release", (void (easy3d::VertexArrayObject::*)() const) &easy3d::VertexArrayObject::release, "C++: easy3d::VertexArrayObject::release() const --> void");
		cl.def("create_array_buffer", [](easy3d::VertexArrayObject &o, unsigned int & a0, unsigned int const & a1, const void * a2, std::size_t const & a3, std::size_t const & a4) -> bool { return o.create_array_buffer(a0, a1, a2, a3, a4); }, "", pybind11::arg("buffer"), pybind11::arg("index"), pybind11::arg("data"), pybind11::arg("size"), pybind11::arg("dim"));
		cl.def("create_array_buffer", (bool (easy3d::VertexArrayObject::*)(unsigned int &, unsigned int, const void *, std::size_t, std::size_t, bool)) &easy3d::VertexArrayObject::create_array_buffer, "Creates an OpenGL array buffer and upload data to the buffer.\n \n\n The name of the buffer object.\n \n\n  The index of the generic vertex attribute to be enabled.\n \n\n   The pointer to the data.\n \n\n   The size of the data in bytes.\n \n\n    The number of components per generic vertex attribute. Must be 1, 2, 3, 4.\n \n\n The expected usage pattern is GL_STATIC_DRAW or GL_DYNAMIC_DRAW.\n \n\n OpenGL error code.\n\nC++: easy3d::VertexArrayObject::create_array_buffer(unsigned int &, unsigned int, const void *, std::size_t, std::size_t, bool) --> bool", pybind11::arg("buffer"), pybind11::arg("index"), pybind11::arg("data"), pybind11::arg("size"), pybind11::arg("dim"), pybind11::arg("dynamic"));
		cl.def("create_element_buffer", [](easy3d::VertexArrayObject &o, unsigned int & a0, const void * a1, std::size_t const & a2) -> bool { return o.create_element_buffer(a0, a1, a2); }, "", pybind11::arg("buffer"), pybind11::arg("data"), pybind11::arg("size"));
		cl.def("create_element_buffer", (bool (easy3d::VertexArrayObject::*)(unsigned int &, const void *, std::size_t, bool)) &easy3d::VertexArrayObject::create_element_buffer, "C++: easy3d::VertexArrayObject::create_element_buffer(unsigned int &, const void *, std::size_t, bool) --> bool", pybind11::arg("buffer"), pybind11::arg("data"), pybind11::arg("size"), pybind11::arg("dynamic"));
		cl.def("create_storage_buffer", (bool (easy3d::VertexArrayObject::*)(unsigned int &, unsigned int, const void *, std::size_t)) &easy3d::VertexArrayObject::create_storage_buffer, "C++: easy3d::VertexArrayObject::create_storage_buffer(unsigned int &, unsigned int, const void *, std::size_t) --> bool", pybind11::arg("buffer"), pybind11::arg("index"), pybind11::arg("data"), pybind11::arg("size"));
		cl.def("update_storage_buffer", (bool (easy3d::VertexArrayObject::*)(unsigned int &, long, long, const void *)) &easy3d::VertexArrayObject::update_storage_buffer, "C++: easy3d::VertexArrayObject::update_storage_buffer(unsigned int &, long, long, const void *) --> bool", pybind11::arg("buffer"), pybind11::arg("offset"), pybind11::arg("size"), pybind11::arg("data"));
		cl.def_static("release_buffer", (void (*)(unsigned int &)) &easy3d::VertexArrayObject::release_buffer, "Frees the GPU memory of the buffer specified by 'handle'\n\nC++: easy3d::VertexArrayObject::release_buffer(unsigned int &) --> void", pybind11::arg("buffer"));
		cl.def_static("get_buffer_data", (void (*)(unsigned int, unsigned int, long, long, void *)) &easy3d::VertexArrayObject::get_buffer_data, "Returns a subset of a buffer object's data store.\n \n\n can be GL_ARRAY_BUFFER, GL_ELEMENT_ARRAY_BUFFER, GL_SHADER_STORAGE_BUFFER, etc.\n \n\n the name of the buffer object.\n \n\n the offset into the buffer object's data store from which data will be returned,\n      measured in bytes.\n \n\n   the size in bytes of the data store region being returned.\n \n\n   a pointer to the location where buffer object data is returned.\n\nC++: easy3d::VertexArrayObject::get_buffer_data(unsigned int, unsigned int, long, long, void *) --> void", pybind11::arg("target"), pybind11::arg("buffer"), pybind11::arg("offset"), pybind11::arg("size"), pybind11::arg("data"));
		cl.def_static("map_buffer", (void * (*)(unsigned int, unsigned int, unsigned int)) &easy3d::VertexArrayObject::map_buffer, "C++: easy3d::VertexArrayObject::map_buffer(unsigned int, unsigned int, unsigned int) --> void *", pybind11::return_value_policy::automatic, pybind11::arg("target"), pybind11::arg("buffer"), pybind11::arg("access"));
		cl.def_static("unmap_buffer", (void (*)(unsigned int, unsigned int)) &easy3d::VertexArrayObject::unmap_buffer, "C++: easy3d::VertexArrayObject::unmap_buffer(unsigned int, unsigned int) --> void", pybind11::arg("target"), pybind11::arg("buffer"));
	}
}
