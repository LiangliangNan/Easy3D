#include <easy3d/util/progress.h>

#include <pybind11/pybind11.h>
#include <string>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

// easy3d::ProgressClient file:easy3d/util/progress.h line:24
struct PyCallBack_easy3d_ProgressClient : public easy3d::ProgressClient {
	using easy3d::ProgressClient::ProgressClient;

	void notify(std::size_t a0, bool a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::ProgressClient *>(this), "notify");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"ProgressClient::notify\"");
	}
	void cancel() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::ProgressClient *>(this), "cancel");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return ProgressClient::cancel();
	}
};

// easy3d::ProgressLogger file:easy3d/util/progress.h line:38
struct PyCallBack_easy3d_ProgressLogger : public easy3d::ProgressLogger {
	using easy3d::ProgressLogger::ProgressLogger;

	void notify(std::size_t a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::ProgressLogger *>(this), "notify");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return ProgressLogger::notify(a0);
	}
	void next() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::ProgressLogger *>(this), "next");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return ProgressLogger::next();
	}
	void done() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::ProgressLogger *>(this), "done");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return ProgressLogger::done();
	}
	void update() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::ProgressLogger *>(this), "update");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return ProgressLogger::update();
	}
};

void bind_easy3d_util_progress(pybind11::module_& m)
{
	{ // easy3d::ProgressClient file:easy3d/util/progress.h line:24
		pybind11::class_<easy3d::ProgressClient, std::shared_ptr<easy3d::ProgressClient>, PyCallBack_easy3d_ProgressClient> cl(m, "ProgressClient", "The based class of GUI element reporting the progress.\n \n\n\n     ");
		cl.def( pybind11::init( [](){ return new PyCallBack_easy3d_ProgressClient(); } ) );
		cl.def("notify", (void (easy3d::ProgressClient::*)(std::size_t, bool)) &easy3d::ProgressClient::notify, "C++: easy3d::ProgressClient::notify(std::size_t, bool) --> void", pybind11::arg("percent"), pybind11::arg("update_viewer"));
		cl.def("cancel", (void (easy3d::ProgressClient::*)()) &easy3d::ProgressClient::cancel, "C++: easy3d::ProgressClient::cancel() --> void");
		cl.def("assign", (class easy3d::ProgressClient & (easy3d::ProgressClient::*)(const class easy3d::ProgressClient &)) &easy3d::ProgressClient::operator=, "C++: easy3d::ProgressClient::operator=(const class easy3d::ProgressClient &) --> class easy3d::ProgressClient &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
	{ // easy3d::ProgressLogger file:easy3d/util/progress.h line:38
		pybind11::class_<easy3d::ProgressLogger, std::shared_ptr<easy3d::ProgressLogger>, PyCallBack_easy3d_ProgressLogger> cl(m, "ProgressLogger", "An implementation of progress logging mechanism.\n \n\n\n     ");
		cl.def( pybind11::init( [](std::size_t const & a0, bool const & a1){ return new easy3d::ProgressLogger(a0, a1); }, [](std::size_t const & a0, bool const & a1){ return new PyCallBack_easy3d_ProgressLogger(a0, a1); } ), "doc");
		cl.def( pybind11::init<std::size_t, bool, bool>(), pybind11::arg("max_val"), pybind11::arg("update_viewer"), pybind11::arg("quiet") );

		cl.def("notify", (void (easy3d::ProgressLogger::*)(std::size_t)) &easy3d::ProgressLogger::notify, "C++: easy3d::ProgressLogger::notify(std::size_t) --> void", pybind11::arg("new_value"));
		cl.def("next", (void (easy3d::ProgressLogger::*)()) &easy3d::ProgressLogger::next, "C++: easy3d::ProgressLogger::next() --> void");
		cl.def("done", (void (easy3d::ProgressLogger::*)()) &easy3d::ProgressLogger::done, "C++: easy3d::ProgressLogger::done() --> void");
		cl.def("is_canceled", (bool (easy3d::ProgressLogger::*)() const) &easy3d::ProgressLogger::is_canceled, "C++: easy3d::ProgressLogger::is_canceled() const --> bool");
		cl.def("reset", (void (easy3d::ProgressLogger::*)()) &easy3d::ProgressLogger::reset, "Resets the progress logger without changing the progress range.\n\nC++: easy3d::ProgressLogger::reset() --> void");
		cl.def("reset", (void (easy3d::ProgressLogger::*)(std::size_t)) &easy3d::ProgressLogger::reset, "Resets the progress logger, and meanwhile changes the progress range.\n\nC++: easy3d::ProgressLogger::reset(std::size_t) --> void", pybind11::arg("max_val"));
		cl.def("assign", (class easy3d::ProgressLogger & (easy3d::ProgressLogger::*)(const class easy3d::ProgressLogger &)) &easy3d::ProgressLogger::operator=, "C++: easy3d::ProgressLogger::operator=(const class easy3d::ProgressLogger &) --> class easy3d::ProgressLogger &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}

	// easy3d::print_progress(float) file:easy3d/util/progress.h line:70
	m.def("print_progress", (void (*)(float)) &easy3d::print_progress, "A simple progress indicator for console applications. Given percentage = 0.75, the output looks like\n     75% [||||||||||||||||||||||||||||||||||||||||||               ]\n\nC++: easy3d::print_progress(float) --> void", pybind11::arg("percentage"));
}
