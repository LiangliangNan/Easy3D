#include <easy3d/core/signal.h>
#include <easy3d/core/vec.h>

#include <functional>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>
#include <string_view>
#include <typeinfo>
#include <vector>

#include <functional>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

void bind_easy3d_core_signal(pybind11::module_& m)
{
//	 { // easy3d::Signal file:easy3d/core/signal.h line:56
//	 	pybind11::class_<easy3d::Signal>, std::shared_ptr<easy3d::Signal>>> cl(m, "Signal_t", "");
//	 	cl.def( pybind11::init( [](){ return new easy3d::Signal>(); } ) );
//	 	cl.def( pybind11::init( [](easy3d::Signal> const &o){ return new easy3d::Signal>(o); } ) );
//	 	cl.def("assign", (class easy3d::Signal<> & (easy3d::Signal>::*)(const class easy3d::Signal<> &)) &easy3d::Signal<>::operator=, "C++: easy3d::Signal<>::operator=(const class easy3d::Signal<> &) --> class easy3d::Signal<> &", pybind11::return_value_policy::automatic, pybind11::arg("other"));
//	 	cl.def("connect", (int (easy3d::Signal>::*)(const class std::function<void (void)> &)) &easy3d::Signal<>::connect, "C++: easy3d::Signal<>::connect(const class std::function<void (void)> &) --> int", pybind11::arg("slot"));
//	 	cl.def("connect", (int (easy3d::Signal>::*)(class easy3d::Signal<> *)) &easy3d::Signal<>::connect, "C++: easy3d::Signal<>::connect(class easy3d::Signal<> *) --> int", pybind11::arg("receiver"));
//	 	cl.def("disconnect", (void (easy3d::Signal>::*)(int)) &easy3d::Signal<>::disconnect, "C++: easy3d::Signal<>::disconnect(int) --> void", pybind11::arg("id"));
//	 	cl.def("disconnect_all", (void (easy3d::Signal>::*)()) &easy3d::Signal<>::disconnect_all, "C++: easy3d::Signal<>::disconnect_all() --> void");
//	 	cl.def("send", (void (easy3d::Signal>::*)()) &easy3d::Signal<>::send, "C++: easy3d::Signal<>::send() --> void");
//	 	cl.def("send_for_all_but_one", (void (easy3d::Signal>::*)(int)) &easy3d::Signal<>::send_for_all_but_one, "C++: easy3d::Signal<>::send_for_all_but_one(int) --> void", pybind11::arg("excludedConnectionID"));
//	 	cl.def("emit_for", (void (easy3d::Signal>::*)(int)) &easy3d::Signal<>::emit_for, "C++: easy3d::Signal<>::emit_for(int) --> void", pybind11::arg("connectionID"));
//	 }


}
