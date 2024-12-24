#include <easy3d/renderer/opengl_timer.h>
#include <easy3d/renderer/opengl_util.h>
#include <ios>
#include <memory>
#include <ostream>
#include <sstream> // __str__
#include <streambuf>
#include <string>
#include <string_view>

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

void bind_easy3d_renderer_opengl_timer(pybind11::module_& m)
{
	{ // easy3d::OpenGLTimer file:easy3d/renderer/opengl_timer.h line:53
		pybind11::class_<easy3d::OpenGLTimer, std::shared_ptr<easy3d::OpenGLTimer>> cl(m, "OpenGLTimer", "Accurate timing of GPU operations.\n\n \n\n \n Because the GPU is based on an asynchronously processed command stream that is implicitly generated\n by OpenGL calls, it is not possible to determine the amount of time spent processing a set of OpenGL calls by\n timing the calls themselves at the application level. Instead, the timing must be done via an asynchronous\n timer system. This timer are started and stopped at the beginning and end of the OpenGL commands of interest.\n However, the timings results must be queried at some point later, as the pipeline must complete before the\n timer values are known. This class implements such a timer that behaves like a CPU timer (See StopWatch)\n\n Usage example:\n      \n\n\n\n\n\n\n     ");
		cl.def( pybind11::init( [](){ return new easy3d::OpenGLTimer(); } ), "doc" );
		cl.def( pybind11::init<bool>(), pybind11::arg("start_timing") );

		cl.def("start", (void (easy3d::OpenGLTimer::*)()) &easy3d::OpenGLTimer::start, "C++: easy3d::OpenGLTimer::start() --> void");
		cl.def("is_running", (bool (easy3d::OpenGLTimer::*)() const) &easy3d::OpenGLTimer::is_running, "C++: easy3d::OpenGLTimer::is_running() const --> bool");
		cl.def("stop", (void (easy3d::OpenGLTimer::*)()) &easy3d::OpenGLTimer::stop, "C++: easy3d::OpenGLTimer::stop() --> void");
		cl.def("time", (double (easy3d::OpenGLTimer::*)()) &easy3d::OpenGLTimer::time, "C++: easy3d::OpenGLTimer::time() --> double");
		cl.def("assign", (class easy3d::OpenGLTimer & (easy3d::OpenGLTimer::*)(const class easy3d::OpenGLTimer &)) &easy3d::OpenGLTimer::operator=, "C++: easy3d::OpenGLTimer::operator=(const class easy3d::OpenGLTimer &) --> class easy3d::OpenGLTimer &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
	{ // easy3d::OpenglUtil file:easy3d/renderer/opengl_util.h line:41
		pybind11::class_<easy3d::OpenglUtil, std::shared_ptr<easy3d::OpenglUtil>> cl(m, "OpenglUtil", "Utilities for OpenGL initialization and states query.\n \n\n\n     ");
		cl.def( pybind11::init( [](){ return new easy3d::OpenglUtil(); } ) );
		cl.def_static("init", (bool (*)()) &easy3d::OpenglUtil::init, "C++: easy3d::OpenglUtil::init() --> bool");
		cl.def_static("is_initialized", (bool (*)()) &easy3d::OpenglUtil::is_initialized, "C++: easy3d::OpenglUtil::is_initialized() --> bool");
		cl.def_static("gl_profile", (int (*)()) &easy3d::OpenglUtil::gl_profile, "returns either GL_CONTEXT_CORE_PROFILE_BIT or GL_CONTEXT_COMPATIBILITY_PROFILE_BIT\n\nC++: easy3d::OpenglUtil::gl_profile() --> int");
		cl.def_static("is_supported", (bool (*)(const std::string &)) &easy3d::OpenglUtil::is_supported, "C++: easy3d::OpenglUtil::is_supported(const std::string &) --> bool", pybind11::arg("name"));
		cl.def_static("has_extension", (bool (*)(const std::string &)) &easy3d::OpenglUtil::has_extension, "C++: easy3d::OpenglUtil::has_extension(const std::string &) --> bool", pybind11::arg("name"));
		cl.def_static("glew_version", (std::string (*)()) &easy3d::OpenglUtil::glew_version, "C++: easy3d::OpenglUtil::glew_version() --> std::string");
		cl.def_static("gl_vendor", (std::string (*)()) &easy3d::OpenglUtil::gl_vendor, "C++: easy3d::OpenglUtil::gl_vendor() --> std::string");
		cl.def_static("gl_renderer", (std::string (*)()) &easy3d::OpenglUtil::gl_renderer, "C++: easy3d::OpenglUtil::gl_renderer() --> std::string");
		cl.def_static("gl_version", (std::string (*)()) &easy3d::OpenglUtil::gl_version, "C++: easy3d::OpenglUtil::gl_version() --> std::string");
		cl.def_static("gl_extensions", (std::string (*)()) &easy3d::OpenglUtil::gl_extensions, "C++: easy3d::OpenglUtil::gl_extensions() --> std::string");
		cl.def_static("glsl_version", (std::string (*)()) &easy3d::OpenglUtil::glsl_version, "C++: easy3d::OpenglUtil::glsl_version() --> std::string");
		cl.def_static("gl_major_version", (int (*)()) &easy3d::OpenglUtil::gl_major_version, "C++: easy3d::OpenglUtil::gl_major_version() --> int");
		cl.def_static("gl_minor_version", (int (*)()) &easy3d::OpenglUtil::gl_minor_version, "C++: easy3d::OpenglUtil::gl_minor_version() --> int");
		cl.def_static("glew_version_number", (float (*)()) &easy3d::OpenglUtil::glew_version_number, "C++: easy3d::OpenglUtil::glew_version_number() --> float");
		cl.def_static("gl_version_number", (float (*)()) &easy3d::OpenglUtil::gl_version_number, "C++: easy3d::OpenglUtil::gl_version_number() --> float");
		cl.def_static("glsl_version_number", (float (*)()) &easy3d::OpenglUtil::glsl_version_number, "C++: easy3d::OpenglUtil::glsl_version_number() --> float");
		cl.def_static("samples", (int (*)()) &easy3d::OpenglUtil::samples, "C++: easy3d::OpenglUtil::samples() --> int");
		cl.def_static("viewport", (void (*)(int &, int &, int &, int &)) &easy3d::OpenglUtil::viewport, "Query the OpenGL viewport. The parameters are the same as in glViewport(x, y, width, height).\n x, y: the lower left corner of the viewport rectangle, in pixels.\n width, height: the width and height of the viewport.\n\nC++: easy3d::OpenglUtil::viewport(int &, int &, int &, int &) --> void", pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("width"), pybind11::arg("height"));
		cl.def_static("total_gpu_memory", (int (*)()) &easy3d::OpenglUtil::total_gpu_memory, "in MB.\n returns 0 if the query fails.\n NOTE: (1) OpenGL >= 2.0 is required.\n		  (2) currently only NVidia GPUs are supported\n\nC++: easy3d::OpenglUtil::total_gpu_memory() --> int");
		cl.def_static("available_gpu_memory", (int (*)()) &easy3d::OpenglUtil::available_gpu_memory, "C++: easy3d::OpenglUtil::available_gpu_memory() --> int");
		cl.def_static("set_output", (void (*)(std::ostream *)) &easy3d::OpenglUtil::set_output, "sets the output stream for the messages.\n if null, LOG(INFO) is the default output stream\n\nC++: easy3d::OpenglUtil::set_output(std::ostream *) --> void", pybind11::arg("out"));
		cl.def_static("get_current_buffer_info", (void (*)()) &easy3d::OpenglUtil::get_current_buffer_info, "display current bound buffer info\n\nC++: easy3d::OpenglUtil::get_current_buffer_info() --> void");
		cl.def_static("get_buffer_info", (void (*)(unsigned int, int)) &easy3d::OpenglUtil::get_buffer_info, "display the buffer information\n\nC++: easy3d::OpenglUtil::get_buffer_info(unsigned int, int) --> void", pybind11::arg("target"), pybind11::arg("bufferName"));
		cl.def_static("get_vao_info", (void (*)(unsigned int)) &easy3d::OpenglUtil::get_vao_info, "display VAO information, including its attributes\n\nC++: easy3d::OpenglUtil::get_vao_info(unsigned int) --> void", pybind11::arg("buffer"));
		cl.def_static("get_program_info", (void (*)(unsigned int)) &easy3d::OpenglUtil::get_program_info, "display detailed info for a program\n\nC++: easy3d::OpenglUtil::get_program_info(unsigned int) --> void", pybind11::arg("program"));
		cl.def_static("get_attributes_info", (void (*)(unsigned int)) &easy3d::OpenglUtil::get_attributes_info, "display detailed info for attributes in a program\n\nC++: easy3d::OpenglUtil::get_attributes_info(unsigned int) --> void", pybind11::arg("program"));
		cl.def_static("get_uniforms_info", (void (*)(unsigned int)) &easy3d::OpenglUtil::get_uniforms_info, "display info for all active uniforms in a program\n\nC++: easy3d::OpenglUtil::get_uniforms_info(unsigned int) --> void", pybind11::arg("program"));
		cl.def_static("get_uniform_info", (void (*)(unsigned int, const std::string &)) &easy3d::OpenglUtil::get_uniform_info, "display a uniform's value(s)\n\nC++: easy3d::OpenglUtil::get_uniform_info(unsigned int, const std::string &) --> void", pybind11::arg("program"), pybind11::arg("uniName"));
		cl.def_static("get_uniform_in_block_info", (void (*)(unsigned int, const std::string &, const std::string &)) &easy3d::OpenglUtil::get_uniform_in_block_info, "display the values for a uniform in a named block\n\nC++: easy3d::OpenglUtil::get_uniform_in_block_info(unsigned int, const std::string &, const std::string &) --> void", pybind11::arg("program"), pybind11::arg("blockName"), pybind11::arg("uniName"));
	}
}
