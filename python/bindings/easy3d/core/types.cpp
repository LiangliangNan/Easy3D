#include <easy3d/core/types.h>
#include <easy3d/core/vec.h>

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

void bind_easy3d_core_types(pybind11::module_& m)
{
    // easy3d::geom::bounding_box(const std::vector<easy3d::vec3> &) file:easy3d/core/types.h line:179
    m.def("bounding_box", [](const std::vector<easy3d::vec3> &points) {
              return easy3d::geom::bounding_box<easy3d::Box3, std::vector<easy3d::vec3> >(points);
          }, pybind11::arg("points"),
          R"doc(
            Compute the bounding box of a set of points.

            Parameters:
                points (List[vec3]): A list of 3D points.

            Returns:
                box3: The computed bounding box.
        )doc");

	// easy3d::geom::orthogonal(const class easy3d::Vec<3, float> &) file:easy3d/core/types.h line:163
	m.def("orthogonal", (class easy3d::Vec<3, float> (*)(const class easy3d::Vec<3, float> &)) &easy3d::geom::orthogonal, "Returns a vector orthogonal to v. Its norm() depends on v, but is zero only for a null v.\n\nC++: easy3d::geom::orthogonal(const class easy3d::Vec<3, float> &) --> class easy3d::Vec<3, float>", pybind11::arg("v"));

	// easy3d::geom::point_in_polygon(const class easy3d::Vec<2, float> &, const class std::vector<class easy3d::Vec<2, float> > &) file:easy3d/core/types.h line:227
	m.def("point_in_polygon", (bool (*)(const class easy3d::Vec<2, float> &, const class std::vector<class easy3d::Vec<2, float> > &)) &easy3d::geom::point_in_polygon, "Tests if a point  lies inside or outside of a \n This function is robust to handle general polygons (no matter convex or concave).\n\nC++: easy3d::geom::point_in_polygon(const class easy3d::Vec<2, float> &, const class std::vector<class easy3d::Vec<2, float> > &) --> bool", pybind11::arg("p"), pybind11::arg("polygon"));

	// easy3d::geom::clamp_cot(const double) file:easy3d/core/types.h line:230
	m.def("clamp_cot", (double (*)(const double)) &easy3d::geom::clamp_cot, "Clamps cotangent values as if angles are in [1, 179]    \n\nC++: easy3d::geom::clamp_cot(const double) --> double", pybind11::arg("v"));

	// easy3d::geom::clamp_cos(const double) file:easy3d/core/types.h line:236
	m.def("clamp_cos", (double (*)(const double)) &easy3d::geom::clamp_cos, "Clamps cosine values as if angles are in [1, 179]    \n\nC++: easy3d::geom::clamp_cos(const double) --> double", pybind11::arg("v"));

	// easy3d::geom::triangle_area(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) file:easy3d/core/types.h line:278
	m.def("triangle_area", (float (*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &)) &easy3d::geom::triangle_area, "Computes area of a triangle given by three points. \n\nC++: easy3d::geom::triangle_area(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) --> float", pybind11::arg("p1"), pybind11::arg("p2"), pybind11::arg("p3"));

	// easy3d::geom::triangle_signed_area(const class easy3d::Vec<2, float> &, const class easy3d::Vec<2, float> &, const class easy3d::Vec<2, float> &) file:easy3d/core/types.h line:283
	m.def("triangle_signed_area", (float (*)(const class easy3d::Vec<2, float> &, const class easy3d::Vec<2, float> &, const class easy3d::Vec<2, float> &)) &easy3d::geom::triangle_signed_area, "Computes signed area of a triangle given by three points. \n\nC++: easy3d::geom::triangle_signed_area(const class easy3d::Vec<2, float> &, const class easy3d::Vec<2, float> &, const class easy3d::Vec<2, float> &) --> float", pybind11::arg("p1"), pybind11::arg("p2"), pybind11::arg("p3"));

	// easy3d::geom::triangle_normal(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) file:easy3d/core/types.h line:288
	m.def("triangle_normal", (class easy3d::Vec<3, float> (*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &)) &easy3d::geom::triangle_normal, "Computes the normal vector of a triangle given by three points. \n\nC++: easy3d::geom::triangle_normal(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) --> class easy3d::Vec<3, float>", pybind11::arg("p1"), pybind11::arg("p2"), pybind11::arg("p3"));

	// easy3d::geom::dist_point_line_segment(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, class easy3d::Vec<3, float> &) file:easy3d/core/types.h line:294
	m.def("dist_point_line_segment", (float (*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, class easy3d::Vec<3, float> &)) &easy3d::geom::dist_point_line_segment, "Computes the distance of a point p to a line segment given by vec3s (v0,v1). \n\nC++: easy3d::geom::dist_point_line_segment(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, class easy3d::Vec<3, float> &) --> float", pybind11::arg("p"), pybind11::arg("v0"), pybind11::arg("v1"), pybind11::arg("nearest_point"));

	// easy3d::geom::dist_point_triangle(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, class easy3d::Vec<3, float> &) file:easy3d/core/types.h line:298
	m.def("dist_point_triangle", (float (*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, class easy3d::Vec<3, float> &)) &easy3d::geom::dist_point_triangle, "Computes the distance of a point p to the triangle given by vec3s (v0, v1, v2). \n\nC++: easy3d::geom::dist_point_triangle(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, class easy3d::Vec<3, float> &) --> float", pybind11::arg("p"), pybind11::arg("v0"), pybind11::arg("v1"), pybind11::arg("v2"), pybind11::arg("nearest_point"));

	// easy3d::geom::tetra_circum_center(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) file:easy3d/core/types.h line:301
	m.def("tetra_circum_center", (class easy3d::Vec<3, float> (*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &)) &easy3d::geom::tetra_circum_center, "Computes the circum center of a tetrahedron. \n\nC++: easy3d::geom::tetra_circum_center(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) --> class easy3d::Vec<3, float>", pybind11::arg("p"), pybind11::arg("q"), pybind11::arg("r"), pybind11::arg("s"));

    // easy3d::color::encode(int, int, int) file:easy3d/core/types.h line:311
    m.def("encode", (int (*)(int, int, int)) &easy3d::color::encode, "Encodes an RGB (each component in the range [0, 255]) color in an integer value.\n\nC++: easy3d::color::encode(int, int, int) --> int", pybind11::arg("r"), pybind11::arg("g"), pybind11::arg("b"));

    // easy3d::color::encode(int, int, int, int) file:easy3d/core/types.h line:316
    m.def("encode", (int (*)(int, int, int, int)) &easy3d::color::encode, "Encodes an RGBA (each component in the range [0, 255]) color in an integer value.\n\nC++: easy3d::color::encode(int, int, int, int) --> int", pybind11::arg("r"), pybind11::arg("g"), pybind11::arg("b"), pybind11::arg("a"));

    // easy3d::color::decode(int, int &, int &, int &) file:easy3d/core/types.h line:323
    m.def("decode", (void (*)(int, int &, int &, int &)) &easy3d::color::decode, "Decodes an integer value as RGB color (each component in the range [0, 255]).\n\nC++: easy3d::color::decode(int, int &, int &, int &) --> void", pybind11::arg("value"), pybind11::arg("r"), pybind11::arg("g"), pybind11::arg("b"));

    // easy3d::color::decode(int, int &, int &, int &, int &) file:easy3d/core/types.h line:330
    m.def("decode", (void (*)(int, int &, int &, int &, int &)) &easy3d::color::decode, "Decodes an integer value as RGBA color (each component in the range [0, 255]).\n\nC++: easy3d::color::decode(int, int &, int &, int &, int &) --> void", pybind11::arg("value"), pybind11::arg("r"), pybind11::arg("g"), pybind11::arg("b"), pybind11::arg("a"));

    // easy3d::color::red(int) file:easy3d/core/types.h line:341
    m.def("red", (int (*)(int)) &easy3d::color::red, "Gets the red component of RGB. [0, 255]\n\nC++: easy3d::color::red(int) --> int", pybind11::arg("color"));

    // easy3d::color::green(int) file:easy3d/core/types.h line:344
    m.def("green", (int (*)(int)) &easy3d::color::green, "Gets the green component of RGB. [0, 255]\n\nC++: easy3d::color::green(int) --> int", pybind11::arg("color"));

    // easy3d::color::blue(int) file:easy3d/core/types.h line:347
    m.def("blue", (int (*)(int)) &easy3d::color::blue, "Gets the blue component of RGB. [0, 255]\n\nC++: easy3d::color::blue(int) --> int", pybind11::arg("color"));

    // easy3d::color::alpha(int) file:easy3d/core/types.h line:350
    m.def("alpha", (int (*)(int)) &easy3d::color::alpha, "Gets the alpha component of RGBA. [0, 255]\n\nC++: easy3d::color::alpha(int) --> int", pybind11::arg("color"));

    // easy3d::next_pow2(int) file:easy3d/core/types.h line:143
    m.def("next_pow2", (int (*)(int)) &easy3d::next_pow2, "Calculates the next larger power of 2. If the input is already a power of 2, it will return itself.\n \n\n The starting point for finding the next power of 2.\n \n\n value^2.\n Example:\n      next_pow2(50);  // returns 64\n      next_pow2(64);  // returns 64\n      next_pow2(401); // returns 512\n\nC++: easy3d::next_pow2(int) --> int", pybind11::arg("a"));
}
