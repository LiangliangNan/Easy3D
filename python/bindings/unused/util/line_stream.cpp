#include <easy3d/util/line_stream.h>

#include <memory>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_util_line_stream(pybind11::module_& m)
{
	{ // easy3d::io::LineInputStream file:easy3d/util/line_stream.h line:47
		pybind11::class_<easy3d::io::LineInputStream, std::shared_ptr<easy3d::io::LineInputStream>> cl(m, "LineInputStream", "Input stream class to operate on ASCII files.\n \n\n\n         ");
		cl.def( pybind11::init<std::istream &>(), pybind11::arg("in") );

		cl.def( pybind11::init( [](easy3d::io::LineInputStream const &o){ return new easy3d::io::LineInputStream(o); } ) );
		cl.def("eof", (bool (easy3d::io::LineInputStream::*)() const) &easy3d::io::LineInputStream::eof, "C++: easy3d::io::LineInputStream::eof() const --> bool");
		cl.def("eol", (bool (easy3d::io::LineInputStream::*)() const) &easy3d::io::LineInputStream::eol, "C++: easy3d::io::LineInputStream::eol() const --> bool");
		cl.def("fail", (bool (easy3d::io::LineInputStream::*)() const) &easy3d::io::LineInputStream::fail, "C++: easy3d::io::LineInputStream::fail() const --> bool");
		cl.def("get_line", (void (easy3d::io::LineInputStream::*)()) &easy3d::io::LineInputStream::get_line, "C++: easy3d::io::LineInputStream::get_line() --> void");
		cl.def("line", (std::istream & (easy3d::io::LineInputStream::*)()) &easy3d::io::LineInputStream::line, "C++: easy3d::io::LineInputStream::line() --> std::istream &", pybind11::return_value_policy::automatic);
		cl.def("current_line", (const std::string & (easy3d::io::LineInputStream::*)() const) &easy3d::io::LineInputStream::current_line, "C++: easy3d::io::LineInputStream::current_line() const --> const std::string &", pybind11::return_value_policy::automatic);
	}
}
