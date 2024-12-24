#include <easy3d/core/mat.h>
#include <easy3d/core/quat.h>
#include <easy3d/core/vec.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/transform_decompose.h>

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

void bind_easy3d_renderer_transform(pybind11::module_& m)
{
	// easy3d::transform::ortho(float, float, float, float, float, float) file:easy3d/renderer/transform.h line:51
	M("easy3d::transform").def("ortho", (class easy3d::Mat4<float> (*)(float, float, float, float, float, float)) &easy3d::transform::ortho, "Creates a matrix for an orthographic parallel viewing volume. Simulating glFrustum().\n \n\n Specifies the distance from the viewer to the near clipping plane (always positive).\n \n\n  Specifies the distance from the viewer to the far clipping plane (always positive).\n See http://www.songho.ca/opengl/gl_projectionmatrix.html\n     https://ksimek.github.io/2013/06/03/calibrated_cameras_in_opengl/\n\nC++: easy3d::transform::ortho(float, float, float, float, float, float) --> class easy3d::Mat4<float>", pybind11::arg("left"), pybind11::arg("right"), pybind11::arg("bottom"), pybind11::arg("top"), pybind11::arg("near"), pybind11::arg("far"));

	// easy3d::transform::ortho(float, float, float, float) file:easy3d/renderer/transform.h line:54
	M("easy3d::transform").def("ortho", (class easy3d::Mat4<float> (*)(float, float, float, float)) &easy3d::transform::ortho, "Creates a matrix for projecting two-dimensional coordinates onto the screen.\n\nC++: easy3d::transform::ortho(float, float, float, float) --> class easy3d::Mat4<float>", pybind11::arg("left"), pybind11::arg("right"), pybind11::arg("bottom"), pybind11::arg("top"));

	// easy3d::transform::frustum(float, float, float, float, float, float) file:easy3d/renderer/transform.h line:61
	M("easy3d::transform").def("frustum", (class easy3d::Mat4<float> (*)(float, float, float, float, float, float)) &easy3d::transform::frustum, "Creates a frustum perspective matrix. Simulating glFrustum().\n See http://www.songho.ca/opengl/gl_projectionmatrix.html\n 	   https://ksimek.github.io/2013/06/03/calibrated_cameras_in_opengl/\n\nC++: easy3d::transform::frustum(float, float, float, float, float, float) --> class easy3d::Mat4<float>", pybind11::arg("left"), pybind11::arg("right"), pybind11::arg("bottom"), pybind11::arg("top"), pybind11::arg("near"), pybind11::arg("far"));

	// easy3d::transform::perspective(float, float, float, float) file:easy3d/renderer/transform.h line:73
	M("easy3d::transform").def("perspective", (class easy3d::Mat4<float> (*)(float, float, float, float)) &easy3d::transform::perspective, "Creates a matrix for a right-handed symmetric perspective-view frustum. Simulating gluPerspective().\n \n\n  Specifies the field of view angle, in the y direction. Expressed in radians.\n \n\n Specifies the aspect ratio that determines the field of view in the x direction. The aspect\n      ratio is the ratio of x (width) to y (height).\n \n\n   Specifies the distance from the viewer to the near clipping plane (always positive).\n \n\n    Specifies the distance from the viewer to the far clipping plane (always positive).\n \n\n Degrees are an unhandy unit to work with. Thus, I use radians for everything!\n See https://ksimek.github.io/2013/06/18/calibrated-cameras-and-gluperspective/\n\nC++: easy3d::transform::perspective(float, float, float, float) --> class easy3d::Mat4<float>", pybind11::arg("fov_y"), pybind11::arg("aspect"), pybind11::arg("near"), pybind11::arg("far"));

	// easy3d::transform::perspective(float, float, float, float, float) file:easy3d/renderer/transform.h line:74
	M("easy3d::transform").def("perspective", (class easy3d::Mat4<float> (*)(float, float, float, float, float)) &easy3d::transform::perspective, "C++: easy3d::transform::perspective(float, float, float, float, float) --> class easy3d::Mat4<float>", pybind11::arg("fov_y"), pybind11::arg("width"), pybind11::arg("height"), pybind11::arg("near"), pybind11::arg("far"));

	// easy3d::transform::infinite_perspective(float, float, float) file:easy3d/renderer/transform.h line:83
	M("easy3d::transform").def("infinite_perspective", (class easy3d::Mat4<float> (*)(float, float, float)) &easy3d::transform::infinite_perspective, "Creates a matrix for a symmetric perspective-view frustum with far plane at infinite.\n \n\n  Specifies the field of view angle, in the y direction. Expressed in radians.\n \n\n Specifies the aspect ratio that determines the field of view in the x direction. The\n		   aspect ratio is the ratio of x (width) to y (height).\n \n\n   Specifies the distance from the viewer to the near clipping plane (always positive).\n\nC++: easy3d::transform::infinite_perspective(float, float, float) --> class easy3d::Mat4<float>", pybind11::arg("fov_y"), pybind11::arg("aspect"), pybind11::arg("near"));

	// easy3d::transform::viewport(float, float) file:easy3d/renderer/transform.h line:86
	M("easy3d::transform").def("viewport", (class easy3d::Mat4<float> (*)(float, float)) &easy3d::transform::viewport, "Creates a viewport matrix. Simulating glViewport().\n\nC++: easy3d::transform::viewport(float, float) --> class easy3d::Mat4<float>", pybind11::arg("width"), pybind11::arg("height"));

	// easy3d::transform::look_at(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) file:easy3d/renderer/transform.h line:94
	M("easy3d::transform").def("look_at", (class easy3d::Mat4<float> (*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &)) &easy3d::transform::look_at, "Builds a look at view matrix simulating gluLookAt().\n \n\n    Position of the camera.\n \n\n Position where the camera is looking at.\n \n\n     Normalized up vector determining how the camera is oriented. Typically (0, 0, 1).\n\nC++: easy3d::transform::look_at(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) --> class easy3d::Mat4<float>", pybind11::arg("eye"), pybind11::arg("center"), pybind11::arg("up"));

	// easy3d::transform::pick_matrix(const class easy3d::Vec<2, float> &, const class easy3d::Vec<2, float> &, const class easy3d::Vec<4, float> &) file:easy3d/renderer/transform.h line:97
	M("easy3d::transform").def("pick_matrix", (class easy3d::Mat4<float> (*)(const class easy3d::Vec<2, float> &, const class easy3d::Vec<2, float> &, const class easy3d::Vec<4, float> &)) &easy3d::transform::pick_matrix, "Defines a picking region\n\nC++: easy3d::transform::pick_matrix(const class easy3d::Vec<2, float> &, const class easy3d::Vec<2, float> &, const class easy3d::Vec<4, float> &) --> class easy3d::Mat4<float>", pybind11::arg("center"), pybind11::arg("delta"), pybind11::arg("viewport"));

	// easy3d::transform::normal_matrix(const class easy3d::Mat4<float> &) file:easy3d/renderer/transform.h line:137
	M("easy3d::transform").def("normal_matrix", (class easy3d::Mat3<float> (*)(const class easy3d::Mat4<float> &)) &easy3d::transform::normal_matrix, "Computes the normal matrix based on mat.\n \n\n The returned matrix is NOT padded. Use the padded version for uniform blocks.\n\nC++: easy3d::transform::normal_matrix(const class easy3d::Mat4<float> &) --> class easy3d::Mat3<float>", pybind11::arg("mat"));

	// easy3d::transform::decompose(const class easy3d::Mat4<float> &, class easy3d::Vec<3, float> &, class easy3d::Mat3<float> &, class easy3d::Vec<3, float> &) file:easy3d/renderer/transform_decompose.h line:69
	M("easy3d::transform").def("decompose", (void (*)(const class easy3d::Mat4<float> &, class easy3d::Vec<3, float> &, class easy3d::Mat3<float> &, class easy3d::Vec<3, float> &)) &easy3d::transform::decompose, "Decomposes a transformation matrix (M = translation * rotation * scaling) into its original components.\n  \n\n is the input transformation matrix\n  \n\n receives the output scaling for the x, y, z axes\n  \n\n receives the output rotation\n  \n\n receives the output translation for the x, y, z axes\n \n\n This function cannot handle skew and perspective transformation. See the overloaded function below.\n \n\n Add functions that extract single components, i.e.,\n      - Quat extract_rotation(const mat4& M);\n      - vec3 extract_scale(const mat4& M);\n      - vec3 extract_translation(const mat4& M);\n\nC++: easy3d::transform::decompose(const class easy3d::Mat4<float> &, class easy3d::Vec<3, float> &, class easy3d::Mat3<float> &, class easy3d::Vec<3, float> &) --> void", pybind11::arg("M"), pybind11::arg("scaling"), pybind11::arg("rotation"), pybind11::arg("translation"));

	// easy3d::transform::decompose(const class easy3d::Mat4<float> &, class easy3d::Vec<3, float> &, class easy3d::Quat<float> &, class easy3d::Vec<3, float> &) file:easy3d/renderer/transform_decompose.h line:70
	M("easy3d::transform").def("decompose", (void (*)(const class easy3d::Mat4<float> &, class easy3d::Vec<3, float> &, class easy3d::Quat<float> &, class easy3d::Vec<3, float> &)) &easy3d::transform::decompose, "C++: easy3d::transform::decompose(const class easy3d::Mat4<float> &, class easy3d::Vec<3, float> &, class easy3d::Quat<float> &, class easy3d::Vec<3, float> &) --> void", pybind11::arg("M"), pybind11::arg("scaling"), pybind11::arg("rotation"), pybind11::arg("translation"));

	// easy3d::transform::decompose_no_scaling(const class easy3d::Mat4<float> &, class easy3d::Mat3<float> &, class easy3d::Vec<3, float> &) file:easy3d/renderer/transform_decompose.h line:81
	M("easy3d::transform").def("decompose_no_scaling", (void (*)(const class easy3d::Mat4<float> &, class easy3d::Mat3<float> &, class easy3d::Vec<3, float> &)) &easy3d::transform::decompose_no_scaling, "Decomposes a transformation matrix without scaling (M = translation * rotation) into its original components.\n  \n\n receives the output rotation\n  \n\n receives the output translation for the x, y, z axes\n \n\n This function cannot handle rotation, skew, and perspective transformation. See the overloaded function below.\n \n\n Add functions that extract single components, i.e.,\n      - Quat  extract_rotation(const mat4& M);\n      - vec3  extract_translation(const mat4& M);\n\nC++: easy3d::transform::decompose_no_scaling(const class easy3d::Mat4<float> &, class easy3d::Mat3<float> &, class easy3d::Vec<3, float> &) --> void", pybind11::arg("M"), pybind11::arg("rotation"), pybind11::arg("translation"));

	// easy3d::transform::decompose_no_scaling(const class easy3d::Mat4<float> &, class easy3d::Quat<float> &, class easy3d::Vec<3, float> &) file:easy3d/renderer/transform_decompose.h line:82
	M("easy3d::transform").def("decompose_no_scaling", (void (*)(const class easy3d::Mat4<float> &, class easy3d::Quat<float> &, class easy3d::Vec<3, float> &)) &easy3d::transform::decompose_no_scaling, "C++: easy3d::transform::decompose_no_scaling(const class easy3d::Mat4<float> &, class easy3d::Quat<float> &, class easy3d::Vec<3, float> &) --> void", pybind11::arg("M"), pybind11::arg("rotation"), pybind11::arg("translation"));

	// easy3d::transform::decompose(const class easy3d::Mat4<float> &, class easy3d::Vec<3, float> &, class easy3d::Quat<float> &, class easy3d::Vec<3, float> &, class easy3d::Vec<3, float> &, class easy3d::Vec<4, float> &) file:easy3d/renderer/transform_decompose.h line:89
	M("easy3d::transform").def("decompose", (bool (*)(const class easy3d::Mat4<float> &, class easy3d::Vec<3, float> &, class easy3d::Quat<float> &, class easy3d::Vec<3, float> &, class easy3d::Vec<3, float> &, class easy3d::Vec<4, float> &)) &easy3d::transform::decompose, "Decomposes a transformation matrix into to its original components (i.e., scaling, rotation,\n      translation, skew and perspective).\n \n\n Not tested yet.\n\nC++: easy3d::transform::decompose(const class easy3d::Mat4<float> &, class easy3d::Vec<3, float> &, class easy3d::Quat<float> &, class easy3d::Vec<3, float> &, class easy3d::Vec<3, float> &, class easy3d::Vec<4, float> &) --> bool", pybind11::arg("M"), pybind11::arg("scaling"), pybind11::arg("rotation"), pybind11::arg("translation"), pybind11::arg("skew"), pybind11::arg("persp"));

}
