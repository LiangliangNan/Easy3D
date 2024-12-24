#include <easy3d/util/timer.h>
#include <memory>

#include <pybind11/pybind11.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_util_timer(pybind11::module_& m)
{
//	 { // easy3d::Timer file:easy3d/util/timer.h line:49
//	 	pybind11::class_<easy3d::Timer>, std::shared_ptr<easy3d::Timer>>> cl(m, "Timer_t", "");
//	 	cl.def( pybind11::init( [](){ return new easy3d::Timer>(); } ) );
//	 	cl.def_static("single_shot", (void (*)(int, const class std::function<void (void)> &)) &easy3d::Timer<>::single_shot, "C++: easy3d::Timer<>::single_shot(int, const class std::function<void (void)> &) --> void", pybind11::arg("delay"), pybind11::arg("func"));
//	 	cl.def("set_timeout", (void (easy3d::Timer>::*)(int, const class std::function<void (void)> &) const) &easy3d::Timer<>::set_timeout, "C++: easy3d::Timer<>::set_timeout(int, const class std::function<void (void)> &) const --> void", pybind11::arg("delay"), pybind11::arg("func"));
//	 	cl.def("set_interval", (void (easy3d::Timer>::*)(int, const class std::function<void (void)> &) const) &easy3d::Timer<>::set_interval, "C++: easy3d::Timer<>::set_interval(int, const class std::function<void (void)> &) const --> void", pybind11::arg("interval"), pybind11::arg("func"));
//	 	cl.def("stop", (void (easy3d::Timer>::*)()) &easy3d::Timer<>::stop, "C++: easy3d::Timer<>::stop() --> void");
//	 	cl.def("is_stopped", (bool (easy3d::Timer>::*)() const) &easy3d::Timer<>::is_stopped, "C++: easy3d::Timer<>::is_stopped() const --> bool");
//	 	cl.def("pause", (void (easy3d::Timer>::*)()) &easy3d::Timer<>::pause, "C++: easy3d::Timer<>::pause() --> void");
//	 	cl.def("is_paused", (bool (easy3d::Timer>::*)() const) &easy3d::Timer<>::is_paused, "C++: easy3d::Timer<>::is_paused() const --> bool");
//	 	cl.def("resume", (void (easy3d::Timer>::*)()) &easy3d::Timer<>::resume, "C++: easy3d::Timer<>::resume() --> void");
//	 }
}
