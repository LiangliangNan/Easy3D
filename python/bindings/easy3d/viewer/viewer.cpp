#include <easy3d/core/vec.h>
#include <easy3d/core/model.h>
#include <easy3d/core/point_cloud.h>
#include <easy3d/core/surface_mesh.h>
#include <easy3d/core/graph.h>
#include <easy3d/core/poly_mesh.h>
#include <easy3d/renderer/drawable.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/viewer/viewer.h>

#include <memory>
#include <vector>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

// easy3d::Viewer file:easy3d/viewer/viewer.h line:60
struct PyCallBack_easy3d_Viewer : public easy3d::Viewer {
	using easy3d::Viewer::Viewer;

	void exit() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "exit");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Viewer::exit();
	}
	bool open() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "open");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return Viewer::open();
	}
	bool save() const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "save");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return Viewer::save();
	}
	bool snapshot() const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "snapshot");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return Viewer::snapshot();
	}
	using _binder_ret_0 = easy3d::Vec<3, float>;
	_binder_ret_0 point_under_pixel(int a0, int a1, bool & a2) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "point_under_pixel");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
			if (pybind11::detail::cast_is_temporary_value_reference<_binder_ret_0>::value) {
				static pybind11::detail::override_caster_t<_binder_ret_0> caster;
				return pybind11::detail::cast_ref<_binder_ret_0>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<_binder_ret_0>(std::move(o));
		}
		return Viewer::point_under_pixel(a0, a1, a2);
	}
	void draw() const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "draw");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Viewer::draw();
	}
	void init() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "init");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Viewer::init();
	}
	void pre_draw() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "pre_draw");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Viewer::pre_draw();
	}
	void post_draw() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "post_draw");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Viewer::post_draw();
	}
	void post_resize(int a0, int a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "post_resize");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Viewer::post_resize(a0, a1);
	}
	bool mouse_press_event(int a0, int a1, int a2, int a3) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "mouse_press_event");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return Viewer::mouse_press_event(a0, a1, a2, a3);
	}
	bool mouse_release_event(int a0, int a1, int a2, int a3) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "mouse_release_event");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return Viewer::mouse_release_event(a0, a1, a2, a3);
	}
	bool mouse_drag_event(int a0, int a1, int a2, int a3, int a4, int a5) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "mouse_drag_event");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3, a4, a5);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return Viewer::mouse_drag_event(a0, a1, a2, a3, a4, a5);
	}
	bool mouse_free_move_event(int a0, int a1, int a2, int a3, int a4) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "mouse_free_move_event");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3, a4);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return Viewer::mouse_free_move_event(a0, a1, a2, a3, a4);
	}
	bool mouse_scroll_event(int a0, int a1, int a2, int a3) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "mouse_scroll_event");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return Viewer::mouse_scroll_event(a0, a1, a2, a3);
	}
	bool char_input_event(unsigned int a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "char_input_event");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return Viewer::char_input_event(a0);
	}
	bool key_press_event(int a0, int a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "key_press_event");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return Viewer::key_press_event(a0, a1);
	}
	bool key_release_event(int a0, int a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "key_release_event");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return Viewer::key_release_event(a0, a1);
	}
	bool drop_event(const class std::vector<std::string> & a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "drop_event");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return Viewer::drop_event(a0);
	}
	bool focus_event(bool a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "focus_event");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return Viewer::focus_event(a0);
	}
	bool callback_event_cursor_pos(double a0, double a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "callback_event_cursor_pos");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return Viewer::callback_event_cursor_pos(a0, a1);
	}
	bool callback_event_mouse_button(int a0, int a1, int a2) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "callback_event_mouse_button");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return Viewer::callback_event_mouse_button(a0, a1, a2);
	}
	bool callback_event_keyboard(int a0, int a1, int a2) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "callback_event_keyboard");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return Viewer::callback_event_keyboard(a0, a1, a2);
	}
	bool callback_event_character(unsigned int a0) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "callback_event_character");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return Viewer::callback_event_character(a0);
	}
	bool callback_event_scroll(double a0, double a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "callback_event_scroll");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return Viewer::callback_event_scroll(a0, a1);
	}
	void callback_event_resize(int a0, int a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Viewer *>(this), "callback_event_resize");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Viewer::callback_event_resize(a0, a1);
	}
};

