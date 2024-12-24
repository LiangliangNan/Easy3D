#include <easy3d/core/mat.h>
#include <easy3d/core/vec.h>
#include <easy3d/renderer/drawable.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
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

// easy3d::LinesDrawable file:easy3d/renderer/drawable_lines.h line:40
struct PyCallBack_easy3d_LinesDrawable : public easy3d::LinesDrawable {
	using easy3d::LinesDrawable::LinesDrawable;

	enum easy3d::Drawable::Type type() const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::LinesDrawable *>(this), "type");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<enum easy3d::Drawable::Type>::value) {
				static pybind11::detail::override_caster_t<enum easy3d::Drawable::Type> caster;
				return pybind11::detail::cast_ref<enum easy3d::Drawable::Type>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<enum easy3d::Drawable::Type>(std::move(o));
		}
		return LinesDrawable::type();
	}
	void update_buffers_internal() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::LinesDrawable *>(this), "update_buffers_internal");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Drawable::update_buffers_internal();
	}
};

// easy3d::PointsDrawable file:easy3d/renderer/drawable_points.h line:42
struct PyCallBack_easy3d_PointsDrawable : public easy3d::PointsDrawable {
	using easy3d::PointsDrawable::PointsDrawable;

	enum easy3d::Drawable::Type type() const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PointsDrawable *>(this), "type");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<enum easy3d::Drawable::Type>::value) {
				static pybind11::detail::override_caster_t<enum easy3d::Drawable::Type> caster;
				return pybind11::detail::cast_ref<enum easy3d::Drawable::Type>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<enum easy3d::Drawable::Type>(std::move(o));
		}
		return PointsDrawable::type();
	}
	void update_buffers_internal() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::PointsDrawable *>(this), "update_buffers_internal");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Drawable::update_buffers_internal();
	}
};

// easy3d::TrianglesDrawable file:easy3d/renderer/drawable_triangles.h line:46
struct PyCallBack_easy3d_TrianglesDrawable : public easy3d::TrianglesDrawable {
	using easy3d::TrianglesDrawable::TrianglesDrawable;

	enum easy3d::Drawable::Type type() const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::TrianglesDrawable *>(this), "type");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<enum easy3d::Drawable::Type>::value) {
				static pybind11::detail::override_caster_t<enum easy3d::Drawable::Type> caster;
				return pybind11::detail::cast_ref<enum easy3d::Drawable::Type>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<enum easy3d::Drawable::Type>(std::move(o));
		}
		return TrianglesDrawable::type();
	}
	void update_buffers_internal() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::TrianglesDrawable *>(this), "update_buffers_internal");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Drawable::update_buffers_internal();
	}
};

