#include <easy3d/algo/text_mesher.h>
#include <easy3d/core/polygon.h>
#include <easy3d/core/surface_mesh.h>

#include <memory>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif


void bind_easy3d_algo_text_mesher(pybind11::module_& m)
{
	{ // easy3d::TextMesher file:easy3d/algo/text_mesher.h line:42
		pybind11::class_<easy3d::TextMesher, std::shared_ptr<easy3d::TextMesher>> cl(m, "TextMesher", "Generate a 3D surface mesh from a text string.\n \n");
		cl.def( pybind11::init( [](const std::string & a0){ return new easy3d::TextMesher(a0); } ), "doc" , pybind11::arg("font_file"));
		cl.def( pybind11::init<const std::string &, unsigned short>(), pybind11::arg("font_file"), pybind11::arg("quality") );

		cl.def( pybind11::init( [](easy3d::TextMesher const &o){ return new easy3d::TextMesher(o); } ) );
		cl.def("set_font", (void (easy3d::TextMesher::*)(const std::string &)) &easy3d::TextMesher::set_font, "Change the font.\n \n\n The full path name to a TrueType font file (normally with an \"ttf\" extension).\n \n\n The font will be used in all subsequent generation until being modified by calling to set_font().\n\nC++: easy3d::TextMesher::set_font(const std::string &) --> void", pybind11::arg("font_file"));
		cl.def("generate", [](easy3d::TextMesher &o, const std::string & a0, float const & a1, float const & a2, int const & a3, float const & a4) -> easy3d::SurfaceMesh * { return o.generate(a0, a1, a2, a3, a4); }, "", pybind11::return_value_policy::automatic, pybind11::arg("text"), pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("font_size"), pybind11::arg("extrude"));
		cl.def("generate", (class easy3d::SurfaceMesh * (easy3d::TextMesher::*)(const std::string &, float, float, int, float, bool)) &easy3d::TextMesher::generate, "Generate a 3D surface mesh of a text.\n \n\n The input text.\n \n\n The x-coordinate of the starting position.\n \n\n The y-coordinate of the starting position.\n \n\n The size of the font.\n \n\n The height (in the Z direction) of the 3D model.\n \n\n If true, the final mesh will be free of intersections between characters.\n \n\n The generated triangular surface mesh.\n\nC++: easy3d::TextMesher::generate(const std::string &, float, float, int, float, bool) --> class easy3d::SurfaceMesh *", pybind11::return_value_policy::automatic, pybind11::arg("text"), pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("font_size"), pybind11::arg("extrude"), pybind11::arg("collision_free"));
		cl.def("generate", [](easy3d::TextMesher &o, class easy3d::SurfaceMesh * a0, const std::string & a1, float const & a2, float const & a3, int const & a4, float const & a5) -> bool { return o.generate(a0, a1, a2, a3, a4, a5); }, "", pybind11::arg("mesh"), pybind11::arg("text"), pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("font_size"), pybind11::arg("extrude"));
		cl.def("generate", (bool (easy3d::TextMesher::*)(class easy3d::SurfaceMesh *, const std::string &, float, float, int, float, bool)) &easy3d::TextMesher::generate, "Generate 3D surface representation of a text and append the surface to an existing mesh.\n \n\n The input text.\n \n\n The x-coordinate of the starting position.\n \n\n The y-coordinate of the starting position.\n \n\n The size of the font.\n \n\n The height (in the Z direction) of the 3D model.\n \n\n If true, the final mesh will be free of intersections between characters.\n \n\n  on success and  on failure.\n\nC++: easy3d::TextMesher::generate(class easy3d::SurfaceMesh *, const std::string &, float, float, int, float, bool) --> bool", pybind11::arg("mesh"), pybind11::arg("text"), pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("font_size"), pybind11::arg("extrude"), pybind11::arg("collision_free"));
		cl.def("generate", (bool (easy3d::TextMesher::*)(const std::string &, float, float, int, class std::vector<class std::vector<class easy3d::GenericPolygon<float> > > &, bool)) &easy3d::TextMesher::generate, "Generate contours from a text.\n \n\n The input text.\n \n\n The x-coordinate of the starting position.\n \n\n The y-coordinate of the starting position.\n \n\n The size of the font.\n \n\n The contours of the text (each character may have multiple contours). The generated contours\n        are simply appended to his variable.\n \n\n If true, the generated contours will be free of intersections between characters.\n\nC++: easy3d::TextMesher::generate(const std::string &, float, float, int, class std::vector<class std::vector<class easy3d::GenericPolygon<float> > > &, bool) --> bool", pybind11::arg("text"), pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("font_size"), pybind11::arg("contours"), pybind11::arg("collision_free"));
		cl.def("assign", (class easy3d::TextMesher & (easy3d::TextMesher::*)(const class easy3d::TextMesher &)) &easy3d::TextMesher::operator=, "C++: easy3d::TextMesher::operator=(const class easy3d::TextMesher &) --> class easy3d::TextMesher &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}

}
