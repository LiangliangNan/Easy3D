#include <easy3d/core/box.h>
#include <easy3d/core/mat.h>
#include <easy3d/core/model.h>
#include <easy3d/core/quat.h>
#include <easy3d/core/vec.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/constraint.h>
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/manipulator.h>
#include <easy3d/renderer/renderer.h>
#include <memory>
#include <ostream>
#include <sstream> // __str__
#include <string>
#include <string_view>
#include <vector>

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

// easy3d::ManipulatedFrame file:easy3d/renderer/manipulated_frame.h line:109
struct PyCallBack_easy3d_ManipulatedFrame : public easy3d::ManipulatedFrame {
	using easy3d::ManipulatedFrame::ManipulatedFrame;

	void action_start() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::ManipulatedFrame *>(this), "action_start");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return ManipulatedFrame::action_start();
	}
	void action_end() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::ManipulatedFrame *>(this), "action_end");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return ManipulatedFrame::action_end();
	}
	void action_rotate(int a0, int a1, int a2, int a3, class easy3d::Camera * a4, enum easy3d::ManipulatedFrame::ScreenAxis a5) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::ManipulatedFrame *>(this), "action_rotate");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3, a4, a5);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return ManipulatedFrame::action_rotate(a0, a1, a2, a3, a4, a5);
	}
	void action_translate(int a0, int a1, int a2, int a3, class easy3d::Camera * a4, enum easy3d::ManipulatedFrame::ScreenAxis a5) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::ManipulatedFrame *>(this), "action_translate");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3, a4, a5);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return ManipulatedFrame::action_translate(a0, a1, a2, a3, a4, a5);
	}
	void action_zoom(int a0, class easy3d::Camera * a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::ManipulatedFrame *>(this), "action_zoom");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return ManipulatedFrame::action_zoom(a0, a1);
	}
};

// easy3d::ManipulatedCameraFrame file:easy3d/renderer/manipulated_camera_frame.h line:68
struct PyCallBack_easy3d_ManipulatedCameraFrame : public easy3d::ManipulatedCameraFrame {
	using easy3d::ManipulatedCameraFrame::ManipulatedCameraFrame;

	void action_rotate(int a0, int a1, int a2, int a3, class easy3d::Camera * a4, enum easy3d::ManipulatedFrame::ScreenAxis a5) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::ManipulatedCameraFrame *>(this), "action_rotate");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3, a4, a5);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return ManipulatedCameraFrame::action_rotate(a0, a1, a2, a3, a4, a5);
	}
	void action_translate(int a0, int a1, int a2, int a3, class easy3d::Camera * a4, enum easy3d::ManipulatedFrame::ScreenAxis a5) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::ManipulatedCameraFrame *>(this), "action_translate");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1, a2, a3, a4, a5);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return ManipulatedCameraFrame::action_translate(a0, a1, a2, a3, a4, a5);
	}
	void action_zoom(int a0, class easy3d::Camera * a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::ManipulatedCameraFrame *>(this), "action_zoom");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return ManipulatedCameraFrame::action_zoom(a0, a1);
	}
	void action_turn(float a0, class easy3d::Camera * a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::ManipulatedCameraFrame *>(this), "action_turn");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return ManipulatedCameraFrame::action_turn(a0, a1);
	}
	void action_start() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::ManipulatedCameraFrame *>(this), "action_start");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return ManipulatedFrame::action_start();
	}
	void action_end() override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::ManipulatedCameraFrame *>(this), "action_end");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>();
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return ManipulatedFrame::action_end();
	}
};