void bind_easy3d_renderer_drawable_lines(pybind11::module_& m)
{
	{ // easy3d::LinesDrawable file:easy3d/renderer/drawable_lines.h line:40
		pybind11::class_<easy3d::LinesDrawable, std::shared_ptr<easy3d::LinesDrawable>, PyCallBack_easy3d_LinesDrawable, easy3d::Drawable> cl(m, "LinesDrawable", "The drawable for rendering a set of line segments, e.g., edges of a mesh, vector fields\n \n\n\n \n PointsDrawable, TrianglesDrawable");
		cl.def( pybind11::init( [](PyCallBack_easy3d_LinesDrawable const &o){ return new PyCallBack_easy3d_LinesDrawable(o); } ) );
		cl.def( pybind11::init( [](easy3d::LinesDrawable const &o){ return new easy3d::LinesDrawable(o); } ) );

		pybind11::enum_<easy3d::LinesDrawable::ImposterType>(cl, "ImposterType", pybind11::arithmetic(), "")
			.value("PLAIN", easy3d::LinesDrawable::PLAIN)
			.value("CYLINDER", easy3d::LinesDrawable::CYLINDER)
			.value("CONE", easy3d::LinesDrawable::CONE)
			.export_values();

		cl.def("type", (enum easy3d::Drawable::Type (easy3d::LinesDrawable::*)() const) &easy3d::LinesDrawable::type, "C++: easy3d::LinesDrawable::type() const --> enum easy3d::Drawable::Type");
		cl.def("impostor_type", (enum easy3d::LinesDrawable::ImposterType (easy3d::LinesDrawable::*)() const) &easy3d::LinesDrawable::impostor_type, "C++: easy3d::LinesDrawable::impostor_type() const --> enum easy3d::LinesDrawable::ImposterType");
		cl.def("set_impostor_type", (void (easy3d::LinesDrawable::*)(enum easy3d::LinesDrawable::ImposterType)) &easy3d::LinesDrawable::set_impostor_type, "C++: easy3d::LinesDrawable::set_impostor_type(enum easy3d::LinesDrawable::ImposterType) --> void", pybind11::arg("t"));
		cl.def("line_width", (float (easy3d::LinesDrawable::*)() const) &easy3d::LinesDrawable::line_width, "C++: easy3d::LinesDrawable::line_width() const --> float");
		cl.def("set_line_width", (void (easy3d::LinesDrawable::*)(float)) &easy3d::LinesDrawable::set_line_width, "C++: easy3d::LinesDrawable::set_line_width(float) --> void", pybind11::arg("w"));
		cl.def("assign", (class easy3d::LinesDrawable & (easy3d::LinesDrawable::*)(const class easy3d::LinesDrawable &)) &easy3d::LinesDrawable::operator=, "C++: easy3d::LinesDrawable::operator=(const class easy3d::LinesDrawable &) --> class easy3d::LinesDrawable &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
	{ // easy3d::PointsDrawable file:easy3d/renderer/drawable_points.h line:42
		pybind11::class_<easy3d::PointsDrawable, std::shared_ptr<easy3d::PointsDrawable>, PyCallBack_easy3d_PointsDrawable, easy3d::Drawable> cl(m, "PointsDrawable", "The drawable for rendering a set of points, e.g., point clouds, vertices of a mesh.\n \n\n\n \n LinesDrawable, TrianglesDrawable");
		cl.def( pybind11::init( [](PyCallBack_easy3d_PointsDrawable const &o){ return new PyCallBack_easy3d_PointsDrawable(o); } ) );
		cl.def( pybind11::init( [](easy3d::PointsDrawable const &o){ return new easy3d::PointsDrawable(o); } ) );

		pybind11::enum_<easy3d::PointsDrawable::ImposterType>(cl, "ImposterType", pybind11::arithmetic(), "")
			.value("PLAIN", easy3d::PointsDrawable::PLAIN)
			.value("SPHERE", easy3d::PointsDrawable::SPHERE)
			.value("SURFEL", easy3d::PointsDrawable::SURFEL)
			.export_values();

		cl.def("type", (enum easy3d::Drawable::Type (easy3d::PointsDrawable::*)() const) &easy3d::PointsDrawable::type, "C++: easy3d::PointsDrawable::type() const --> enum easy3d::Drawable::Type");
		cl.def("impostor_type", (enum easy3d::PointsDrawable::ImposterType (easy3d::PointsDrawable::*)() const) &easy3d::PointsDrawable::impostor_type, "Get/Set impostor type. \n\nC++: easy3d::PointsDrawable::impostor_type() const --> enum easy3d::PointsDrawable::ImposterType");
		cl.def("set_impostor_type", (void (easy3d::PointsDrawable::*)(enum easy3d::PointsDrawable::ImposterType)) &easy3d::PointsDrawable::set_impostor_type, "C++: easy3d::PointsDrawable::set_impostor_type(enum easy3d::PointsDrawable::ImposterType) --> void", pybind11::arg("t"));
		cl.def("point_size", (float (easy3d::PointsDrawable::*)() const) &easy3d::PointsDrawable::point_size, "Get/Set point size. \n\nC++: easy3d::PointsDrawable::point_size() const --> float");
		cl.def("set_point_size", (void (easy3d::PointsDrawable::*)(float)) &easy3d::PointsDrawable::set_point_size, "C++: easy3d::PointsDrawable::set_point_size(float) --> void", pybind11::arg("s"));
		cl.def("assign", (class easy3d::PointsDrawable & (easy3d::PointsDrawable::*)(const class easy3d::PointsDrawable &)) &easy3d::PointsDrawable::operator=, "C++: easy3d::PointsDrawable::operator=(const class easy3d::PointsDrawable &) --> class easy3d::PointsDrawable &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
	{ // easy3d::TrianglesDrawable file:easy3d/renderer/drawable_triangles.h line:46
		pybind11::class_<easy3d::TrianglesDrawable, std::shared_ptr<easy3d::TrianglesDrawable>, PyCallBack_easy3d_TrianglesDrawable, easy3d::Drawable> cl(m, "TrianglesDrawable", "The drawable for rendering a set of triangles, e.g., the surface of a triangular mesh.\n \n\n\n \n LinesDrawable, PointsDrawable\n\n \n TrianglesDrawable supports triangles only. Visualizing general polygons typically requires tessellating\n the faces into a set of triangles (using Tessellator or any other methods). Vertex coordinates and properties\n (e.g., color, normal) must be provided as consecutive triplets in an array to be transferred to GPU.\n See Drawable::update_vertex_buffer().");
		cl.def( pybind11::init( [](PyCallBack_easy3d_TrianglesDrawable const &o){ return new PyCallBack_easy3d_TrianglesDrawable(o); } ) );
		cl.def( pybind11::init( [](easy3d::TrianglesDrawable const &o){ return new easy3d::TrianglesDrawable(o); } ) );
		cl.def("type", (enum easy3d::Drawable::Type (easy3d::TrianglesDrawable::*)() const) &easy3d::TrianglesDrawable::type, "C++: easy3d::TrianglesDrawable::type() const --> enum easy3d::Drawable::Type");
		cl.def("smooth_shading", (bool (easy3d::TrianglesDrawable::*)() const) &easy3d::TrianglesDrawable::smooth_shading, "C++: easy3d::TrianglesDrawable::smooth_shading() const --> bool");
		cl.def("set_smooth_shading", (void (easy3d::TrianglesDrawable::*)(bool)) &easy3d::TrianglesDrawable::set_smooth_shading, "C++: easy3d::TrianglesDrawable::set_smooth_shading(bool) --> void", pybind11::arg("b"));
		cl.def("opacity", (float (easy3d::TrianglesDrawable::*)() const) &easy3d::TrianglesDrawable::opacity, "Query the opacity of the drawable, in the range [0.0, 1.0].\n \n\n The opacity of the drawable.\n \n\n Opacity is effective only when transparency (e.g., DualDepthPeeling, AverageColorBlending) rendering is enabled.\n\nC++: easy3d::TrianglesDrawable::opacity() const --> float");
		cl.def("set_opacity", (void (easy3d::TrianglesDrawable::*)(float)) &easy3d::TrianglesDrawable::set_opacity, "Set the opacity of the drawable.\n \n\n The new opacity value, in the range [0.0, 1.0].\n \n\n Opacity is effective only when transparency (e.g., DualDepthPeeling, AverageColorBlending) rendering is enabled.\n\nC++: easy3d::TrianglesDrawable::set_opacity(float) --> void", pybind11::arg("opacity"));
		cl.def("assign", (class easy3d::TrianglesDrawable & (easy3d::TrianglesDrawable::*)(const class easy3d::TrianglesDrawable &)) &easy3d::TrianglesDrawable::operator=, "C++: easy3d::TrianglesDrawable::operator=(const class easy3d::TrianglesDrawable &) --> class easy3d::TrianglesDrawable &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}

}
