#include <easy3d/core/mat.h>
#include <easy3d/core/vec.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/vertex_array_object.h>

#include <functional>
#include <pybind11/pybind11.h>
#include <string>
#include <pybind11/stl.h>
#include <pybind11/numpy.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif


// easy3d::Drawable file:easy3d/renderer/drawable.h line:56
struct PyCallBack_easy3d_Drawable : public easy3d::Drawable {
    using easy3d::Drawable::Drawable;

    enum easy3d::Drawable::Type type() const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Drawable *>(this), "type");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>();
            if (pybind11::detail::cast_is_temporary_value_reference<enum easy3d::Drawable::Type>::value) {
                static pybind11::detail::override_caster_t<enum easy3d::Drawable::Type> caster;
                return pybind11::detail::cast_ref<enum easy3d::Drawable::Type>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<enum easy3d::Drawable::Type>(std::move(o));
        }
        pybind11::pybind11_fail("Tried to call pure virtual function \"Drawable::type\"");
    }
    void draw(const class easy3d::Camera * a0) const override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Drawable *>(this), "draw");
        if (overload) {
            auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
            if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
                static pybind11::detail::override_caster_t<void> caster;
                return pybind11::detail::cast_ref<void>(std::move(o), caster);
            }
            return pybind11::detail::cast_safe<void>(std::move(o));
        }
        pybind11::pybind11_fail("Tried to call pure virtual function \"Drawable::draw\"");
    }
    void update_buffers_internal() override {
        pybind11::gil_scoped_acquire gil;
        pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Drawable *>(this), "update_buffers_internal");
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


