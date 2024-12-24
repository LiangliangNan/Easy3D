#include <easy3d/core/vec.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/viewer/multi_viewer.h>
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

// easy3d::MultiViewer file:easy3d/viewer/multi_viewer.h line:44
struct PyCallBack_easy3d_MultiViewer : public easy3d::MultiViewer {
	using easy3d::MultiViewer::MultiViewer;

	using _binder_ret_0 = easy3d::Vec<3, float>;
	_binder_ret_0 point_under_pixel(int a0, int a1, bool & a2) const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "point_under_pixel");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2);
			if (pybind11::detail::cast_is_temporary_value_reference<_binder_ret_0>::value) {
				static pybind11::detail::override_caster_t<_binder_ret_0> caster;
				return pybind11::detail::cast_ref<_binder_ret_0>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<_binder_ret_0>(std::move(o));
		}
		return MultiViewer::point_under_pixel(a0, a1, a2);
	}
	bool snapshot() const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "snapshot");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return MultiViewer::snapshot();
	}
	void init() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "init");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return MultiViewer::init();
	}
	void post_resize(int a0, int a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "post_resize");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return MultiViewer::post_resize(a0, a1);
	}
	void draw() const override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "draw");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return MultiViewer::draw();
	}
	bool mouse_press_event(int a0, int a1, int a2, int a3) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "mouse_press_event");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return MultiViewer::mouse_press_event(a0, a1, a2, a3);
	}
	bool mouse_release_event(int a0, int a1, int a2, int a3) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "mouse_release_event");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return MultiViewer::mouse_release_event(a0, a1, a2, a3);
	}
	bool mouse_drag_event(int a0, int a1, int a2, int a3, int a4, int a5) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "mouse_drag_event");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3, a4, a5);
			if (pybind11::detail::cast_is_temporary_value_reference<bool>::value) {
				static pybind11::detail::override_caster_t<bool> caster;
				return pybind11::detail::cast_ref<bool>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<bool>(std::move(o));
		}
		return MultiViewer::mouse_drag_event(a0, a1, a2, a3, a4, a5);
	}
	void exit() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "exit");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "open");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "save");
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
	void pre_draw() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "pre_draw");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "post_draw");
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
	bool mouse_free_move_event(int a0, int a1, int a2, int a3, int a4) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "mouse_free_move_event");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "mouse_scroll_event");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "char_input_event");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "key_press_event");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "key_release_event");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "drop_event");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "focus_event");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "callback_event_cursor_pos");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "callback_event_mouse_button");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "callback_event_keyboard");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "callback_event_character");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "callback_event_scroll");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::MultiViewer *>(this), "callback_event_resize");
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

void bind_easy3d_viewer_multi_viewer(pybind11::module_& m)
{
	{ // easy3d::MultiViewer file:easy3d/viewer/multi_viewer.h line:44
		pybind11::class_<easy3d::MultiViewer, std::shared_ptr<easy3d::MultiViewer>, PyCallBack_easy3d_MultiViewer, easy3d::Viewer> cl(m, "MultiViewer", "A viewer that supports multiple views (arranged in a grid layout).\n \n\n\n     ");
		cl.def( pybind11::init( [](int const & a0, int const & a1){ return new easy3d::MultiViewer(a0, a1); }, [](int const & a0, int const & a1){ return new PyCallBack_easy3d_MultiViewer(a0, a1); } ), "doc");
		cl.def( pybind11::init<int, int, const std::string &>(), pybind11::arg("rows"), pybind11::arg("cols"), pybind11::arg("title") );

		cl.def( pybind11::init( [](PyCallBack_easy3d_MultiViewer const &o){ return new PyCallBack_easy3d_MultiViewer(o); } ) );
		cl.def( pybind11::init( [](easy3d::MultiViewer const &o){ return new easy3d::MultiViewer(o); } ) );
		cl.def("set_division_visible", (void (easy3d::MultiViewer::*)(bool)) &easy3d::MultiViewer::set_division_visible, "Sets the visibility of the splitting lines of the views (visible by default).\n\nC++: easy3d::MultiViewer::set_division_visible(bool) --> void", pybind11::arg("b"));
		cl.def("division_visible", (bool (easy3d::MultiViewer::*)() const) &easy3d::MultiViewer::division_visible, "Returns if the splitting lines of the views are visible.\n\nC++: easy3d::MultiViewer::division_visible() const --> bool");
		cl.def("point_under_pixel", (class easy3d::Vec<3, float> (easy3d::MultiViewer::*)(int, int, bool &) const) &easy3d::MultiViewer::point_under_pixel, "Query the XYZ coordinates of the surface point under the cursor.\n \n\n The cursor x-coordinate, relative to the left edge of the content area.\n \n\n The cursor y-coordinate, relative to the top edge of the content area.\n \n\n indicates whether the point was found or not.\n \n\n The coordinates of the 3D point located at pixel (x, y) on screen. The returned point is valid\n      only if found was returned true.\n \n\n The screen point (x, y) is expressed in the screen coordinate system with an origin in the\n      upper left corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g., a\n      Mac with a Retina display. If your inherited viewer uses a customized content area, you must also\n      reimplement this function such that the x and y are relative to left and top edges of the content\n      area, respectively.\n\nC++: easy3d::MultiViewer::point_under_pixel(int, int, bool &) const --> class easy3d::Vec<3, float>", pybind11::arg("x"), pybind11::arg("y"), pybind11::arg("found"));
		cl.def("rows", (int (easy3d::MultiViewer::*)() const) &easy3d::MultiViewer::rows, "Return the number of rows (of the grid-like layout) of the viewer.\n\nC++: easy3d::MultiViewer::rows() const --> int");
		cl.def("columns", (int (easy3d::MultiViewer::*)() const) &easy3d::MultiViewer::columns, "Return the number of columns (of the grid-like layout) of the viewer.\n\nC++: easy3d::MultiViewer::columns() const --> int");
		cl.def("set_layout", (void (easy3d::MultiViewer::*)(int, int)) &easy3d::MultiViewer::set_layout, "Set/Change the layout of the viewer.\n \n\n The number of rows (of the grid-like layout).\n \n\n The number of columns (of the grid-like layout).\n\nC++: easy3d::MultiViewer::set_layout(int, int) --> void", pybind11::arg("rows"), pybind11::arg("cols"));
        cl.def("camera", (class easy3d::Camera* (easy3d::MultiViewer::*)()) &easy3d::MultiViewer::camera, "Returns the camera used by the viewer", pybind11::return_value_policy::automatic);
        cl.def("camera", (const class easy3d::Camera* (easy3d::MultiViewer::*)() const) &easy3d::MultiViewer::camera, "Returns the camera used by the viewer", pybind11::return_value_policy::automatic);
        cl.def("snapshot", (bool (easy3d::MultiViewer::*)() const) &easy3d::MultiViewer::snapshot, "Take a snapshot of the screen and save it to a file.\n \n\n This method takes a snapshot of the screen and saves the snapshot into an image file.\n          Internally, it will pop up a file dialog for specifying the file name.\n \n\n true on success and false otherwise.\n\nC++: easy3d::MultiViewer::snapshot() const --> bool");
		cl.def("assign", (class easy3d::MultiViewer & (easy3d::MultiViewer::*)(const class easy3d::MultiViewer &)) &easy3d::MultiViewer::operator=, "C++: easy3d::MultiViewer::operator=(const class easy3d::MultiViewer &) --> class easy3d::MultiViewer &", pybind11::return_value_policy::reference_internal, pybind11::arg(""));
	}

}
