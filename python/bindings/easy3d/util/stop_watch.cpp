#include <easy3d/util/stop_watch.h>

#include <memory>

#include <pybind11/pybind11.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_util_stop_watch(pybind11::module_& m)
{
	{ // easy3d::StopWatch file:easy3d/util/stop_watch.h line:55
		pybind11::class_<easy3d::StopWatch, std::shared_ptr<easy3d::StopWatch>> cl(m, "StopWatch", "A high resolution stop watch/timer.\n \n\n This timer is able to measure the elapsed time with 1 micro-second accuracy on Windows, Linux, and Unix.\n  Usage example:\n      \n\n\n\n\n\n\n\n\n \n\n     ");
		cl.def( pybind11::init( [](){ return new easy3d::StopWatch(); } ) );
		cl.def("start", (void (easy3d::StopWatch::*)()) &easy3d::StopWatch::start, "starts the timer\n\nC++: easy3d::StopWatch::start() --> void");
		cl.def("restart", (void (easy3d::StopWatch::*)()) &easy3d::StopWatch::restart, "restarts the timer. It has the same effect as start()\n\nC++: easy3d::StopWatch::restart() --> void");
		cl.def("elapsed_seconds", [](easy3d::StopWatch const &o) -> double { return o.elapsed_seconds(); }, "");
		cl.def("elapsed_seconds", (double (easy3d::StopWatch::*)(int) const) &easy3d::StopWatch::elapsed_seconds, "returns user elapsed time (in seconds) since the construction / start.\n\nC++: easy3d::StopWatch::elapsed_seconds(int) const --> double", pybind11::arg("num_digits"));
		cl.def("time_string", [](easy3d::StopWatch const &o) -> std::string { return o.time_string(); }, "");
		cl.def("time_string", (std::string (easy3d::StopWatch::*)(int) const) &easy3d::StopWatch::time_string, "the elapsed time string, e.g., 88ms, 2.3s, 1.7m, 0.1h. This function automatically determines the best unit.\n\nC++: easy3d::StopWatch::time_string(int) const --> std::string", pybind11::arg("num_digits"));
	}
}