void bind_easy3d_renderer_drawables(pybind11::module_& m)
{
    { // easy3d::Drawable file:easy3d/renderer/drawable.h line:56
        pybind11::class_<easy3d::Drawable, std::shared_ptr<easy3d::Drawable>, PyCallBack_easy3d_Drawable, easy3d::State> cl(m, "Drawable", "The base class for drawable objects. A drawable represent a set of points, line segments, or triangles.\n \n\n\n \n A Drawable is an abstraction for \"something that can be drawn\", e.g., a point cloud, the surface\n          of a mesh, the wireframe of a surface mesh, the vertices of a graph, the border of a polyhedral mesh.\n          A drawable manages its rendering status and controls the upload of the data to the GPU.\n          A drawable can live independently or be associated with a Model.\n          The rendering states of a drawable can be accessed/modified through the functions provided in its\n          parent class State.\n \n\n State, Renderer");
        cl.def( pybind11::init( [](){ return new PyCallBack_easy3d_Drawable(); } ), "doc");
        cl.def( pybind11::init( [](const std::string & a0){ return new PyCallBack_easy3d_Drawable(a0); } ), "doc");
        cl.def( pybind11::init<const std::string &, class easy3d::Model *>(), pybind11::arg("name"), pybind11::arg("model") );

        cl.def(pybind11::init<PyCallBack_easy3d_Drawable const &>());

        pybind11::enum_<easy3d::Drawable::Type>(cl, "Type", pybind11::arithmetic(), "")
                .value("DT_POINTS", easy3d::Drawable::DT_POINTS)
                .value("DT_LINES", easy3d::Drawable::DT_LINES)
                .value("DT_TRIANGLES", easy3d::Drawable::DT_TRIANGLES)
                .export_values();

        cl.def("type", (enum easy3d::Drawable::Type (easy3d::Drawable::*)() const) &easy3d::Drawable::type, "Returns the type of the drawable.\n\nC++: easy3d::Drawable::type() const --> enum easy3d::Drawable::Type");
        cl.def("name", (const std::string & (easy3d::Drawable::*)() const) &easy3d::Drawable::name, "C++: easy3d::Drawable::name() const --> const std::string &", pybind11::return_value_policy::automatic);
        cl.def("set_name", (void (easy3d::Drawable::*)(const std::string &)) &easy3d::Drawable::set_name, "C++: easy3d::Drawable::set_name(const std::string &) --> void", pybind11::arg("n"));
        cl.def("model", (class easy3d::Model * (easy3d::Drawable::*)()) &easy3d::Drawable::model, "the model to which the drawable is attached to (can be NULL).\n\nC++: easy3d::Drawable::model() --> class easy3d::Model *", pybind11::return_value_policy::automatic);
        cl.def("set_model", (void (easy3d::Drawable::*)(class easy3d::Model *)) &easy3d::Drawable::set_model, "C++: easy3d::Drawable::set_model(class easy3d::Model *) --> void", pybind11::arg("m"));
        cl.def("bounding_box", (const class easy3d::GenericBox<3, float> & (easy3d::Drawable::*)() const) &easy3d::Drawable::bounding_box, "C++: easy3d::Drawable::bounding_box() const --> const class easy3d::GenericBox<3, float> &", pybind11::return_value_policy::automatic);
        cl.def("state", (class easy3d::State & (easy3d::Drawable::*)()) &easy3d::Drawable::state, "C++: easy3d::Drawable::state() --> class easy3d::State &", pybind11::return_value_policy::automatic);
        cl.def("set_state", (void (easy3d::Drawable::*)(const class easy3d::State &)) &easy3d::Drawable::set_state, "C++: easy3d::Drawable::set_state(const class easy3d::State &) --> void", pybind11::arg("s"));
        cl.def("buffer_stats", (void (easy3d::Drawable::*)(std::ostream &) const) &easy3d::Drawable::buffer_stats, "print statistics (e.g., num vertices, memory usage) of the buffers to an output stream (e.g., std::cout).\n\nC++: easy3d::Drawable::buffer_stats(std::ostream &) const --> void", pybind11::arg("output"));
        cl.def("vertex_buffer", (unsigned int (easy3d::Drawable::*)() const) &easy3d::Drawable::vertex_buffer, "@{\n\nC++: easy3d::Drawable::vertex_buffer() const --> unsigned int");
        cl.def("color_buffer", (unsigned int (easy3d::Drawable::*)() const) &easy3d::Drawable::color_buffer, "C++: easy3d::Drawable::color_buffer() const --> unsigned int");
        cl.def("normal_buffer", (unsigned int (easy3d::Drawable::*)() const) &easy3d::Drawable::normal_buffer, "C++: easy3d::Drawable::normal_buffer() const --> unsigned int");
        cl.def("texcoord_buffer", (unsigned int (easy3d::Drawable::*)() const) &easy3d::Drawable::texcoord_buffer, "C++: easy3d::Drawable::texcoord_buffer() const --> unsigned int");
        cl.def("element_buffer", (unsigned int (easy3d::Drawable::*)() const) &easy3d::Drawable::element_buffer, "C++: easy3d::Drawable::element_buffer() const --> unsigned int");

        // Liangliang: In C++, the compiler can differentiate between function overloads based on the argument types at
        //      compile time. However, in Python, the type checking is dynamic, so pybind11 uses the order in which
        //      functions are registered to decide which overload to call.
        //      Always define the most specific overloads before the more general ones. General overloads like
        //      pybind11::object or pybind11::list should come last to avoid overshadowing specific ones.
        cl.def("update_vertex_buffer",
               [](easy3d::Drawable &self, const class std::vector<class easy3d::Vec<3, float> > &vertices, bool dynamic = false) -> void {
                   // Directly update vertex buffer with a std::vector<easy3d::vec3>
                   self.update_vertex_buffer(vertices, dynamic);
               },
               "Update vertex buffer with a std::vector of easy3d::vec3.",
               pybind11::arg("vertices"),
               pybind11::arg("dynamic") = false);

        // from a list of tuples (points) or a NumPy array.
        cl.def("update_vertex_buffer",
               [](easy3d::Drawable &self, pybind11::object points, bool dynamic = false) -> void {
                   // Handle list of tuples or numpy array
                   if (pybind11::isinstance<pybind11::list>(points)) {
                       std::vector<easy3d::vec3> vertices;
                       for (auto item : points.cast<pybind11::list>()) {
                           auto tuple = item.cast<pybind11::tuple>();
                           if (tuple.size() != 3) {
                               throw std::invalid_argument("Each point must have 3 coordinates.");
                           }
                           vertices.emplace_back(
                                   tuple[0].cast<float>(),
                                   tuple[1].cast<float>(),
                                   tuple[2].cast<float>());
                       }
                       self.update_vertex_buffer(vertices, dynamic);
                   } else if (pybind11::isinstance<pybind11::array_t<float>>(points)) {  // Handle numpy array of floats
                       auto buf = points.cast<pybind11::array_t<float>>().unchecked<2>();
                       if (buf.ndim() != 2 || buf.shape(1) != 3) {
                           throw std::invalid_argument("Input array must have shape (n, 3).");
                       }
                       std::vector<easy3d::vec3> vertices(buf.shape(0));
                       for (ssize_t i = 0; i < buf.shape(0); ++i)
                           vertices[i] = easy3d::vec3(buf(i, 0), buf(i, 1), buf(i, 2));
                       self.update_vertex_buffer(vertices, dynamic);
                   } else if (pybind11::isinstance<pybind11::array_t<double>>(points)) {  // Handle numpy array of doubles
                       auto buf = points.cast<pybind11::array_t<double>>().unchecked<2>();
                       if (buf.ndim() != 2 || buf.shape(1) != 3) {
                           throw std::invalid_argument("Input array must have shape (n, 3).");
                       }
                       std::vector<easy3d::vec3> vertices(buf.shape(0));
                       for (ssize_t i = 0; i < buf.shape(0); ++i)
                           vertices[i] = easy3d::vec3(
                                   static_cast<float>(buf(i, 0)),
                                   static_cast<float>(buf(i, 1)),
                                   static_cast<float>(buf(i, 2)));
                       self.update_vertex_buffer(vertices, dynamic);
                   } else {
                       throw std::invalid_argument("Input must be a std::vector<easy3d::vec3>, a list of tuples, or a NumPy array (float32 or float64).");
                   }
               },
               "Update vertex buffer with a list of tuples, NumPy array, or std::vector of easy3d::vec3.",
               pybind11::arg("points"),
               pybind11::arg("dynamic") = false);

        cl.def("update_color_buffer", [](easy3d::Drawable &o, const class std::vector<class easy3d::Vec<3, float> > & a0) -> void { return o.update_color_buffer(a0); }, "", pybind11::arg("colors"));
        cl.def("update_color_buffer", (void (easy3d::Drawable::*)(const class std::vector<class easy3d::Vec<3, float> > &, bool)) &easy3d::Drawable::update_color_buffer, "C++: easy3d::Drawable::update_color_buffer(const class std::vector<class easy3d::Vec<3, float> > &, bool) --> void", pybind11::arg("colors"), pybind11::arg("dynamic"));
        cl.def("update_normal_buffer", [](easy3d::Drawable &o, const class std::vector<class easy3d::Vec<3, float> > & a0) -> void { return o.update_normal_buffer(a0); }, "", pybind11::arg("normals"));
        cl.def("update_normal_buffer", (void (easy3d::Drawable::*)(const class std::vector<class easy3d::Vec<3, float> > &, bool)) &easy3d::Drawable::update_normal_buffer, "C++: easy3d::Drawable::update_normal_buffer(const class std::vector<class easy3d::Vec<3, float> > &, bool) --> void", pybind11::arg("normals"), pybind11::arg("dynamic"));
        cl.def("update_texcoord_buffer", [](easy3d::Drawable &o, const class std::vector<class easy3d::Vec<2, float> > & a0) -> void { return o.update_texcoord_buffer(a0); }, "", pybind11::arg("texcoords"));
        cl.def("update_texcoord_buffer", (void (easy3d::Drawable::*)(const class std::vector<class easy3d::Vec<2, float> > &, bool)) &easy3d::Drawable::update_texcoord_buffer, "C++: easy3d::Drawable::update_texcoord_buffer(const class std::vector<class easy3d::Vec<2, float> > &, bool) --> void", pybind11::arg("texcoords"), pybind11::arg("dynamic"));
        cl.def("update_element_buffer", (void (easy3d::Drawable::*)(const class std::vector<unsigned int> &)) &easy3d::Drawable::update_element_buffer, "C++: easy3d::Drawable::update_element_buffer(const class std::vector<unsigned int> &) --> void", pybind11::arg("elements"));
        cl.def("update_element_buffer", (void (easy3d::Drawable::*)(const class std::vector<class std::vector<unsigned int> > &)) &easy3d::Drawable::update_element_buffer, "Updates the element buffer.\n \n\n This is an overload of the above update_element_buffer() method.\n \n\n Each entry must have 2 (for LinesDrawable) or 3 elements (for TrianglesDrawable).\n\nC++: easy3d::Drawable::update_element_buffer(const class std::vector<class std::vector<unsigned int> > &) --> void", pybind11::arg("elements"));
        cl.def("disable_element_buffer", (void (easy3d::Drawable::*)()) &easy3d::Drawable::disable_element_buffer, "Disables the use of the element buffer.\n \n\n This method should be used if existing vertex data is sufficient for rendering (may require\n         duplicating vertex data).\n \n\n This method also releases the element buffer.\n\nC++: easy3d::Drawable::disable_element_buffer() --> void");
        cl.def("num_vertices", (std::size_t (easy3d::Drawable::*)() const) &easy3d::Drawable::num_vertices, "@}\n\nC++: easy3d::Drawable::num_vertices() const --> std::size_t");
        cl.def("draw", (void (easy3d::Drawable::*)(const class easy3d::Camera *) const) &easy3d::Drawable::draw, "The draw method.\n\nC++: easy3d::Drawable::draw(const class easy3d::Camera *) const --> void", pybind11::arg("camera"));
        cl.def("gl_draw", (void (easy3d::Drawable::*)() const) &easy3d::Drawable::gl_draw, "The internal draw method of this drawable.\n NOTE: this functions should be called when your shader program is in use,\n		 i.e., between glUseProgram(id) and glUseProgram(0);\n\nC++: easy3d::Drawable::gl_draw() const --> void");
        cl.def("update", (void (easy3d::Drawable::*)()) &easy3d::Drawable::update, "Requests an update of the OpenGL buffers.\n \n\n This function sets the status to trigger an update of the OpenGL buffers. The actual update does\n      not occur immediately but is deferred to the rendering phase.\n \n\n This method works for both standard drawables (no update function required) and non-standard\n      drawable (update function required). Standard drawables include:\n            - SurfaceMesh: \"faces\", \"edges\", \"vertices\", \"borders\", and \"locks\";\n            - PointCloud: \"vertices\";\n            - Graph: \"edges\", and \"vertices\",\n            - PolyMesh: \"faces:border\" and \"faces:interior\".\n \n\n set_update_func(), Renderer::update()\n\nC++: easy3d::Drawable::update() --> void");
        cl.def("set_update_func", (void (easy3d::Drawable::*)(const class std::function<void (class easy3d::Model *, class easy3d::Drawable *)> &)) &easy3d::Drawable::set_update_func, "Setups how a drawable updates its rendering buffers.\n \n\n This function is required by only non-standard drawables for a special visualization purpose.\n      Rendering buffers of standard drawables attached to a model can be automatically updated and do not\n      require this function.\n      The drawable is considered standalone if  is \n \n\n update(), Renderer::update().\n \n\n For a subclassed drawable, you can also reimplement update_buffers_internal() for the same purpose.\n\nC++: easy3d::Drawable::set_update_func(const class std::function<void (class easy3d::Model *, class easy3d::Drawable *)> &) --> void", pybind11::arg("func"));
        cl.def("manipulator", (class easy3d::Manipulator * (easy3d::Drawable::*)()) &easy3d::Drawable::manipulator, "@{\n\n \n Gets the manipulator attached to this drawable.\n \n\n If the drawable is part of a model, it returns the model's manipulator.\n      Returns  if the drawable cannot be manipulated.\n\nC++: easy3d::Drawable::manipulator() --> class easy3d::Manipulator *", pybind11::return_value_policy::automatic);
        cl.def("set_manipulator", (void (easy3d::Drawable::*)(class easy3d::Manipulator *)) &easy3d::Drawable::set_manipulator, "Attaches a manipulator to this model.\n \n\n This is used to manipulate a drawable that is not part of a model.\n\nC++: easy3d::Drawable::set_manipulator(class easy3d::Manipulator *) --> void", pybind11::arg("manip"));
        cl.def("manipulated_matrix", (class easy3d::Mat4<float> (easy3d::Drawable::*)() const) &easy3d::Drawable::manipulated_matrix, "Returns the manipulation matrix.\n\nC++: easy3d::Drawable::manipulated_matrix() const --> class easy3d::Mat4<float>");
        cl.def("vao", (class easy3d::VertexArrayObject * (easy3d::Drawable::*)()) &easy3d::Drawable::vao, "Returns the vertex array object of this drawable.\n\nC++: easy3d::Drawable::vao() --> class easy3d::VertexArrayObject *", pybind11::return_value_policy::automatic);
        cl.def("assign", (class easy3d::Drawable & (easy3d::Drawable::*)(const class easy3d::Drawable &)) &easy3d::Drawable::operator=, "C++: easy3d::Drawable::operator=(const class easy3d::Drawable &) --> class easy3d::Drawable &", pybind11::return_value_policy::automatic, pybind11::arg(""));
    }

	{ // easy3d::LinesDrawable file:easy3d/renderer/drawable_lines.h line:40
		pybind11::class_<easy3d::LinesDrawable, std::shared_ptr<easy3d::LinesDrawable>, PyCallBack_easy3d_LinesDrawable, easy3d::Drawable> cl(m, "LinesDrawable", "The drawable for rendering a set of line segments, e.g., edges of a mesh, vector fields\n \n\n\n \n PointsDrawable, TrianglesDrawable");
        cl.def( pybind11::init( [](){ return new easy3d::LinesDrawable(); } ) );
        cl.def( pybind11::init( [](const std::string& name){ return new easy3d::LinesDrawable(name); } ) );
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
        cl.def( pybind11::init( [](){ return new easy3d::PointsDrawable(); } ) );
        cl.def( pybind11::init( [](const std::string& name){ return new easy3d::PointsDrawable(name); } ) );
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
        cl.def( pybind11::init( [](){ return new easy3d::TrianglesDrawable(); } ) );
        cl.def( pybind11::init( [](const std::string& name){ return new easy3d::TrianglesDrawable(name); } ) );
        cl.def( pybind11::init( [](PyCallBack_easy3d_TrianglesDrawable const &o){ return new PyCallBack_easy3d_TrianglesDrawable(o); } ) );
		cl.def( pybind11::init( [](easy3d::TrianglesDrawable const &o){ return new easy3d::TrianglesDrawable(o); } ) );
		cl.def("type", (enum easy3d::Drawable::Type (easy3d::TrianglesDrawable::*)() const) &easy3d::TrianglesDrawable::type, "C++: easy3d::TrianglesDrawable::type() const --> enum easy3d::Drawable::Type");
		cl.def("smooth_shading", (bool (easy3d::TrianglesDrawable::*)() const) &easy3d::TrianglesDrawable::smooth_shading, "C++: easy3d::TrianglesDrawable::smooth_shading() const --> bool");
		cl.def("set_smooth_shading", (void (easy3d::TrianglesDrawable::*)(bool)) &easy3d::TrianglesDrawable::set_smooth_shading, "C++: easy3d::TrianglesDrawable::set_smooth_shading(bool) --> void", pybind11::arg("b"));
		cl.def("opacity", (float (easy3d::TrianglesDrawable::*)() const) &easy3d::TrianglesDrawable::opacity, "Query the opacity of the drawable, in the range [0.0, 1.0].\n \n\n The opacity of the drawable.\n \n\n Opacity is effective only when transparency (e.g., DualDepthPeeling, AverageColorBlending) rendering is enabled.\n\nC++: easy3d::TrianglesDrawable::opacity() const --> float");
		cl.def("set_opacity", (void (easy3d::TrianglesDrawable::*)(float)) &easy3d::TrianglesDrawable::set_opacity, "Set the opacity of the drawable.\n \n\n The new opacity value, in the range [0.0, 1.0].\n \n\n Opacity is effective only when transparency (e.g., DualDepthPeeling, AverageColorBlending) rendering is enabled.\n\nC++: easy3d::TrianglesDrawable::set_opacity(float) --> void", pybind11::arg("opacity"));
		cl.def("assign", (class easy3d::TrianglesDrawable & (easy3d::TrianglesDrawable::*)(const class easy3d::TrianglesDrawable &)) &easy3d::TrianglesDrawable::operator=, "C++: easy3d::TrianglesDrawable::operator=(const class easy3d::TrianglesDrawable &) --> class easy3d::TrianglesDrawable &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}

    { // easy3d::TrianglesDrawable2D file:easy3d/renderer/drawable_triangles_2D.h line:43
        pybind11::class_<easy3d::LinesDrawable2D, std::shared_ptr<easy3d::LinesDrawable2D>, easy3d::Drawable> cl(m, "LinesDrawable2D");
        cl.def(pybind11::init<const std::string &>(), pybind11::arg("name") = "");
        cl.def("type", &easy3d::LinesDrawable2D::type);
        cl.def("draw", &easy3d::LinesDrawable2D::draw, pybind11::arg("camera"));

        cl.def("update_vertex_buffer",
               [](easy3d::LinesDrawable2D &self, const class std::vector<class easy3d::Vec<2, float> > &vertices, int width, int height, bool dynamic = false) -> void {
                   // Directly update vertex buffer with a std::vector<easy3d::vec3>
                   self.update_vertex_buffer(vertices, width, height, dynamic);
               },
               "Update vertex buffer with a std::vector of easy3d::vec3.",
               pybind11::arg("vertices"),
               pybind11::arg("width"),
               pybind11::arg("height"),
               pybind11::arg("dynamic") = false,
               R"doc(
                 Updates the vertex buffer from an array of easy3d::vec2.
                 Parameters:
                     vertices: An array of easy3d::vec2.
                     width: The width of the screen/viewport.
                     height: The height of the screen/viewport.
                     dynamic: If True, the buffer is dynamic and can be updated.
                 )doc"
        );

        cl.def("update_vertex_buffer",
               [](easy3d::LinesDrawable2D &self, pybind11::object vertices, int width, int height, bool dynamic = false) {
                   std::vector<easy3d::vec2> vec_vertices;
                   // Handle list of tuples
                   if (pybind11::isinstance<pybind11::list>(vertices)) {
                       for (auto item: vertices.cast<pybind11::list>()) {
                           auto tuple = item.cast<pybind11::tuple>();
                           if (tuple.size() != 2) {
                               throw std::invalid_argument("Each vertex must have 2 coordinates.");
                           }
                           vec_vertices.emplace_back(
                                   tuple[0].cast<float>(),
                                   tuple[1].cast<float>()
                           );
                       }
                   }
                       // Handle NumPy array
                   else if (pybind11::isinstance<pybind11::array_t<float>>(vertices)) {
                       auto buf = vertices.cast<pybind11::array_t<float>>().unchecked<2>();
                       if (buf.ndim() != 2 || buf.shape(1) != 2) {
                           throw std::invalid_argument("Input array must have shape (n, 2).");
                       }
                       vec_vertices.reserve(buf.shape(0));
                       for (ssize_t i = 0; i < buf.shape(0); ++i) {
                           vec_vertices.emplace_back(buf(i, 0), buf(i, 1));
                       }
                   } else if (pybind11::isinstance<pybind11::array_t<double>>(vertices)) {
                       auto buf = vertices.cast<pybind11::array_t<double>>().unchecked<2>();
                       if (buf.ndim() != 2 || buf.shape(1) != 2) {
                           throw std::invalid_argument("Input array must have shape (n, 2).");
                       }
                       vec_vertices.reserve(buf.shape(0));
                       for (ssize_t i = 0; i < buf.shape(0); ++i) {
                           vec_vertices.emplace_back(
                                   static_cast<float>(buf(i, 0)),
                                   static_cast<float>(buf(i, 1))
                           );
                       }
                   }
                   // Unsupported type
                   else {
                       throw std::invalid_argument("Vertices must be a list of tuples or a NumPy array (float32 or float64).");
                   }
                   // Call the C++ method
                   self.update_vertex_buffer(vec_vertices, width, height, dynamic);
               },
               pybind11::arg("vertices"),
               pybind11::arg("width"),
               pybind11::arg("height"),
               pybind11::arg("dynamic") = false,
               R"doc(
                 Updates the vertex buffer.

                 Parameters:
                     vertices: A list of (x, y) tuples or a NumPy array of shape (n, 2).
                     width: The width of the screen/viewport.
                     height: The height of the screen/viewport.
                     dynamic: If True, the buffer is dynamic and can be updated.
                 )doc"
        );
    }
}
