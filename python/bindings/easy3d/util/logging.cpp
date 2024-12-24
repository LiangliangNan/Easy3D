#include <3rd_party/easyloggingpp/easylogging++.h>
#include <easy3d/util/logging.h>

#include <memory>

#include <pybind11/pybind11.h>

#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

// easy3d::logging::Logger file:easy3d/util/logging.h line:87
struct PyCallBack_easy3d_logging_Logger : public easy3d::logging::Logger {
	using easy3d::logging::Logger::Logger;

	void send(enum el::Level a0, const std::string & a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::logging::Logger *>(this), "send");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		pybind11::pybind11_fail("Tried to call pure virtual function \"Logger::send\"");
	}
};

void bind_easy3d_util_logging(pybind11::module_& m)
{
	// easy3d::logging::initialize(bool, bool, bool, bool, const std::string &, int) file:easy3d/util/logging.h line:69
	m.def("initialize", []() -> void { return easy3d::logging::initialize(); }, "");
	m.def("initialize", [](bool const & a0) -> void { return easy3d::logging::initialize(a0); }, "", pybind11::arg("info_to_stdout"));
	m.def("initialize", [](bool const & a0, bool const & a1) -> void { return easy3d::logging::initialize(a0, a1); }, "", pybind11::arg("info_to_stdout"), pybind11::arg("warning_to_stdcout"));
	m.def("initialize", [](bool const & a0, bool const & a1, bool const & a2) -> void { return easy3d::logging::initialize(a0, a1, a2); }, "", pybind11::arg("info_to_stdout"), pybind11::arg("warning_to_stdcout"), pybind11::arg("error_to_stdcout"));
	m.def("initialize", [](bool const & a0, bool const & a1, bool const & a2, bool const & a3) -> void { return easy3d::logging::initialize(a0, a1, a2, a3); }, "", pybind11::arg("info_to_stdout"), pybind11::arg("warning_to_stdcout"), pybind11::arg("error_to_stdcout"), pybind11::arg("verbose_to_stdcout"));
	m.def("initialize", [](bool const & a0, bool const & a1, bool const & a2, bool const & a3, const std::string & a4) -> void { return easy3d::logging::initialize(a0, a1, a2, a3, a4); }, "", pybind11::arg("info_to_stdout"), pybind11::arg("warning_to_stdcout"), pybind11::arg("error_to_stdcout"), pybind11::arg("verbose_to_stdcout"), pybind11::arg("log_file"));
	m.def("initialize", (void (*)(bool, bool, bool, bool, const std::string &, int)) &easy3d::logging::initialize, "Initializes the logging module.\n \n\n  to log messages at a the  level to standard output.\n \n\n  to log messages at a the  level to standard output.\n \n\n  to log messages at a the  (including  level to standard output.\n \n\n  to log messages at a the  (including  level to standard output.\n \n\n A string specifying the name of log file. Three different values are accepted:\n    - \"\": an empty string, indicating that no log file will be created.\n    - \"default\": create a log file with a title in the form \"AppName.log\" next to the executable file.\n    - any non-empty string: (if it is a valid path) a log file with the same name will be created and log\n      messages will be written to this file.\n \n\n A  with  <=  will be written into the\n      log file (if specified). Value must be in the range [0, 9]. Default is 0.\n \n\n This initialization is optional. If not called, log messages will be written to standard output only.\n\nC++: easy3d::logging::initialize(bool, bool, bool, bool, const std::string &, int) --> void", pybind11::arg("info_to_stdout"), pybind11::arg("warning_to_stdcout"), pybind11::arg("error_to_stdcout"), pybind11::arg("verbose_to_stdcout"), pybind11::arg("log_file"), pybind11::arg("verbosity_threshold"));

	// easy3d::logging::is_initialized() file:easy3d/util/logging.h line:79
	m.def("is_initialized", (bool (*)()) &easy3d::logging::is_initialized, "Returns whether the logging has been initialized.\n\nC++: easy3d::logging::is_initialized() --> bool");

	// easy3d::logging::log_file() file:easy3d/util/logging.h line:82
	m.def("log_file", (std::string (*)()) &easy3d::logging::log_file, "Returns the full path of the log file (empty if no log file has been created).\n\nC++: easy3d::logging::log_file() --> std::string");

	{ // easy3d::logging::Logger file:easy3d/util/logging.h line:87
		pybind11::class_<easy3d::logging::Logger, std::shared_ptr<easy3d::logging::Logger>, PyCallBack_easy3d_logging_Logger> cl(m, "Logger", "Base class for a logger (that can log messages to whatever)\n Users should subclass Logger and override send() to do whatever they want.\n \n");
		cl.def( pybind11::init( [](){ return new PyCallBack_easy3d_logging_Logger(); } ) );
		cl.def("send", (void (easy3d::logging::Logger::*)(enum el::Level, const std::string &)) &easy3d::logging::Logger::send, "writes the log message  (and may also other given information).\n\nC++: easy3d::logging::Logger::send(enum el::Level, const std::string &) --> void", pybind11::arg("level"), pybind11::arg("msg"));
		cl.def("assign", (class easy3d::logging::Logger & (easy3d::logging::Logger::*)(const class easy3d::logging::Logger &)) &easy3d::logging::Logger::operator=, "C++: easy3d::logging::Logger::operator=(const class easy3d::logging::Logger &) --> class easy3d::logging::Logger &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
}
