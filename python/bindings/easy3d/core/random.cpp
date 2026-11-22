#include <easy3d/core/random.h>
#include <easy3d/core/vec.h>

#include <memory>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif


void bind_easy3d_core_random(pybind11::module_& m)
{
	// easy3d::random_float() file:easy3d/core/random.h line:39
	m.def("random_float", (float (*)()) &easy3d::random_float, "Random real in [0, 1]. \n\nC++: easy3d::random_float() --> float");

	// easy3d::random_float(float, float) file:easy3d/core/random.h line:44
	m.def("random_float", (float (*)(float, float)) &easy3d::random_float, "Random real number in the range [min, max]. \n\nC++: easy3d::random_float(float, float) --> float", pybind11::arg("min"), pybind11::arg("max"));

	// easy3d::random_color(bool) file:easy3d/core/random.h line:49
	m.def("random_color", []() -> easy3d::Vec<3, float> { return easy3d::random_color(); }, "");
	m.def("random_color", (class easy3d::Vec<3, float> (*)(bool)) &easy3d::random_color, "Generates a random color. The parameter  controls if too dark colors are allowed. \n\nC++: easy3d::random_color(bool) --> class easy3d::Vec<3, float>", pybind11::arg("allow_dark"));

	// easy3d::color_from_index(std::size_t) file:easy3d/core/random.h line:146
	m.def("color_from_index", (class easy3d::Vec<3, float> (*)(std::size_t)) &easy3d::color_from_index, "Generates a color by an index. Colors from different indices have visual distinction.\n The hue of an index steps by the golden angle around the color wheel, and the result depends only on\n the index: the same thing keeps its color across a redraw, a rerun and a second viewer. The\n closest pair of the colors of the first 8, 16, 32 and 64 indices is 0.26, 0.14, 0.06 and 0.03\n apart in RGB, so the distinction is a claim about a handful to a few dozen colors, not hundreds.\n \n\n index: which color of the sequence. It is a size, so the wheel is stepped forward from zero\n         and never back: a negative index raises TypeError instead of wrapping around.\n\nC++: easy3d::color_from_index(std::size_t) --> class easy3d::Vec<3, float>", pybind11::arg("index"));

	// easy3d::color_table(std::size_t) file:easy3d/core/random.h line:168
	m.def("color_table", (class std::vector<class easy3d::Vec<3, float>> (*)(std::size_t)) &easy3d::color_table, "Generates a table of distinct colors, as a list. The table holds easy3d::color_from_index() of the\n indices 0, 1, ... num - 1, so the entry at position i is the color of index i and the two always\n agree. Build one table for a whole set of things (segments, groups, components) rather than\n calling easy3d::color_from_index() with an unrelated counter: the table is what makes the indices\n of the set the indices of the colors. See easy3d::color_from_index() for how long the colors stay\n distinct from each other.\n \n\n num: number of colors, and the size of the table; zero gives an empty list. It is a size rather\n         than an index, so a negative count raises TypeError instead of giving a table.\n\nC++: easy3d::color_table(std::size_t) --> class std::vector<class easy3d::Vec<3, float>>", pybind11::arg("num"));
}
