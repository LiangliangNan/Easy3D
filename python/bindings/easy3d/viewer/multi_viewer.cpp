#include <easy3d/core/vec.h>
#include <easy3d/core/model.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable.h>
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
    easy3d::vec3 point_under_pixel(int x, int y, bool& found) const override {
        pybind11::gil_scoped_acquire gil;
        auto callback = pybind11::get_override(static_cast<const easy3d::MultiViewer*>(this), "point_under_pixel");
        if (callback) {
            auto result = callback(x, y).cast<pybind11::tuple>();
            found = result[1].cast<bool>();
            return result[0].cast<easy3d::vec3>();
        }
        return MultiViewer::point_under_pixel(x, y, found);
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
    bool draw() override {
        PYBIND11_OVERRIDE(bool,easy3d::MultiViewer,draw);
    }
    bool snapshot(const std::string& file,float scale,int samples,int bg,bool expand) const override {
        PYBIND11_OVERRIDE(bool,easy3d::MultiViewer,snapshot,file,scale,samples,bg,expand);
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
#ifndef EASY3D_HAS_VULKAN
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
#endif
};

void bind_easy3d_viewer_multi_viewer(pybind11::module_& m)
{
	{ // easy3d::MultiViewer file:easy3d/viewer/multi_viewer.h line:44
		pybind11::class_<easy3d::MultiViewer, std::shared_ptr<easy3d::MultiViewer>, PyCallBack_easy3d_MultiViewer, easy3d::Viewer> cl(m, "MultiViewer", "A viewer that supports multiple views (arranged in a grid layout).\n \n\n\n     ");
        cl.def(pybind11::init<int, int, const std::string&, int, int, int, bool, bool, bool>(),
               pybind11::arg("rows"), pybind11::arg("cols"), pybind11::arg("title") = "untitled",
               pybind11::arg("width") = 800, pybind11::arg("height") = 600, pybind11::arg("samples") = 1,
               pybind11::arg("full_screen") = false, pybind11::arg("visible") = true, pybind11::arg("resizable") = true);

        // Assign a model to specific view
        cl.def("assign", [](easy3d::MultiViewer &self, int row, int col, const easy3d::Model *model) {
                   self.assign(row, col, model);
               },
               "Assign a model to the view at position (row, col)",
               pybind11::arg("row"), pybind11::arg("col"), pybind11::arg("model")
        );
        // Assign a drawable to specific view
        cl.def("assign", [](easy3d::MultiViewer &self, int row, int col, const easy3d::Drawable *drawable) {
                   self.assign(row, col, drawable);
               },
               "Assign a drawable to the view at position (row, col)",
               pybind11::arg("row"), pybind11::arg("col"), pybind11::arg("drawable")
        );

        cl.def("set_division_visible", (void (easy3d::MultiViewer::*)(bool)) &easy3d::MultiViewer::set_division_visible, "Sets the visibility of the splitting lines of the views (visible by default).\n\nC++: easy3d::MultiViewer::set_division_visible(bool) --> void", pybind11::arg("b"));
		cl.def("division_visible", (bool (easy3d::MultiViewer::*)() const) &easy3d::MultiViewer::division_visible, "Returns if the splitting lines of the views are visible.\n\nC++: easy3d::MultiViewer::division_visible() const --> bool");
		cl.def("rows", (int (easy3d::MultiViewer::*)() const) &easy3d::MultiViewer::rows, "Return the number of rows (of the grid-like layout) of the viewer.\n\nC++: easy3d::MultiViewer::rows() const --> int");
		cl.def("columns", (int (easy3d::MultiViewer::*)() const) &easy3d::MultiViewer::columns, "Return the number of columns (of the grid-like layout) of the viewer.\n\nC++: easy3d::MultiViewer::columns() const --> int");
		cl.def("set_layout", (void (easy3d::MultiViewer::*)(int, int)) &easy3d::MultiViewer::set_layout, "Set/Change the layout of the viewer.\n \n\n The number of rows (of the grid-like layout).\n \n\n The number of columns (of the grid-like layout).\n\nC++: easy3d::MultiViewer::set_layout(int, int) --> void", pybind11::arg("rows"), pybind11::arg("cols"));
        cl.def("camera", (class easy3d::Camera* (easy3d::MultiViewer::*)()) &easy3d::MultiViewer::camera, "Returns the camera used by the viewer", pybind11::return_value_policy::reference_internal);
        cl.def("camera", (const class easy3d::Camera* (easy3d::MultiViewer::*)() const) &easy3d::MultiViewer::camera, "Returns the camera used by the viewer", pybind11::return_value_policy::reference_internal);
        cl.def("snapshot", (bool (easy3d::MultiViewer::*)() const) &easy3d::MultiViewer::snapshot, "Take a snapshot of the screen and save it to a file.\n \n\n This method takes a snapshot of the screen and saves the snapshot into an image file.\n          Internally, it will pop up a file dialog for specifying the file name.\n \n\n true on success and false otherwise.\n\nC++: easy3d::MultiViewer::snapshot() const --> bool");
        cl.def("snapshot",static_cast<bool (easy3d::MultiViewer::*)(const std::string&,float,int,int,bool) const>(&easy3d::MultiViewer::snapshot),
               pybind11::arg("file_name"),pybind11::arg("scaling")=1.f,pybind11::arg("samples")=4,
               pybind11::arg("back_ground")=1,pybind11::arg("expand")=true);

	}

}