void bind_easy3d_renderer_manipulated_frame(pybind11::module_& m)
{
	{ // easy3d::ManipulatedFrame file:easy3d/renderer/manipulated_frame.h line:109
		pybind11::class_<easy3d::ManipulatedFrame, std::shared_ptr<easy3d::ManipulatedFrame>, PyCallBack_easy3d_ManipulatedFrame, easy3d::Frame> cl(m, "ManipulatedFrame", "A Frame that can be rotated and translated using the mouse.\n \n\n\n \n A ManipulatedFrame converts the mouse motion into a translation and an orientation updates.\n A ManipulatedFrame is used to move an object in the scene. Combined with object selection, its MouseGrabber\n properties and a dynamic update of the scene, the ManipulatedFrame introduces a great reactivity in your\n applications.\n\n	  A ManipulatedFrame is attached to a QGLViewer using\n	  QGLViewer::setManipulatedFrame(): \n\n\n\n\n\n\n\n\n\n\n\n	  See the manipulatedFrame\n	  example for a complete application.\n\n	  Mouse events are normally sent to the QGLViewer::camera(). You have to press\n	  the QGLViewer::FRAME state key (default is  to move the\n	  QGLViewer::manipulatedFrame() instead. See the mouse\n	  page for a description of mouse button bindings.\n\n	  Inherited functionalities\n\n	  A ManipulatedFrame is an overloaded instance of a Frame. The powerful\n	  coordinate system transformation functions (Frame::coordinatesOf(),\n	  Frame::transformOf(), ...) can hence be applied to a ManipulatedFrame.\n\n	  A ManipulatedFrame is also a MouseGrabber. If the mouse cursor gets within a\n	  distance of 10 pixels from the projected position of the ManipulatedFrame, the\n	  ManipulatedFrame becomes the new QGLViewer::mouseGrabber(). It can then be\n	  manipulated directly, without any specific state key, object selection or GUI\n	  intervention. This is very convenient to directly move some objects in the\n	  scene (typically a light). See the \nmouseGrabber example as an\n	  illustration. Note that QWidget::setMouseTracking() needs to be enabled in\n	  order to use this feature (see the MouseGrabber documentation).\n\n	  Advanced functionalities\n\n	  A QGLViewer can handle at most one ManipulatedFrame at a time. If you want to\n	  move several objects in the scene, you simply have to keep a list of the\n	  different ManipulatedFrames, and to activate the right one (using\n	  QGLViewer::setManipulatedFrame()) when needed. This can for instance be done\n	  according to an object selection: see the luxo\n	  example for an illustration.\n\n	  When the ManipulatedFrame is being manipulated using the mouse (mouse pressed\n	  and not yet released), isManipulated() returns  This might be used to\n	  trigger a specific action or display (as is done with QGLViewer::fastDraw()).\n\n	  The ManipulatedFrame also emits a manipulated() signal each time its state is\n	  modified by the mouse. This signal is automatically connected to the\n	  QGLViewer::update() slot when the ManipulatedFrame is attached to a viewer\n	  using QGLViewer::setManipulatedFrame().");
		cl.def( pybind11::init( [](){ return new easy3d::ManipulatedFrame(); }, [](){ return new PyCallBack_easy3d_ManipulatedFrame(); } ) );
		cl.def( pybind11::init( [](PyCallBack_easy3d_ManipulatedFrame const &o){ return new PyCallBack_easy3d_ManipulatedFrame(o); } ) );
		cl.def( pybind11::init( [](easy3d::ManipulatedFrame const &o){ return new easy3d::ManipulatedFrame(o); } ) );

		pybind11::enum_<easy3d::ManipulatedFrame::ScreenAxis>(cl, "ScreenAxis", pybind11::arithmetic(), "")
			.value("NONE", easy3d::ManipulatedFrame::NONE)
			.value("HORIZONTAL", easy3d::ManipulatedFrame::HORIZONTAL)
			.value("VERTICAL", easy3d::ManipulatedFrame::VERTICAL)
			.value("ORTHOGONAL", easy3d::ManipulatedFrame::ORTHOGONAL)
			.export_values();

		cl.def("assign", (class easy3d::ManipulatedFrame & (easy3d::ManipulatedFrame::*)(const class easy3d::ManipulatedFrame &)) &easy3d::ManipulatedFrame::operator=, "C++: easy3d::ManipulatedFrame::operator=(const class easy3d::ManipulatedFrame &) --> class easy3d::ManipulatedFrame &", pybind11::return_value_policy::automatic, pybind11::arg("mf"));
		cl.def("setRotationSensitivity", (void (easy3d::ManipulatedFrame::*)(float)) &easy3d::ManipulatedFrame::setRotationSensitivity, "Defines the rotationSensitivity(). \n\nC++: easy3d::ManipulatedFrame::setRotationSensitivity(float) --> void", pybind11::arg("sensitivity"));
		cl.def("setTranslationSensitivity", (void (easy3d::ManipulatedFrame::*)(float)) &easy3d::ManipulatedFrame::setTranslationSensitivity, "Defines the translationSensitivity(). \n\nC++: easy3d::ManipulatedFrame::setTranslationSensitivity(float) --> void", pybind11::arg("sensitivity"));
		cl.def("setWheelSensitivity", (void (easy3d::ManipulatedFrame::*)(float)) &easy3d::ManipulatedFrame::setWheelSensitivity, "Defines the wheelSensitivity(). \n\nC++: easy3d::ManipulatedFrame::setWheelSensitivity(float) --> void", pybind11::arg("sensitivity"));
		cl.def("setZoomSensitivity", (void (easy3d::ManipulatedFrame::*)(float)) &easy3d::ManipulatedFrame::setZoomSensitivity, "Defines the zoomSensitivity(). \n\nC++: easy3d::ManipulatedFrame::setZoomSensitivity(float) --> void", pybind11::arg("sensitivity"));
		cl.def("rotationSensitivity", (float (easy3d::ManipulatedFrame::*)() const) &easy3d::ManipulatedFrame::rotationSensitivity, "Returns the influence of a mouse displacement on the ManipulatedFrame\n		rotation.\n\n		Default value is 1.0. With an identical mouse displacement, a higher value\n		will generate a larger rotation (and inversely for lower values). A 0.0 value\n		will forbid ManipulatedFrame mouse rotation (see also constraint()).\n\n		See also setRotationSensitivity(), translationSensitivity(),\n		spinningSensitivity() and wheelSensitivity(). \n\nC++: easy3d::ManipulatedFrame::rotationSensitivity() const --> float");
		cl.def("translationSensitivity", (float (easy3d::ManipulatedFrame::*)() const) &easy3d::ManipulatedFrame::translationSensitivity, "Returns the influence of a mouse displacement on the ManipulatedFrame\n		translation.\n\n		Default value is 1.0. You should not have to modify this value, since with 1.0\n		the ManipulatedFrame precisely stays under the mouse cursor.\n\n		With an identical mouse displacement, a higher value will generate a larger\n		translation (and inversely for lower values). A 0.0 value will forbid\n		ManipulatedFrame mouse translation (see also constraint()).\n\n		\n When the ManipulatedFrame is used to move a  (see the\n		ManipulatedCameraFrame class documentation), after zooming on a small region\n		of your scene, the camera may translate too fast. For a camera, it is the\n		Camera::pivotPoint() that exactly matches the mouse displacement. Hence,\n		instead of changing the translationSensitivity(), solve the problem by\n		(temporarily) setting the Camera::pivotPoint() to a point on the zoomed region\n		(see the QGLViewer::RAP_FROM_PIXEL mouse binding in the \nmouse page).\n\n		See also setTranslationSensitivity(), rotationSensitivity(),\n		spinningSensitivity() and wheelSensitivity(). \n\nC++: easy3d::ManipulatedFrame::translationSensitivity() const --> float");
		cl.def("zoomSensitivity", (float (easy3d::ManipulatedFrame::*)() const) &easy3d::ManipulatedFrame::zoomSensitivity, "Returns the zoom sensitivity.\n\n		Default value is 1.0. A higher value will make the zoom faster.\n		Use a negative value to invert the zoom in and out directions.\n\n		See also setZoomSensitivity(), translationSensitivity(), rotationSensitivity()\n		wheelSensitivity() and spinningSensitivity(). \n\nC++: easy3d::ManipulatedFrame::zoomSensitivity() const --> float");
		cl.def("wheelSensitivity", (float (easy3d::ManipulatedFrame::*)() const) &easy3d::ManipulatedFrame::wheelSensitivity, "Returns the mouse wheel sensitivity.\n\n		Default value is 1.0. A higher value will make the wheel action more efficient\n		(usually meaning a faster zoom). Use a negative value to invert the zoom in\n		and out directions.\n\n		See also setWheelSensitivity(), translationSensitivity(),\n		rotationSensitivity() zoomSensitivity() and spinningSensitivity(). \n\nC++: easy3d::ManipulatedFrame::wheelSensitivity() const --> float");
		cl.def("action_start", (void (easy3d::ManipulatedFrame::*)()) &easy3d::ManipulatedFrame::action_start, "Initiates the ManipulatedFrame mouse manipulation.\n \n\n This function should be called when an action (e.g., drag) starts, e.g., mouse pressed.\n\nC++: easy3d::ManipulatedFrame::action_start() --> void");
		cl.def("action_end", (void (easy3d::ManipulatedFrame::*)()) &easy3d::ManipulatedFrame::action_end, "Stops the ManipulatedFrame mouse manipulation.\n \n\n This function should be called when an action (e.g., drag) ends, e.g., mouse released.\n\nC++: easy3d::ManipulatedFrame::action_end() --> void");
		cl.def("action_rotate", (void (easy3d::ManipulatedFrame::*)(int, int, int, int, class easy3d::Camera *, enum easy3d::ManipulatedFrame::ScreenAxis)) &easy3d::ManipulatedFrame::action_rotate, "C++: easy3d::ManipulatedFrame::action_rotate(int, int, int, int, class easy3d::Camera *, enum easy3d::ManipulatedFrame::ScreenAxis) --> void", pybind11::arg("mouse_x"), pybind11::arg("mouse_y"), pybind11::arg("mouse_dx"), pybind11::arg("mouse_dy"), pybind11::arg("camera"), pybind11::arg("axis"));
		cl.def("action_translate", (void (easy3d::ManipulatedFrame::*)(int, int, int, int, class easy3d::Camera *, enum easy3d::ManipulatedFrame::ScreenAxis)) &easy3d::ManipulatedFrame::action_translate, "C++: easy3d::ManipulatedFrame::action_translate(int, int, int, int, class easy3d::Camera *, enum easy3d::ManipulatedFrame::ScreenAxis) --> void", pybind11::arg("mouse_x"), pybind11::arg("mouse_y"), pybind11::arg("mouse_dx"), pybind11::arg("mouse_dy"), pybind11::arg("camera"), pybind11::arg("axis"));
		cl.def("action_zoom", (void (easy3d::ManipulatedFrame::*)(int, class easy3d::Camera *)) &easy3d::ManipulatedFrame::action_zoom, "C++: easy3d::ManipulatedFrame::action_zoom(int, class easy3d::Camera *) --> void", pybind11::arg("dy_wheel"), pybind11::arg("camera"));
	}
	{ // easy3d::ManipulatedCameraFrame file:easy3d/renderer/manipulated_camera_frame.h line:68
		pybind11::class_<easy3d::ManipulatedCameraFrame, std::shared_ptr<easy3d::ManipulatedCameraFrame>, PyCallBack_easy3d_ManipulatedCameraFrame, easy3d::ManipulatedFrame> cl(m, "ManipulatedCameraFrame", "A manipulated frame with camera specific mouse bindings.\n      \n\n\n      \n A ManipulatedCameraFrame is a specialization of a ManipulatedFrame, designed\n      to be set as the Camera::frame(). Mouse motions are basically interpreted in a\n      negated way: when the mouse goes to the right, the ManipulatedFrame\n      translation goes to the right, while the ManipulatedCameraFrame has to go to\n      the  so that the  seems to move to the right.\n\n      A ManipulatedCameraFrame rotates around its pivotPoint(), which corresponds to\n      the associated Camera::pivotPoint().\n\n      A ManipulatedCameraFrame can also \"fly\" in the scene. It basically moves\n      forward, and turns according to the mouse motion. See flySpeed(),\n      sceneUpVector() and the QGLViewer::MOVE_FORWARD and QGLViewer::MOVE_BACKWARD\n      QGLViewer::MouseAction.\n\n      See the mouse page for a description of the\n      possible actions that can be performed using the mouse and their bindings.");
		cl.def( pybind11::init( [](){ return new easy3d::ManipulatedCameraFrame(); }, [](){ return new PyCallBack_easy3d_ManipulatedCameraFrame(); } ) );
		cl.def( pybind11::init( [](PyCallBack_easy3d_ManipulatedCameraFrame const &o){ return new PyCallBack_easy3d_ManipulatedCameraFrame(o); } ) );
		cl.def( pybind11::init( [](easy3d::ManipulatedCameraFrame const &o){ return new easy3d::ManipulatedCameraFrame(o); } ) );
		cl.def("assign", (class easy3d::ManipulatedCameraFrame & (easy3d::ManipulatedCameraFrame::*)(const class easy3d::ManipulatedCameraFrame &)) &easy3d::ManipulatedCameraFrame::operator=, "C++: easy3d::ManipulatedCameraFrame::operator=(const class easy3d::ManipulatedCameraFrame &) --> class easy3d::ManipulatedCameraFrame &", pybind11::return_value_policy::automatic, pybind11::arg("mcf"));
		cl.def("pivotPoint", (class easy3d::Vec<3, float> (easy3d::ManipulatedCameraFrame::*)() const) &easy3d::ManipulatedCameraFrame::pivotPoint, "Returns the point the ManipulatedCameraFrame pivot point, around which the\n        camera rotates.\n\n        It is defined in the world coordinate system. Default value is (0,0,0).\n\n        When the ManipulatedCameraFrame is associated to a Camera,\n        Camera::pivotPoint() also returns this value. This point can interactively be\n        changed using the mouse (see Camera::setPivotPointFromPixel() and\n        QGLViewer::RAP_FROM_PIXEL and QGLViewer::RAP_IS_CENTER in the \nmouse page). \n\nC++: easy3d::ManipulatedCameraFrame::pivotPoint() const --> class easy3d::Vec<3, float>");
		cl.def("setPivotPoint", (void (easy3d::ManipulatedCameraFrame::*)(const class easy3d::Vec<3, float> &)) &easy3d::ManipulatedCameraFrame::setPivotPoint, "Sets the pivotPoint(), defined in the world coordinate system. \n\nC++: easy3d::ManipulatedCameraFrame::setPivotPoint(const class easy3d::Vec<3, float> &) --> void", pybind11::arg("point"));
		cl.def("zoomsOnPivotPoint", (bool (easy3d::ManipulatedCameraFrame::*)() const) &easy3d::ManipulatedCameraFrame::zoomsOnPivotPoint, "Returns whether or not the QGLViewer::ZOOM action zooms on the pivot\n          point.\n\n          When set to  (default), a zoom action will move the camera along its\n          Camera::viewDirection(), i.e. back and forth along a direction perpendicular\n          to the projection screen.\n\n          setZoomsOnPivotPoint() to  will move the camera along an axis defined\n          by the Camera::pivotPoint() and its current position instead. As a result,\n          the projected position of the pivot point on screen will stay the same\n          during a zoom. \n\nC++: easy3d::ManipulatedCameraFrame::zoomsOnPivotPoint() const --> bool");
		cl.def("setZoomsOnPivotPoint", (void (easy3d::ManipulatedCameraFrame::*)(bool)) &easy3d::ManipulatedCameraFrame::setZoomsOnPivotPoint, "Sets the value of zoomsOnPivotPoint(). Default value is false. \n\nC++: easy3d::ManipulatedCameraFrame::setZoomsOnPivotPoint(bool) --> void", pybind11::arg("enabled"));
		cl.def("action_rotate", (void (easy3d::ManipulatedCameraFrame::*)(int, int, int, int, class easy3d::Camera *, enum easy3d::ManipulatedFrame::ScreenAxis)) &easy3d::ManipulatedCameraFrame::action_rotate, "C++: easy3d::ManipulatedCameraFrame::action_rotate(int, int, int, int, class easy3d::Camera *, enum easy3d::ManipulatedFrame::ScreenAxis) --> void", pybind11::arg("mouse_x"), pybind11::arg("mouse_y"), pybind11::arg("mouse_dx"), pybind11::arg("mouse_dy"), pybind11::arg("camera"), pybind11::arg("axis"));
		cl.def("action_translate", (void (easy3d::ManipulatedCameraFrame::*)(int, int, int, int, class easy3d::Camera *, enum easy3d::ManipulatedFrame::ScreenAxis)) &easy3d::ManipulatedCameraFrame::action_translate, "C++: easy3d::ManipulatedCameraFrame::action_translate(int, int, int, int, class easy3d::Camera *, enum easy3d::ManipulatedFrame::ScreenAxis) --> void", pybind11::arg("mouse_x"), pybind11::arg("mouse_y"), pybind11::arg("mouse_dx"), pybind11::arg("mouse_dy"), pybind11::arg("camera"), pybind11::arg("axis"));
		cl.def("action_zoom", (void (easy3d::ManipulatedCameraFrame::*)(int, class easy3d::Camera *)) &easy3d::ManipulatedCameraFrame::action_zoom, "C++: easy3d::ManipulatedCameraFrame::action_zoom(int, class easy3d::Camera *) --> void", pybind11::arg("dy_wheel"), pybind11::arg("camera"));
		cl.def("action_turn", (void (easy3d::ManipulatedCameraFrame::*)(float, class easy3d::Camera *)) &easy3d::ManipulatedCameraFrame::action_turn, "C++: easy3d::ManipulatedCameraFrame::action_turn(float, class easy3d::Camera *) --> void", pybind11::arg("angle_radian"), pybind11::arg("camera"));
	}
	{ // easy3d::Manipulator file:easy3d/renderer/manipulator.h line:62
		pybind11::class_<easy3d::Manipulator, std::shared_ptr<easy3d::Manipulator>> cl(m, "Manipulator", "A manipulator is for manipulation of an object.\n \n\n A manipulator maintains the manipulated frame of an object. The manipulated frame is used to\n      manipulate this object. In this implementation, the origin of the manipulated frame is always at\n      the center of the object.\n \n\n\n The following code shows how to use this manipulator:\n \n\n\n\n\n\n\n\n\n\n \n\n\n\n     ");
		cl.def( pybind11::init( [](){ return new easy3d::Manipulator(); } ), "doc" );
		cl.def( pybind11::init<class easy3d::Model *>(), pybind11::arg("model") );

		cl.def("reset", (void (easy3d::Manipulator::*)()) &easy3d::Manipulator::reset, "Resets the manipulated frame, i.e., no transformation.\n\nC++: easy3d::Manipulator::reset() --> void");
		cl.def("frame", (class easy3d::ManipulatedFrame * (easy3d::Manipulator::*)()) &easy3d::Manipulator::frame, "Returns the manipulated frame.\n\nC++: easy3d::Manipulator::frame() --> class easy3d::ManipulatedFrame *", pybind11::return_value_policy::automatic);
		cl.def("matrix", (class easy3d::Mat4<float> (easy3d::Manipulator::*)() const) &easy3d::Manipulator::matrix, "Returns the transformation introduced by this manipulator.\n \n\n Rotation is performed around object 'center'. Thus the returned transformation is different from\n     'frame()->matrix()'. Their relation is: 'matrix() == frame()->matrix() * mat4::translation(-center)'.\n\nC++: easy3d::Manipulator::matrix() const --> class easy3d::Mat4<float>");
		cl.def("draw_frame", (void (easy3d::Manipulator::*)(const class easy3d::Camera *) const) &easy3d::Manipulator::draw_frame, "Draws the manipulated frame.\n\nC++: easy3d::Manipulator::draw_frame(const class easy3d::Camera *) const --> void", pybind11::arg("cam"));
	}
}
