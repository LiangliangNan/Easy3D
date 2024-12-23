#include <pybind11/pybind11.h>

#include <string>

#include <easy3d/core/point_cloud.h>
#include <easy3d/core/curve.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/manipulator.h>


namespace py = pybind11;

template<int DIM, typename FT>
void bind_generic_box(py::module_ &m, const std::string &class_name) {
    using GenericBox = easy3d::GenericBox<DIM, FT>;
    using Vec = easy3d::Vec<DIM, FT>;

    py::class_<GenericBox>(m, class_name.c_str())
            .def(py::init<>(), R"pbdoc(
            Construct a box that is initially invalid.
        )pbdoc")
            .def(py::init<const Vec&, const Vec&>(), R"pbdoc(
            Construct a box from its diagonal corners.
        )pbdoc")
            .def(py::init<const Vec&, FT>(), R"pbdoc(
            Construct a box from its center and radius.
        )pbdoc")
            .def("is_valid", &GenericBox::is_valid, R"pbdoc(
            Check if the box is valid.
        )pbdoc")
            .def("clear", &GenericBox::clear, R"pbdoc(
            Invalidate the box.
        )pbdoc")
            .def("min_point", py::overload_cast<>(&GenericBox::min_point, py::const_), R"pbdoc(
            Get the minimum corner of the box.
        )pbdoc")
            .def("max_point", py::overload_cast<>(&GenericBox::max_point, py::const_), R"pbdoc(
            Get the maximum corner of the box.
        )pbdoc")
            .def("min_coord", &GenericBox::min_coord, py::arg("axis"), R"pbdoc(
            Get the minimum coordinate along a specified axis.
        )pbdoc")
            .def("max_coord", &GenericBox::max_coord, py::arg("axis"), R"pbdoc(
            Get the maximum coordinate along a specified axis.
        )pbdoc")
            .def("range", &GenericBox::range, py::arg("axis"), R"pbdoc(
            Get the range along a specified axis.
        )pbdoc")
            .def("max_range", &GenericBox::max_range, R"pbdoc(
            Get the maximum range across all dimensions.
        )pbdoc")
            .def("min_range", &GenericBox::min_range, R"pbdoc(
            Get the minimum range across all dimensions.
        )pbdoc")
            .def("max_range_axis", &GenericBox::max_range_axis, R"pbdoc(
            Get the axis index with the maximum range.
        )pbdoc")
            .def("min_range_axis", &GenericBox::min_range_axis, R"pbdoc(
            Get the axis index with the minimum range.
        )pbdoc")
            .def("center", &GenericBox::center, R"pbdoc(
            Get the center of the box.
        )pbdoc")
            .def("diagonal_vector", &GenericBox::diagonal_vector, R"pbdoc(
            Get the diagonal vector of the box.
        )pbdoc")
            .def("diagonal_length", &GenericBox::diagonal_length, R"pbdoc(
            Get the length of the diagonal of the box.
        )pbdoc")
            .def("radius", &GenericBox::radius, R"pbdoc(
            Get the radius of the box (half of the diagonal length).
        )pbdoc")
            .def("surface_area", &GenericBox::surface_area, R"pbdoc(
            Get the surface area of the box (2D or 3D only).
        )pbdoc")
            .def("grow", py::overload_cast<const Vec&>(&GenericBox::grow), py::arg("point"), R"pbdoc(
            Extend the box to include a specified point.
        )pbdoc")
            .def("grow", py::overload_cast<const GenericBox&>(&GenericBox::grow), py::arg("box"), R"pbdoc(
            Extend the box to include another box.
        )pbdoc")
            .def("__add__", &GenericBox::operator+, R"pbdoc(
            Compute the union of two boxes.
        )pbdoc")
            .def("__iadd__", &GenericBox::operator+=, R"pbdoc(
            Extend this box by adding another box.
        )pbdoc")
            .def("contains", py::overload_cast<const Vec&>(&GenericBox::contains, py::const_), py::arg("point"), R"pbdoc(
            Check if this box contains a point.
        )pbdoc")
            .def("contains", py::overload_cast<const GenericBox&>(&GenericBox::contains, py::const_), py::arg("box"), R"pbdoc(
            Check if this box contains another box.
        )pbdoc")
            .def("intersects", &GenericBox::intersects, py::arg("box"), R"pbdoc(
            Check if this box intersects with another box.
        )pbdoc");
}


