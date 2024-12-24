#include <easy3d/algo/collider.h>
#include <easy3d/core/mat.h>
#include <easy3d/core/surface_mesh.h>

#include <memory>
#include <utility>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif


void bind_easy3d_algo_collider(pybind11::module_& m)
{
	{ // easy3d::Collider file:easy3d/algo/collider.h line:51
		pybind11::class_<easy3d::Collider, std::shared_ptr<easy3d::Collider>> cl(m, "Collider", "Efficient collision detection.\n \n\n This class takes two triangle meshes and their transformation matrices as input and outputs the\n      intersecting face pairs. This implementation is a wrapper of Opcode. It can achieve real-time performance\n      for large meshes.\n \n\n\n \n Extension to general polygonal meshes (by internally triangulate the input).");
		cl.def( pybind11::init<class easy3d::SurfaceMesh *, class easy3d::SurfaceMesh *>(), pybind11::arg("mesh0"), pybind11::arg("mesh1") );

		cl.def("detect", (class std::vector<struct std::pair<struct easy3d::SurfaceMesh::Face, struct easy3d::SurfaceMesh::Face> > (easy3d::Collider::*)(const class easy3d::Mat4<float> &, const class easy3d::Mat4<float> &) const) &easy3d::Collider::detect, "Performs collision detection.\n \n\n The transformation of the first mesh.\n \n\n The transformation of the second mesh.\n \n\n The intersecting face pairs.\n\nC++: easy3d::Collider::detect(const class easy3d::Mat4<float> &, const class easy3d::Mat4<float> &) const --> class std::vector<struct std::pair<struct easy3d::SurfaceMesh::Face, struct easy3d::SurfaceMesh::Face> >", pybind11::arg("t0"), pybind11::arg("t1"));
	}

}
