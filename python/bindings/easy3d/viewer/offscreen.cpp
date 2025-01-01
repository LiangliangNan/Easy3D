#include <easy3d/core/vec.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/drawable.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/viewer/offscreen.h>
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


// easy3d::OffScreen file:easy3d/viewer/offscreen.h line:49
struct PyCallBack_easy3d_OffScreen : public easy3d::OffScreen {
	using easy3d::OffScreen::OffScreen;

	void exit() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "exit");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "open");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "save");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "snapshot");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "point_under_pixel");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "draw");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "init");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "pre_draw");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "post_draw");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "post_resize");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "mouse_press_event");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "mouse_release_event");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "mouse_drag_event");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "mouse_free_move_event");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "mouse_scroll_event");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "char_input_event");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "key_press_event");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "key_release_event");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "drop_event");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "focus_event");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "callback_event_cursor_pos");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "callback_event_mouse_button");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "callback_event_keyboard");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "callback_event_character");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "callback_event_scroll");
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
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::OffScreen *>(this), "callback_event_resize");
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

void bind_easy3d_viewer_offscreen(pybind11::module_& m)
{
	{ // easy3d::OffScreen file:easy3d/viewer/offscreen.h line:49
		pybind11::class_<easy3d::OffScreen, std::shared_ptr<easy3d::OffScreen>, PyCallBack_easy3d_OffScreen> cl(m, "OffScreen", "Offscreen rendering.\n \n\n\n \n To use offscreen rendering, simply create an instance of OffScreen and call the render() method, e.g.,\n      \n\n\n\n\n\n\n\n\n\n     ");
		cl.def( pybind11::init( [](){ return new easy3d::OffScreen(); }, [](){ return new PyCallBack_easy3d_OffScreen(); } ), "doc");
		cl.def( pybind11::init( [](int const & a0){ return new easy3d::OffScreen(a0); }, [](int const & a0){ return new PyCallBack_easy3d_OffScreen(a0); } ), "doc");
		cl.def( pybind11::init<int, int>(), pybind11::arg("width"), pybind11::arg("height") );

//		cl.def( pybind11::init( [](PyCallBack_easy3d_OffScreen const &o){ return new PyCallBack_easy3d_OffScreen(o); } ) );
//		cl.def( pybind11::init( [](easy3d::OffScreen const &o){ return new easy3d::OffScreen(o); } ) );

        cl.def("add_model", [](easy3d::OffScreen& self, const std::string& file_name, bool create_default_drawables = true)
               {
                   return self.add_model(file_name, create_default_drawables);
               },
               pybind11::arg("file_name"), pybind11::arg("create_default_drawables") = true,
               pybind11::return_value_policy::reference_internal,
               "Add a model from a file to the viewer.");

        cl.def("add_model", [](easy3d::OffScreen &self, std::shared_ptr<easy3d::Model> model) {
                   return self.add_model(model); // Call the C++ function
               }, pybind11::arg("model"),
               pybind11::return_value_policy::automatic,
               R"doc(
                    Add an existing model to the viewer to be visualized.
                    Parameters:
                        model (Model): The pointer to the model.
                    Returns:
                        Model: The pointer of the model added to the viewer.
                )doc");

        cl.def("add_drawable", [](easy3d::OffScreen &self, std::shared_ptr<easy3d::Drawable> drawable) {
                   return self.add_drawable(drawable); // Call the C++ function
               }, pybind11::arg("drawable"),
               pybind11::return_value_policy::automatic,
               R"doc(
                    Add a drawable to the viewer to be visualized.
                    Parameters:
                        drawable (Drawable): The pointer to the drawable.
                    Returns:
                        Drawable: The pointer of the drawable added to the viewer.
                )doc");

        cl.def("camera", (class easy3d::Camera* (easy3d::OffScreen::*)()) &easy3d::OffScreen::camera, "Returns the camera used by the offscreen renderer", pybind11::return_value_policy::reference_internal);
        cl.def("camera", (const class easy3d::Camera* (easy3d::OffScreen::*)() const) &easy3d::OffScreen::camera, "Returns the camera used by the offscreen renderer", pybind11::return_value_policy::reference_internal);

		cl.def("render", [](easy3d::OffScreen const &o, const std::string & a0) -> bool { return o.render(a0); }, "", pybind11::arg("file_name"));
		cl.def("render", [](easy3d::OffScreen const &o, const std::string & a0, float const & a1) -> bool { return o.render(a0, a1); }, "", pybind11::arg("file_name"), pybind11::arg("scaling"));
		cl.def("render", [](easy3d::OffScreen const &o, const std::string & a0, float const & a1, int const & a2) -> bool { return o.render(a0, a1, a2); }, "", pybind11::arg("file_name"), pybind11::arg("scaling"), pybind11::arg("samples"));
		cl.def("render", [](easy3d::OffScreen const &o, const std::string & a0, float const & a1, int const & a2, int const & a3) -> bool { return o.render(a0, a1, a2, a3); }, "", pybind11::arg("file_name"), pybind11::arg("scaling"), pybind11::arg("samples"), pybind11::arg("back_ground"));
		cl.def("render", (bool (easy3d::OffScreen::*)(const std::string &, float, int, int, bool) const) &easy3d::OffScreen::render, "Render the current scene into an image file. Supported image format: png, jpg, bmp, and tga.\n \n\n This function renders the scene into a framebuffer and takes a snapshot of the framebuffer.\n      It allows the snapshot image to have a dimension different from the offscreen renderer, and it\n      has no limit on the image size (if memory allows).\n \n\n The image file name.\n \n\n The scaling factor that determines the size of the image (default to 1.0, same size as\n      the offscreen renderer), i.e.,\n      image_width = width() * scaling;\n      image_height = height() * scaling;\n \n\n The required number of samples for antialiased rendering (which can be different from\n      that of the default framebuffer). The default value is 4.\n \n\n Determines the background color. 0: current color; 1: white; 2: transparent.\n \n\n Expand the frustum to ensure the image aspect ratio.\n \n\n true on success and false otherwise.\n\nC++: easy3d::OffScreen::render(const std::string &, float, int, int, bool) const --> bool", pybind11::arg("file_name"), pybind11::arg("scaling"), pybind11::arg("samples"), pybind11::arg("back_ground"), pybind11::arg("expand"));
		cl.def("resize", (void (easy3d::OffScreen::*)(int, int)) &easy3d::OffScreen::resize, "Set/Change the size of the offscreen renderer.\n \n\n The requested width/height (in pixels) of the offscreen renderer.\n\nC++: easy3d::OffScreen::resize(int, int) --> void", pybind11::arg("w"), pybind11::arg("h"));
		cl.def("width", (int (easy3d::OffScreen::*)() const) &easy3d::OffScreen::width, "Returns the width of the offscreen renderer.\n\nC++: easy3d::OffScreen::width() const --> int");
		cl.def("height", (int (easy3d::OffScreen::*)() const) &easy3d::OffScreen::height, "Returns the height of the offscreen renderer.\n\nC++: easy3d::OffScreen::height() const --> int");
		cl.def("set_background_color", (void (easy3d::OffScreen::*)(const class easy3d::Vec<4, float> &)) &easy3d::OffScreen::set_background_color, "Set the background color of the offscreen renderer\n \n\n The background color.\n\nC++: easy3d::OffScreen::set_background_color(const class easy3d::Vec<4, float> &) --> void", pybind11::arg("c"));
		cl.def("background_color", (const class easy3d::Vec<4, float> & (easy3d::OffScreen::*)() const) &easy3d::OffScreen::background_color, "Query the background color of the offscreen renderer.\n \n\n The background color of the offscreen renderer\n\nC++: easy3d::OffScreen::background_color() const --> const class easy3d::Vec<4, float> &", pybind11::return_value_policy::automatic);
		cl.def("models", (const class std::vector<class easy3d::Model *> & (easy3d::OffScreen::*)() const) &easy3d::OffScreen::models, "Query the models managed by this offscreen renderer.\n \n\n The models managed by this offscreen renderer.\n\nC++: easy3d::OffScreen::models() const --> const class std::vector<class easy3d::Model *> &", pybind11::return_value_policy::automatic);
		cl.def("drawables", (const class std::vector<class easy3d::Drawable *> & (easy3d::OffScreen::*)() const) &easy3d::OffScreen::drawables, "Query the drawables managed by this offscreen renderer.\n \n\n The drawables managed by this offscreen renderer.\n\nC++: easy3d::OffScreen::drawables() const --> const class std::vector<class easy3d::Drawable *> &", pybind11::return_value_policy::automatic);
		cl.def("clear_scene", (void (easy3d::OffScreen::*)()) &easy3d::OffScreen::clear_scene, "Delete all visual contents of the offscreen renderer (all models and drawables).\n\nC++: easy3d::OffScreen::clear_scene() --> void");
//		cl.def("assign", (class easy3d::OffScreen & (easy3d::OffScreen::*)(const class easy3d::OffScreen &)) &easy3d::OffScreen::operator=, "C++: easy3d::OffScreen::operator=(const class easy3d::OffScreen &) --> class easy3d::OffScreen &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
}
