#include <easy3d/renderer/eye_dome_lighting.h>
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

void bind_easy3d_renderer_eye_dome_lighting(pybind11::module_& m)
{
	{ // easy3d::EyeDomeLighting file:easy3d/renderer/eye_dome_lighting.h line:70
		pybind11::class_<easy3d::EyeDomeLighting, std::shared_ptr<easy3d::EyeDomeLighting>> cl(m, "EyeDomeLighting", "");
		cl.def("begin", (void (easy3d::EyeDomeLighting::*)()) &easy3d::EyeDomeLighting::begin, "C++: easy3d::EyeDomeLighting::begin() --> void");
		cl.def("end", (void (easy3d::EyeDomeLighting::*)()) &easy3d::EyeDomeLighting::end, "C++: easy3d::EyeDomeLighting::end() --> void");
	}
}