void bind_constants(py::module_& m) {
    m.attr("M_PI") = 3.14159265358979323846264338327950288;

    m.def("min_int", &easy3d::min<int>, "Get the minimum value for the int type.");
    m.def("max_int", &easy3d::max<int>, "Get the maximum value for the int type.");
    m.def("min_double", &easy3d::min<double>, "Get the minimum value for the double type.");
    m.def("max_double", &easy3d::max<double>, "Get the maximum value for the double type.");

    m.def("epsilon_double", &easy3d::epsilon<double>, "Get the epsilon value for the double type.");
    m.def("epsilon_sqr_double", &easy3d::epsilon_sqr<double>, "Get the squared epsilon value for the double type.");

    m.def(
            "epsilon_equal",
            &easy3d::epsilon_equal<double>,
            py::arg("x"),
            py::arg("y"),
            py::arg("eps"),
            "Test if two values are approximately equal within a given epsilon."
    );

    m.def(
            "epsilon_not_equal",
            &easy3d::epsilon_not_equal<double>,
            py::arg("x"),
            py::arg("y"),
            py::arg("eps"),
            "Test if two values are not approximately equal within a given epsilon."
    );

}


template<typename Point_t>
void bind_quadratic_binding(py::module_ &m) {
    m.def("quadratic", &easy3d::curve::quadratic<Point_t>,
          py::arg("A"), py::arg("B"), py::arg("C"), py::arg("curve"),
          py::arg("bezier_steps") = 4, py::arg("include_end") = false);
}

template<typename Point_t>
void bind_cubic_binding(py::module_ &m) {
    m.def("cubic", &easy3d::curve::cubic<Point_t>,
          py::arg("A"), py::arg("B"), py::arg("C"), py::arg("D"),
          py::arg("curve"), py::arg("bezier_steps") = 4, py::arg("include_end") = false);
}


template <typename Point_t>
void bind_curve(py::module_ &m) {
    // Bind the base Curve class
    py::class_<easy3d::Curve<Point_t>, std::shared_ptr<easy3d::Curve<Point_t>>>(m, "Curve")
            .def("set_steps", &easy3d::Curve<Point_t>::set_steps)
            .def("add_way_point", &easy3d::Curve<Point_t>::add_way_point)
            .def("node_count", &easy3d::Curve<Point_t>::node_count)
            .def("node", &easy3d::Curve<Point_t>::node)
            .def("length_from_start_point", &easy3d::Curve<Point_t>::length_from_start_point)
            .def("total_length", &easy3d::Curve<Point_t>::total_length)
            .def("clear", &easy3d::Curve<Point_t>::clear);
}

template <typename Point_t>
void bind_bezier(py::module_ &m) {
    // Bind the Bezier class
    py::class_<easy3d::Bezier<Point_t>, easy3d::Curve<Point_t>, std::shared_ptr<easy3d::Bezier<Point_t>>>(m, "Bezier")
            .def(py::init<>())
            .def("add_way_point", &easy3d::Bezier<Point_t>::add_way_point)
            .def("node_count", &easy3d::Bezier<Point_t>::node_count)
            .def("node", &easy3d::Bezier<Point_t>::node);
}

template <typename Point_t>
void bind_bspline(py::module_ &m) {
    // Bind the BSpline class
    py::class_<easy3d::BSpline<Point_t>, easy3d::Curve<Point_t>, std::shared_ptr<easy3d::BSpline<Point_t>>>(m, "BSpline")
            .def(py::init<>())
            .def("add_way_point", &easy3d::BSpline<Point_t>::add_way_point)
            .def("node_count", &easy3d::BSpline<Point_t>::node_count)
            .def("node", &easy3d::BSpline<Point_t>::node);
}

template <typename Point_t>
void bind_catmull_rom(py::module_ &m) {
    // Bind the CatmullRom class
    py::class_<easy3d::CatmullRom<Point_t>, easy3d::Curve<Point_t>, std::shared_ptr<easy3d::CatmullRom<Point_t>>>(m, "CatmullRom")
            .def(py::init<>())
            .def("add_way_point", &easy3d::CatmullRom<Point_t>::add_way_point)
            .def("node_count", &easy3d::CatmullRom<Point_t>::node_count)
            .def("node", &easy3d::CatmullRom<Point_t>::node);
}