void bind_easy3d_viewer_viewer(pybind11::module_& m)
{
	{ // easy3d::Viewer file:easy3d/viewer/viewer.h line:60
		pybind11::class_<easy3d::Viewer, std::shared_ptr<easy3d::Viewer>, PyCallBack_easy3d_Viewer> cl(m, "Viewer", "The built-in Easy3D viewer.\n \n\n\n \n Easy3D is really easy to use. That's why it has such a name.\n To use the viewer, simply create an instance of Viewer, and call the run() method, e.g.,\n      \n\n\n\n\n The default Easy3D viewer can be easily extended by deriving your enhanced viewer from it\n by re-implementing some of its unctions (see the documentation of Viewer).");
		cl.def( pybind11::init( [](){ return new easy3d::Viewer(); }, [](){ return new PyCallBack_easy3d_Viewer(); } ), "doc");
		cl.def( pybind11::init( [](const std::string & a0){ return new easy3d::Viewer(a0); }, [](const std::string & a0){ return new PyCallBack_easy3d_Viewer(a0); } ), "doc");
		cl.def( pybind11::init( [](const std::string & a0, int const & a1){ return new easy3d::Viewer(a0, a1); }, [](const std::string & a0, int const & a1){ return new PyCallBack_easy3d_Viewer(a0, a1); } ), "doc");
		cl.def( pybind11::init( [](const std::string & a0, int const & a1, int const & a2){ return new easy3d::Viewer(a0, a1, a2); }, [](const std::string & a0, int const & a1, int const & a2){ return new PyCallBack_easy3d_Viewer(a0, a1, a2); } ), "doc");
		cl.def( pybind11::init( [](const std::string & a0, int const & a1, int const & a2, int const & a3){ return new easy3d::Viewer(a0, a1, a2, a3); }, [](const std::string & a0, int const & a1, int const & a2, int const & a3){ return new PyCallBack_easy3d_Viewer(a0, a1, a2, a3); } ), "doc");
		cl.def( pybind11::init( [](const std::string & a0, int const & a1, int const & a2, int const & a3, bool const & a4){ return new easy3d::Viewer(a0, a1, a2, a3, a4); }, [](const std::string & a0, int const & a1, int const & a2, int const & a3, bool const & a4){ return new PyCallBack_easy3d_Viewer(a0, a1, a2, a3, a4); } ), "doc");
		cl.def( pybind11::init( [](const std::string & a0, int const & a1, int const & a2, int const & a3, bool const & a4, bool const & a5){ return new easy3d::Viewer(a0, a1, a2, a3, a4, a5); }, [](const std::string & a0, int const & a1, int const & a2, int const & a3, bool const & a4, bool const & a5){ return new PyCallBack_easy3d_Viewer(a0, a1, a2, a3, a4, a5); } ), "doc");
		cl.def( pybind11::init( [](const std::string & a0, int const & a1, int const & a2, int const & a3, bool const & a4, bool const & a5, int const & a6){ return new easy3d::Viewer(a0, a1, a2, a3, a4, a5, a6); }, [](const std::string & a0, int const & a1, int const & a2, int const & a3, bool const & a4, bool const & a5, int const & a6){ return new PyCallBack_easy3d_Viewer(a0, a1, a2, a3, a4, a5, a6); } ), "doc");
		cl.def( pybind11::init( [](const std::string & a0, int const & a1, int const & a2, int const & a3, bool const & a4, bool const & a5, int const & a6, int const & a7){ return new easy3d::Viewer(a0, a1, a2, a3, a4, a5, a6, a7); }, [](const std::string & a0, int const & a1, int const & a2, int const & a3, bool const & a4, bool const & a5, int const & a6, int const & a7){ return new PyCallBack_easy3d_Viewer(a0, a1, a2, a3, a4, a5, a6, a7); } ), "doc");
		cl.def( pybind11::init( [](const std::string & a0, int const & a1, int const & a2, int const & a3, bool const & a4, bool const & a5, int const & a6, int const & a7, int const & a8){ return new easy3d::Viewer(a0, a1, a2, a3, a4, a5, a6, a7, a8); }, [](const std::string & a0, int const & a1, int const & a2, int const & a3, bool const & a4, bool const & a5, int const & a6, int const & a7, int const & a8){ return new PyCallBack_easy3d_Viewer(a0, a1, a2, a3, a4, a5, a6, a7, a8); } ), "doc");
		cl.def( pybind11::init<const std::string &, int, int, int, bool, bool, int, int, int, int>(), pybind11::arg("title"), pybind11::arg("samples"), pybind11::arg("gl_major"), pybind11::arg("gl_minor"), pybind11::arg("full_screen"), pybind11::arg("resizable"), pybind11::arg("depth_bits"), pybind11::arg("stencil_bits"), pybind11::arg("width"), pybind11::arg("height") );

		cl.def( pybind11::init( [](PyCallBack_easy3d_Viewer const &o){ return new PyCallBack_easy3d_Viewer(o); } ) );
		cl.def( pybind11::init( [](easy3d::Viewer const &o){ return new easy3d::Viewer(o); } ) );

		pybind11::enum_<easy3d::Viewer::Key>(cl, "Key", pybind11::arithmetic(), "The keys. Currently only a limited number of commonly used keys are supported.")
			.value("KEY_UNKNOWN", easy3d::Viewer::KEY_UNKNOWN)
			.value("KEY_0", easy3d::Viewer::KEY_0)
			.value("KEY_1", easy3d::Viewer::KEY_1)
			.value("KEY_2", easy3d::Viewer::KEY_2)
			.value("KEY_3", easy3d::Viewer::KEY_3)
			.value("KEY_4", easy3d::Viewer::KEY_4)
			.value("KEY_5", easy3d::Viewer::KEY_5)
			.value("KEY_6", easy3d::Viewer::KEY_6)
			.value("KEY_7", easy3d::Viewer::KEY_7)
			.value("KEY_8", easy3d::Viewer::KEY_8)
			.value("KEY_9", easy3d::Viewer::KEY_9)
			.value("KEY_A", easy3d::Viewer::KEY_A)
			.value("KEY_B", easy3d::Viewer::KEY_B)
			.value("KEY_C", easy3d::Viewer::KEY_C)
			.value("KEY_D", easy3d::Viewer::KEY_D)
			.value("KEY_E", easy3d::Viewer::KEY_E)
			.value("KEY_F", easy3d::Viewer::KEY_F)
			.value("KEY_G", easy3d::Viewer::KEY_G)
			.value("KEY_H", easy3d::Viewer::KEY_H)
			.value("KEY_I", easy3d::Viewer::KEY_I)
			.value("KEY_J", easy3d::Viewer::KEY_J)
			.value("KEY_K", easy3d::Viewer::KEY_K)
			.value("KEY_L", easy3d::Viewer::KEY_L)
			.value("KEY_M", easy3d::Viewer::KEY_M)
			.value("KEY_N", easy3d::Viewer::KEY_N)
			.value("KEY_O", easy3d::Viewer::KEY_O)
			.value("KEY_P", easy3d::Viewer::KEY_P)
			.value("KEY_Q", easy3d::Viewer::KEY_Q)
			.value("KEY_R", easy3d::Viewer::KEY_R)
			.value("KEY_S", easy3d::Viewer::KEY_S)
			.value("KEY_T", easy3d::Viewer::KEY_T)
			.value("KEY_U", easy3d::Viewer::KEY_U)
			.value("KEY_V", easy3d::Viewer::KEY_V)
			.value("KEY_W", easy3d::Viewer::KEY_W)
			.value("KEY_X", easy3d::Viewer::KEY_X)
			.value("KEY_Y", easy3d::Viewer::KEY_Y)
			.value("KEY_Z", easy3d::Viewer::KEY_Z)
			.value("KEY_RIGHT", easy3d::Viewer::KEY_RIGHT)
			.value("KEY_LEFT", easy3d::Viewer::KEY_LEFT)
			.value("KEY_DOWN", easy3d::Viewer::KEY_DOWN)
			.value("KEY_UP", easy3d::Viewer::KEY_UP)
			.value("KEY_F1", easy3d::Viewer::KEY_F1)
			.value("KEY_F2", easy3d::Viewer::KEY_F2)
			.value("KEY_F3", easy3d::Viewer::KEY_F3)
			.value("KEY_F4", easy3d::Viewer::KEY_F4)
			.value("KEY_F5", easy3d::Viewer::KEY_F5)
			.value("KEY_F6", easy3d::Viewer::KEY_F6)
			.value("KEY_F7", easy3d::Viewer::KEY_F7)
			.value("KEY_F8", easy3d::Viewer::KEY_F8)
			.value("KEY_F9", easy3d::Viewer::KEY_F9)
			.value("KEY_SPACE", easy3d::Viewer::KEY_SPACE)
			.value("KEY_COMMA", easy3d::Viewer::KEY_COMMA)
			.value("KEY_MINUS", easy3d::Viewer::KEY_MINUS)
			.value("KEY_PERIOD", easy3d::Viewer::KEY_PERIOD)
			.value("KEY_SLASH", easy3d::Viewer::KEY_SLASH)
			.value("KEY_SEMICOLON", easy3d::Viewer::KEY_SEMICOLON)
			.value("KEY_EQUAL", easy3d::Viewer::KEY_EQUAL)
			.value("KEY_LEFT_BRACKET", easy3d::Viewer::KEY_LEFT_BRACKET)
			.value("KEY_BACKSLASH", easy3d::Viewer::KEY_BACKSLASH)
			.value("KEY_RIGHT_BRACKET", easy3d::Viewer::KEY_RIGHT_BRACKET)
			.export_values();


		pybind11::enum_<easy3d::Viewer::Modifier>(cl, "Modifier", pybind11::arithmetic(), "The key modifiers. Currently only Shift, Ctrl, and Alt are supported.")
			.value("MODIF_NONE", easy3d::Viewer::MODIF_NONE)
			.value("MODIF_SHIFT", easy3d::Viewer::MODIF_SHIFT)
			.value("MODIF_CTRL", easy3d::Viewer::MODIF_CTRL)
			.value("MODIF_ALT", easy3d::Viewer::MODIF_ALT)
			.export_values();


		pybind11::enum_<easy3d::Viewer::Button>(cl, "Button", pybind11::arithmetic(), "Mouse buttons. ")
			.value("BUTTON_LEFT", easy3d::Viewer::BUTTON_LEFT)
			.value("BUTTON_RIGHT", easy3d::Viewer::BUTTON_RIGHT)
			.value("BUTTON_MIDDLE", easy3d::Viewer::BUTTON_MIDDLE)
			.export_values();

		cl.def_readwrite("animation_func_", &easy3d::Viewer::animation_func_);
		cl.def("run", [](easy3d::Viewer &o) -> int { return o.run(); }, "");
		cl.def("run", (int (easy3d::Viewer::*)(bool)) &easy3d::Viewer::run, "Run the viewer.\n \n\n See all contents within the screen? If true, the entire scene (i.e., all models) will be\n      centered on the screen at a proper scale.\n\nC++: easy3d::Viewer::run(bool) --> int", pybind11::arg("see_all"));
		cl.def("exit", (void (easy3d::Viewer::*)()) &easy3d::Viewer::exit, "Terminate the viewer.\n\nC++: easy3d::Viewer::exit() --> void");
		cl.def("set_title", (void (easy3d::Viewer::*)(const std::string &)) &easy3d::Viewer::set_title, "Set the window title of the viewer.\n \n\n The string of the title.\n\nC++: easy3d::Viewer::set_title(const std::string &) --> void", pybind11::arg("title"));
		cl.def("title", (const std::string & (easy3d::Viewer::*)() const) &easy3d::Viewer::title, "Query the window title of the viewer.\n \n\n The string of the window title.\n\nC++: easy3d::Viewer::title() const --> const std::string &", pybind11::return_value_policy::automatic);
		cl.def("resize", (void (easy3d::Viewer::*)(int, int)) &easy3d::Viewer::resize, "Set/Change the viewer size.\n \n\n The requested width/height (in pixels) of the viewer.\n\nC++: easy3d::Viewer::resize(int, int) --> void", pybind11::arg("w"), pybind11::arg("h"));
		cl.def("width", (int (easy3d::Viewer::*)() const) &easy3d::Viewer::width, "Returns the width of the viewer/window.\n\nC++: easy3d::Viewer::width() const --> int");
		cl.def("height", (int (easy3d::Viewer::*)() const) &easy3d::Viewer::height, "Returns the height of the viewer/window.\n\nC++: easy3d::Viewer::height() const --> int");
		cl.def("viewer_size", (void (easy3d::Viewer::*)(int &, int &) const) &easy3d::Viewer::viewer_size, "Queries the size of the viewer/window.\n\nC++: easy3d::Viewer::viewer_size(int &, int &) const --> void", pybind11::arg("w"), pybind11::arg("h"));
		cl.def("framebuffer_width", (int (easy3d::Viewer::*)() const) &easy3d::Viewer::framebuffer_width, "Returns the width of the framebuffer, which is identical to: width() * dpi_scaling().\n\nC++: easy3d::Viewer::framebuffer_width() const --> int");
		cl.def("framebuffer_height", (int (easy3d::Viewer::*)() const) &easy3d::Viewer::framebuffer_height, "Returns the height of the framebuffer, which is identical to: height() * dpi_scaling().\n\nC++: easy3d::Viewer::framebuffer_height() const --> int");
		cl.def("framebuffer_size", (void (easy3d::Viewer::*)(int &, int &) const) &easy3d::Viewer::framebuffer_size, "Queries the size of the framebuffer.\n\nC++: easy3d::Viewer::framebuffer_size(int &, int &) const --> void", pybind11::arg("w"), pybind11::arg("h"));
		cl.def("set_position", (void (easy3d::Viewer::*)(int, int)) &easy3d::Viewer::set_position, "Sets the position of the content area of the viewer.\n \n\n This function sets the position, in screen coordinates, of the upper-left corner of the content\n      area of the viewer. If the viewer is a full screen window, this function does nothing.\n  \n\n The x-coordinate of the upper-left corner of the content area.\n  \n\n The y-coordinate of the upper-left corner of the content area.\n  \n\n This function must only be called from the main thread.\n\nC++: easy3d::Viewer::set_position(int, int) --> void", pybind11::arg("xpos"), pybind11::arg("ypos"));
		cl.def("samples", (int (easy3d::Viewer::*)() const) &easy3d::Viewer::samples, "Query the actual samples of the viewer.\n \n\n The requested sample may not be supported by the context or graphics driver. This\n       method returns the actual samples that the viewer supports.\n\nC++: easy3d::Viewer::samples() const --> int");
		cl.def("dpi_scaling", (float (easy3d::Viewer::*)() const) &easy3d::Viewer::dpi_scaling, "Query the scaling factor for high DPI devices (e.g., MackBook pro).\n \n\n The high DPI scaling factor.\n\nC++: easy3d::Viewer::dpi_scaling() const --> float");
		cl.def("set_background_color", (void (easy3d::Viewer::*)(const class easy3d::Vec<4, float> &)) &easy3d::Viewer::set_background_color, "Set the background color of the viewer\n \n\n The background color.\n\nC++: easy3d::Viewer::set_background_color(const class easy3d::Vec<4, float> &) --> void", pybind11::arg("c"));
		cl.def("background_color", (const class easy3d::Vec<4, float> & (easy3d::Viewer::*)() const) &easy3d::Viewer::background_color, "Query the background color of the viewer.\n \n\n The background color of the viewer\n\nC++: easy3d::Viewer::background_color() const --> const class easy3d::Vec<4, float> &", pybind11::return_value_policy::automatic);
		cl.def("open", (bool (easy3d::Viewer::*)()) &easy3d::Viewer::open, "Open a model (PointCloud/SurfaceMesh/Graph) from a file into the viewer. On\n        success, the viewer will be in charge of the memory management of the model.\n \n\n This method loads a model into the viewer. Internally, it will pop up a file\n          dialog for the user to navigate to the file. After loading the model, the\n          necessary drawables (e.g., \"vertices\" for point clouds, \"faces\" for surface\n          meshes, and \"edges\" and \"vertices\" for graphs) will be created for visualization.\n \n\n true on success and false otherwise.\n\nC++: easy3d::Viewer::open() --> bool");
		cl.def("save", (bool (easy3d::Viewer::*)() const) &easy3d::Viewer::save, "Save the active model (if exists) to a file.\n \n\n This method saves the active model to a file. Internally, it will pop up a file\n          dialog for specifying the file name.\n \n\n true on success and false otherwise.\n\nC++: easy3d::Viewer::save() const --> bool");
        cl.def("add_model", [](easy3d::Viewer& self, const std::string& file_name, bool create_default_drawables = true)
               {
                   return self.add_model(file_name, create_default_drawables);
               },
               pybind11::arg("file_name"), pybind11::arg("create_default_drawables") = true,
               pybind11::return_value_policy::reference_internal,
               "Add a model from a file to the viewer.");
        cl.def("add_model", [](easy3d::Viewer& self, std::shared_ptr<easy3d::PointCloud> point_cloud, bool create_default_drawables = true)
               {
                   return self.add_model(point_cloud, create_default_drawables);
               },
               pybind11::arg("point_cloud"), pybind11::arg("create_default_drawables") = true,
               pybind11::return_value_policy::reference_internal,
               "Add an existing point cloud to the viewer.");
        cl.def("add_model", [](easy3d::Viewer& self, std::shared_ptr<easy3d::SurfaceMesh> surface_mesh, bool create_default_drawables = true)
               {
                   return self.add_model(surface_mesh, create_default_drawables);
               },
               pybind11::arg("surface_mesh"), pybind11::arg("create_default_drawables") = true,
               pybind11::return_value_policy::reference_internal,
               "Add an existing surface mesh to the viewer."
        );
        cl.def("add_model", [](easy3d::Viewer& self, std::shared_ptr<easy3d::Graph> graph, bool create_default_drawables = true)
               {
                   return self.add_model(graph, create_default_drawables);
               },
               pybind11::arg("graph"), pybind11::arg("create_default_drawables") = true,
               pybind11::return_value_policy::reference_internal,
               "Add an existing graph to the viewer."
        );
        cl.def("add_model", [](easy3d::Viewer& self, std::shared_ptr<easy3d::PolyMesh> poly_mesh, bool create_default_drawables = true)
               {
                   return self.add_model(poly_mesh, create_default_drawables);
               },
               pybind11::arg("poly_mesh"), pybind11::arg("create_default_drawables") = true,
               pybind11::return_value_policy::reference_internal,
               "Add an existing poly mesh to the viewer."
        );
		cl.def("models", (const class std::vector<class easy3d::Model *> & (easy3d::Viewer::*)() const) &easy3d::Viewer::models, "Query the models managed by this viewer.\n \n\n The models managed by this viewer.\n\nC++: easy3d::Viewer::models() const --> const class std::vector<class easy3d::Model *> &", pybind11::return_value_policy::automatic);
		cl.def("drawables", (const class std::vector<class easy3d::Drawable *> & (easy3d::Viewer::*)() const) &easy3d::Viewer::drawables, "Query the drawables managed by this viewer.\n \n\n The drawables managed by this viewer.\n\nC++: easy3d::Viewer::drawables() const --> const class std::vector<class easy3d::Drawable *> &", pybind11::return_value_policy::automatic);
		cl.def("clear_scene", (void (easy3d::Viewer::*)()) &easy3d::Viewer::clear_scene, "Delete all visual contents of the viewer (all models and drawables).\n\nC++: easy3d::Viewer::clear_scene() --> void");
		cl.def("update", (void (easy3d::Viewer::*)() const) &easy3d::Viewer::update, "Update the display (i.e., repaint).\n \n\n This method is used to update the display of the rendering. Client should call\n          it when your data/view is changed.\n\nC++: easy3d::Viewer::update() const --> void");

        cl.def("camera", (class easy3d::Camera* (easy3d::Viewer::*)()) &easy3d::Viewer::camera, "Returns the camera used by the viewer", pybind11::return_value_policy::reference_internal);
        cl.def("camera", (const class easy3d::Camera* (easy3d::Viewer::*)() const) &easy3d::Viewer::camera, "Returns the camera used by the viewer", pybind11::return_value_policy::reference_internal);

        cl.def("snapshot", (bool (easy3d::Viewer::*)() const) &easy3d::Viewer::snapshot, "Take a snapshot of the screen and save it to a file.\n \n\n This method takes a snapshot of the screen and saves the snapshot into an image file.\n          Internally, it will pop up a file dialog for specifying the file name.\n \n\n true on success and false otherwise.\n\nC++: easy3d::Viewer::snapshot() const --> bool");
		cl.def("snapshot", [](easy3d::Viewer const &o, const std::string & a0) -> bool { return o.snapshot(a0); }, "", pybind11::arg("file_name"));
		cl.def("snapshot", [](easy3d::Viewer const &o, const std::string & a0, float const & a1) -> bool { return o.snapshot(a0, a1); }, "", pybind11::arg("file_name"), pybind11::arg("scaling"));
		cl.def("snapshot", [](easy3d::Viewer const &o, const std::string & a0, float const & a1, int const & a2) -> bool { return o.snapshot(a0, a1, a2); }, "", pybind11::arg("file_name"), pybind11::arg("scaling"), pybind11::arg("samples"));
		cl.def("snapshot", [](easy3d::Viewer const &o, const std::string & a0, float const & a1, int const & a2, int const & a3) -> bool { return o.snapshot(a0, a1, a2, a3); }, "", pybind11::arg("file_name"), pybind11::arg("scaling"), pybind11::arg("samples"), pybind11::arg("back_ground"));
		cl.def("snapshot", (bool (easy3d::Viewer::*)(const std::string &, float, int, int, bool) const) &easy3d::Viewer::snapshot, "Take a snapshot of the screen and save it to an image file. Supported image format: png, jpg, bmp, and tga.\n \n\n This function renders the scene into a framebuffer and takes a snapshot of the framebuffer.\n      It allows the snapshot image to have a dimension different from the viewer, and it has no limit on the\n      image size (if memory allows).\n \n\n The image file name.\n \n\n The scaling factor that determines the size of the image (default to 1.0, using the viewer size), i.e., \n      image_width = viewer_width * scaling;\n      image_height = viewer_height * scaling;\n \n\n The required number of samples for antialiased rendering (which can be different from\n      that of the default framebuffer). The default value is 4.\n \n\n Determines the background color. 0: current color; 1: white; 2: transparent.\n \n\n Expand the frustum to ensure the image aspect ratio.\n \n\n true on success and false otherwise.\n\nC++: easy3d::Viewer::snapshot(const std::string &, float, int, int, bool) const --> bool", pybind11::arg("file_name"), pybind11::arg("scaling"), pybind11::arg("samples"), pybind11::arg("back_ground"), pybind11::arg("expand"));
		cl.def("point_under_pixel", (class easy3d::Vec<3, float> (easy3d::Viewer::*)(int, int, bool &) const) &easy3d::Viewer::point_under_pixel, "Query the XYZ coordinates of the surface point under the cursor.\n \n\n The cursor x-coordinate, relative to the left edge of the content area.\n \n\n The cursor y-coordinate, relative to the top edge of the content area.\n \n\n indicates whether the point was found or not.\n \n\n The coordinates of the 3D point located at pixel (x,y) on screen. The returned point is valid only\n      if found was returned true.\n \n\n The screen point (x, y) is expressed in the screen coordinate system with an origin in the\n      upper left corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g., a\n      Mac with a Retina display. If your inherited viewer uses a customized content area, you must also\n      reimplement this function such that the x and y are relative to left and top edges of the content\n      area, respectively.\n \n\n This method assumes that a GL context is available, and that its content was drawn using the Camera\n      (i.e. using its projection and model-view matrices). This method hence cannot be used for offscreen\n      Camera computations. Use cameraCoordinatesOf() and worldCoordinatesOf() to perform similar operations\n      in that case. The precision of the method highly depends on the z-Buffer, i.e., how the zNear() and\n      zFar() values are fitted to your scene. Loose boundaries will result in imprecision along the viewing\n      direction.\n\nC++: easy3d::Viewer::point_under_pixel(int, int, bool &) const --> class easy3d::Vec<3, float>", pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("found"));
		cl.def("usage", (const std::string & (easy3d::Viewer::*)() const) &easy3d::Viewer::usage, "The usage information of the viewer. For the time being, it is the manual of this default viewer.\n\nC++: easy3d::Viewer::usage() const --> const std::string &", pybind11::return_value_policy::automatic);
		cl.def("set_usage", (void (easy3d::Viewer::*)(const std::string &)) &easy3d::Viewer::set_usage, "C++: easy3d::Viewer::set_usage(const std::string &) --> void", pybind11::arg("usg"));
		cl.def("set_animation", (void (easy3d::Viewer::*)(bool)) &easy3d::Viewer::set_animation, "Enable/Disable animation.\n \n\n To have animation,  must be provided to specify how scene geometry is modified.\n\nC++: easy3d::Viewer::set_animation(bool) --> void", pybind11::arg("b"));
		cl.def("is_animating", (bool (easy3d::Viewer::*)() const) &easy3d::Viewer::is_animating, "Is animation currently being performed.\n\nC++: easy3d::Viewer::is_animating() const --> bool");
		cl.def("assign", (class easy3d::Viewer & (easy3d::Viewer::*)(const class easy3d::Viewer &)) &easy3d::Viewer::operator=, "C++: easy3d::Viewer::operator=(const class easy3d::Viewer &) --> class easy3d::Viewer &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
}