// Define the necessary binding code for GenericLine with double precision (using double)
template <int DIM>
void bind_generic_line(py::module& m) {
    using Line = easy3d::GenericLine<DIM, double>;

    py::class_<Line>(m, ("Line" + std::to_string(DIM)).c_str())
            .def(py::init<>())  // Default constructor
            .def_static("from_point_and_direction", &Line::from_point_and_direction,
                        py::arg("point"), py::arg("direction"))  // Static method with two arguments
            .def_static("from_two_points", &Line::from_two_points,
                        py::arg("p"), py::arg("q"))  // Static method with two points
            .def("set", &Line::set, py::arg("point"), py::arg("direction"))  // Set method with two arguments
            .def("direction", &Line::direction)  // Returns the direction of the line
            .def("point", &Line::point)  // Returns a point on the line
            .def("projection", &Line::projection, py::arg("point"))  // Projects a point on the line
            .def("squared_distance", &Line::squared_distance, py::arg("point"))  // Returns the squared distance
            .def("feet", &Line::feet, py::arg("other"), py::arg("p1"), py::arg("p2"));  // Computes perpendicular feet
}


void bind_model(py::module& m) {
    py::class_<easy3d::Model>(m, "Model")
            .def("set_name", &easy3d::Model::set_name, py::arg("name"), "Set the name of the model.")
            .def("name", &easy3d::Model::name, py::return_value_policy::reference_internal, "Query the name of the model.")
            .def("bounding_box", &easy3d::Model::bounding_box, py::arg("recompute") = false)
            .def("invalidate_bounding_box", &easy3d::Model::invalidate_bounding_box)
            .def("points", py::overload_cast<>(&easy3d::Model::points, py::const_))
            .def("empty", &easy3d::Model::empty)
            .def("property_stats", &easy3d::Model::property_stats)
            .def("set_renderer", &easy3d::Model::set_renderer)
            .def("renderer", py::overload_cast<>(&easy3d::Model::renderer, py::const_))
            .def("set_manipulator", &easy3d::Model::set_manipulator)
            .def("manipulator", py::overload_cast<>(&easy3d::Model::manipulator, py::const_));
}

void bind_point_cloud(py::module& m) {
    py::class_<easy3d::PointCloud, easy3d::Model>(m, "PointCloud")
            .def(py::init<>())
            .def("add_vertex", &easy3d::PointCloud::add_vertex)
            .def("n_vertices", &easy3d::PointCloud::n_vertices)
            .def("clear", &easy3d::PointCloud::clear)
            .def("resize", &easy3d::PointCloud::resize, py::arg("n"))
            .def("has_garbage", &easy3d::PointCloud::has_garbage)
            .def("collect_garbage", &easy3d::PointCloud::collect_garbage)
            .def("delete_vertex", &easy3d::PointCloud::delete_vertex, py::arg("v"))
            .def("is_deleted", &easy3d::PointCloud::is_deleted)
            .def("is_valid", &easy3d::PointCloud::is_valid)
            .def("add_vertex_property", [](easy3d::PointCloud& self, const std::string& name) {
                return self.add_vertex_property<double>(name);
            }, py::arg("name"))
            .def("get_vertex_property", [](easy3d::PointCloud& self, const std::string& name) {
                return self.get_vertex_property<double>(name);
            }, py::arg("name"))
            .def("add_model_property", [](easy3d::PointCloud& self, const std::string& name) {
                return self.add_model_property<double>(name);
            }, py::arg("name"))
            .def("get_model_property", [](easy3d::PointCloud& self, const std::string& name) {
                return self.get_model_property<double>(name);
            }, py::arg("name"));
}


void init_core(py::module_& m) {
    m.doc() = "Bindings for Easy3D core functions";

    // Bind GenericBox for 2D
    bind_generic_box<2, double>(m, "Box2");

    // Bind GenericBox for 3D
    bind_generic_box<3, double>(m, "Box3");

    // Optional: Bind functions for union and intersection
    m.def("box_union", &easy3d::geom::box_union<3, double>, R"pbdoc(
        Compute the union of two 3D boxes.
    )pbdoc");
    m.def("box_intersection", &easy3d::geom::box_intersection<3, double>, R"pbdoc(
        Compute the intersection of two 3D boxes.
    )pbdoc");

    bind_constants(m);
    // Binding for quadratic and cubic functions
    bind_quadratic_binding<easy3d::vec3>(m);
    bind_cubic_binding<easy3d::vec3>(m);

    // Binding for Curve classes
    bind_curve<easy3d::vec3>(m);
    bind_bezier<easy3d::vec3>(m);
    bind_bspline<easy3d::vec3>(m);
    bind_catmull_rom<easy3d::vec3>(m);

    // Bind GenericLine for both 2D and 3D
    bind_generic_line<2>(m);
    bind_generic_line<3>(m);

    bind_model(m);
    bind_point_cloud(m);
}