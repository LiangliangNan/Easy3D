#include <easy3d/core/mat.h>
#include <easy3d/core/quat.h>
#include <easy3d/core/vec.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/constraint.h>
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <sstream> // __str__

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

// easy3d::WorldConstraint file:easy3d/renderer/constraint.h line:349
struct PyCallBack_easy3d_WorldConstraint : public easy3d::WorldConstraint {
	using easy3d::WorldConstraint::WorldConstraint;

	void constrainTranslation(class easy3d::Vec<3, float> & a0, class easy3d::Frame *const a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::WorldConstraint *>(this), "constrainTranslation");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return WorldConstraint::constrainTranslation(a0, a1);
	}
	void constrainRotation(class easy3d::Quat<float> & a0, class easy3d::Frame *const a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::WorldConstraint *>(this), "constrainRotation");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return WorldConstraint::constrainRotation(a0, a1);
	}
};

// easy3d::CameraConstraint file:easy3d/renderer/constraint.h line:367
struct PyCallBack_easy3d_CameraConstraint : public easy3d::CameraConstraint {
	using easy3d::CameraConstraint::CameraConstraint;

	void constrainTranslation(class easy3d::Vec<3, float> & a0, class easy3d::Frame *const a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::CameraConstraint *>(this), "constrainTranslation");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return CameraConstraint::constrainTranslation(a0, a1);
	}
	void constrainRotation(class easy3d::Quat<float> & a0, class easy3d::Frame *const a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::CameraConstraint *>(this), "constrainRotation");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return CameraConstraint::constrainRotation(a0, a1);
	}
};

void bind_easy3d_renderer_constraint(pybind11::module_& m)
{
	{ // easy3d::WorldConstraint file:easy3d/renderer/constraint.h line:349
		pybind11::class_<easy3d::WorldConstraint, std::shared_ptr<easy3d::WorldConstraint>, PyCallBack_easy3d_WorldConstraint, easy3d::AxisPlaneConstraint> cl(m, "WorldConstraint", "An AxisPlaneConstraint defined in the world coordinate system.\n			\n\n\n	  \n The translationConstraintDirection() and rotationConstraintDirection() are\n	  expressed in world coordinate system.\n\n	  See the constrainedFrame and\n	  multiView examples for an\n	  illustration. ");
		cl.def( pybind11::init( [](){ return new easy3d::WorldConstraint(); }, [](){ return new PyCallBack_easy3d_WorldConstraint(); } ) );
		cl.def( pybind11::init( [](PyCallBack_easy3d_WorldConstraint const &o){ return new PyCallBack_easy3d_WorldConstraint(o); } ) );
		cl.def( pybind11::init( [](easy3d::WorldConstraint const &o){ return new easy3d::WorldConstraint(o); } ) );
		cl.def("constrainTranslation", (void (easy3d::WorldConstraint::*)(class easy3d::Vec<3, float> &, class easy3d::Frame *const)) &easy3d::WorldConstraint::constrainTranslation, "C++: easy3d::WorldConstraint::constrainTranslation(class easy3d::Vec<3, float> &, class easy3d::Frame *const) --> void", pybind11::arg("translation"), pybind11::arg("frame"));
		cl.def("constrainRotation", (void (easy3d::WorldConstraint::*)(class easy3d::Quat<float> &, class easy3d::Frame *const)) &easy3d::WorldConstraint::constrainRotation, "C++: easy3d::WorldConstraint::constrainRotation(class easy3d::Quat<float> &, class easy3d::Frame *const) --> void", pybind11::arg("rotation"), pybind11::arg("frame"));
		cl.def("assign", (class easy3d::WorldConstraint & (easy3d::WorldConstraint::*)(const class easy3d::WorldConstraint &)) &easy3d::WorldConstraint::operator=, "C++: easy3d::WorldConstraint::operator=(const class easy3d::WorldConstraint &) --> class easy3d::WorldConstraint &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
	{ // easy3d::CameraConstraint file:easy3d/renderer/constraint.h line:367
		pybind11::class_<easy3d::CameraConstraint, std::shared_ptr<easy3d::CameraConstraint>, PyCallBack_easy3d_CameraConstraint, easy3d::AxisPlaneConstraint> cl(m, "CameraConstraint", "An AxisPlaneConstraint defined in the camera coordinate system.\n	  \n\n\n	  \n The translationConstraintDirection() and rotationConstraintDirection() are\n	  expressed in the associated camera() coordinate system.\n\n	  See the constrainedFrame and\n	  constrainedCamera examples\n	  for an illustration. ");
		cl.def( pybind11::init<const class easy3d::Camera *const>(), pybind11::arg("camera") );

		cl.def( pybind11::init( [](PyCallBack_easy3d_CameraConstraint const &o){ return new PyCallBack_easy3d_CameraConstraint(o); } ) );
		cl.def( pybind11::init( [](easy3d::CameraConstraint const &o){ return new easy3d::CameraConstraint(o); } ) );
		cl.def("constrainTranslation", (void (easy3d::CameraConstraint::*)(class easy3d::Vec<3, float> &, class easy3d::Frame *const)) &easy3d::CameraConstraint::constrainTranslation, "C++: easy3d::CameraConstraint::constrainTranslation(class easy3d::Vec<3, float> &, class easy3d::Frame *const) --> void", pybind11::arg("translation"), pybind11::arg("frame"));
		cl.def("constrainRotation", (void (easy3d::CameraConstraint::*)(class easy3d::Quat<float> &, class easy3d::Frame *const)) &easy3d::CameraConstraint::constrainRotation, "C++: easy3d::CameraConstraint::constrainRotation(class easy3d::Quat<float> &, class easy3d::Frame *const) --> void", pybind11::arg("rotation"), pybind11::arg("frame"));
		cl.def("camera", (const class easy3d::Camera * (easy3d::CameraConstraint::*)() const) &easy3d::CameraConstraint::camera, "Returns the associated Camera. Set using the CameraConstraint constructor.\n\nC++: easy3d::CameraConstraint::camera() const --> const class easy3d::Camera *", pybind11::return_value_policy::automatic);
	}
	{ // easy3d::Frame file:easy3d/renderer/frame.h line:152
		pybind11::class_<easy3d::Frame, std::shared_ptr<easy3d::Frame>> cl(m, "Frame", "The Frame class represents a coordinate system, defined by a position\n      and an orientation.\n      \n\n\n      \n A Frame is a 3D coordinate system, represented by a position() and an\n      orientation(). The order of these transformations is important: the Frame is\n      first translated   rotated around the new translated origin.\n\n      A Frame is useful to define the position and orientation of a 3D rigid object,\n      using its matrix() method, as shown below: \n\n\n\n\n\n\n\n\n\n\n      Many functions are provided to transform a 3D point from one coordinate system\n      (Frame) to an other: see coordinatesOf(), inverseCoordinatesOf(),\n      coordinatesOfIn(), coordinatesOfFrom()...\n\n      You may also want to transform a 3D vector (such as a normal), which\n      corresponds to applying only the rotational part of the frame transformation:\n      see transformOf() and inverseTransformOf(). See the \nframeTransform example for an\n      illustration.\n\n      The translation() and the rotation() that are encapsulated in a Frame can also\n      be used to represent a   of space. Such a\n      transformation can also be interpreted as a change of coordinate system, and\n      the coordinate system conversion functions actually allow you to use a Frame\n      as a rigid transformation. Use inverseCoordinatesOf() (resp. coordinatesOf())\n      to apply the transformation (resp. its inverse). Note the inversion.\n\n      Hierarchy of Frames\n\n      The position and the orientation of a Frame are actually defined with respect\n      to a referenceFrame(). The default referenceFrame() is the world coordinate\n      system (represented by a  referenceFrame()). If you setReferenceFrame()\n      to a different Frame, you must then differentiate:\n\n      \n the  translation() and rotation(), defined with respect to the\n      referenceFrame(),\n\n      \n the  position() and orientation(), always defined with respect\n      to the world coordinate system.\n\n      A Frame is actually defined by its translation() with respect to its\n      referenceFrame(), and then by a rotation() of the coordinate system around the\n      new translated origin.\n\n      This terminology for  (translation() and rotation()) and \n      (position() and orientation()) definitions is used in all the methods' names\n      and should be sufficient to prevent ambiguities. These notions are obviously\n      identical when the referenceFrame() is  i.e. when the Frame is defined\n      in the world coordinate system (the one you are in at the beginning of the\n      Viewer::draw() method, see the introduction\n      page).\n\n      Frames can hence easily be organized in a tree hierarchy, which root is the\n      world coordinate system. A loop in the hierarchy would result in an\n      inconsistent (multiple) Frame definition.\n      settingAsReferenceFrameWillCreateALoop() checks this and prevents\n      setReferenceFrame() from creating such a loop.\n\n      This frame hierarchy is used in methods like coordinatesOfIn(),\n      coordinatesOfFrom()... which allow coordinates (or vector) conversions from a\n      Frame to any other one (including the world coordinate system).\n\n      However, one must note that this hierarchical representation is internal to\n      the Frame classes. When the Frames represent OpenGL coordinates system, one\n      should map this hierarchical representation to the OpenGL GL_MODELVIEW matrix\n      stack. See the matrix() documentation for details.\n\n      Constraints\n\n      An interesting feature of Frames is that their displacements can be\n      constrained. When a Constraint is attached to a Frame, it filters the input of\n      translate() and rotate(), and only the resulting filtered motion is applied to\n      the Frame. The default constraint() is  resulting in no filtering. Use\n      setConstraint() to attach a Constraint to a frame.\n\n      Constraints are especially useful for the ManipulatedFrame instances, in\n      order to forbid some mouse motions. See the \nconstrainedFrame, \nconstrainedCamera and \nluxo examples for an illustration.\n\n      Classical constraints are provided for convenience (see LocalConstraint,\n      WorldConstraint and CameraConstraint) and new constraints can very easily be\n      implemented.\n\n      Derived classes\n\n      The ManipulatedFrame class inherits Frame and implements a mouse motion\n      conversion, so that a Frame (and hence an object) can be manipulated in the\n      scene with the mouse.\n\n      ");
		cl.def( pybind11::init( [](){ return new easy3d::Frame(); } ) );
		cl.def( pybind11::init( [](easy3d::Frame const &o){ return new easy3d::Frame(o); } ) );
		cl.def( pybind11::init<const class easy3d::Vec<3, float> &, const class easy3d::Quat<float> &>(), pybind11::arg("position"), pybind11::arg("orientation") );

		cl.def_readwrite("modified", &easy3d::Frame::modified);
		cl.def("assign", (class easy3d::Frame & (easy3d::Frame::*)(const class easy3d::Frame &)) &easy3d::Frame::operator=, "C++: easy3d::Frame::operator=(const class easy3d::Frame &) --> class easy3d::Frame &", pybind11::return_value_policy::automatic, pybind11::arg("frame"));
		cl.def("setPosition", (void (easy3d::Frame::*)(const class easy3d::Vec<3, float> &)) &easy3d::Frame::setPosition, "C++: easy3d::Frame::setPosition(const class easy3d::Vec<3, float> &) --> void", pybind11::arg("position"));
		cl.def("setPositionWithConstraint", (void (easy3d::Frame::*)(class easy3d::Vec<3, float> &)) &easy3d::Frame::setPositionWithConstraint, "C++: easy3d::Frame::setPositionWithConstraint(class easy3d::Vec<3, float> &) --> void", pybind11::arg("position"));
		cl.def("setOrientation", (void (easy3d::Frame::*)(const class easy3d::Quat<float> &)) &easy3d::Frame::setOrientation, "C++: easy3d::Frame::setOrientation(const class easy3d::Quat<float> &) --> void", pybind11::arg("orientation"));
		cl.def("setOrientationWithConstraint", (void (easy3d::Frame::*)(class easy3d::Quat<float> &)) &easy3d::Frame::setOrientationWithConstraint, "C++: easy3d::Frame::setOrientationWithConstraint(class easy3d::Quat<float> &) --> void", pybind11::arg("orientation"));
		cl.def("setPositionAndOrientation", (void (easy3d::Frame::*)(const class easy3d::Vec<3, float> &, const class easy3d::Quat<float> &)) &easy3d::Frame::setPositionAndOrientation, "C++: easy3d::Frame::setPositionAndOrientation(const class easy3d::Vec<3, float> &, const class easy3d::Quat<float> &) --> void", pybind11::arg("position"), pybind11::arg("orientation"));
		cl.def("setPositionAndOrientationWithConstraint", (void (easy3d::Frame::*)(class easy3d::Vec<3, float> &, class easy3d::Quat<float> &)) &easy3d::Frame::setPositionAndOrientationWithConstraint, "C++: easy3d::Frame::setPositionAndOrientationWithConstraint(class easy3d::Vec<3, float> &, class easy3d::Quat<float> &) --> void", pybind11::arg("position"), pybind11::arg("orientation"));
		cl.def("position", (class easy3d::Vec<3, float> (easy3d::Frame::*)() const) &easy3d::Frame::position, "C++: easy3d::Frame::position() const --> class easy3d::Vec<3, float>");
		cl.def("orientation", (class easy3d::Quat<float> (easy3d::Frame::*)() const) &easy3d::Frame::orientation, "C++: easy3d::Frame::orientation() const --> class easy3d::Quat<float>");
		cl.def("setTranslation", (void (easy3d::Frame::*)(const class easy3d::Vec<3, float> &)) &easy3d::Frame::setTranslation, "Sets the translation() of the frame, locally defined with respect to the\n        referenceFrame(). Emits the modified() signal.\n\n        Use setPosition() to define the world coordinates position(). Use\n        setTranslationWithConstraint() to take into account the potential constraint()\n        of the Frame. \n\nC++: easy3d::Frame::setTranslation(const class easy3d::Vec<3, float> &) --> void", pybind11::arg("translation"));
		cl.def("setTranslationWithConstraint", (void (easy3d::Frame::*)(class easy3d::Vec<3, float> &)) &easy3d::Frame::setTranslationWithConstraint, "C++: easy3d::Frame::setTranslationWithConstraint(class easy3d::Vec<3, float> &) --> void", pybind11::arg("translation"));
		cl.def("setRotation", (void (easy3d::Frame::*)(const class easy3d::Quat<float> &)) &easy3d::Frame::setRotation, "Sets the rotation() of the Frame, locally defined with respect to the\n         referenceFrame(). Emits the modified() signal.\n\n         Use setOrientation() to define the world coordinates orientation(). The\n         potential constraint() of the Frame is not taken into account, use\n         setRotationWithConstraint() instead. \n\nC++: easy3d::Frame::setRotation(const class easy3d::Quat<float> &) --> void", pybind11::arg("rotation"));
		cl.def("setRotationWithConstraint", (void (easy3d::Frame::*)(class easy3d::Quat<float> &)) &easy3d::Frame::setRotationWithConstraint, "C++: easy3d::Frame::setRotationWithConstraint(class easy3d::Quat<float> &) --> void", pybind11::arg("rotation"));
		cl.def("setTranslationAndRotation", (void (easy3d::Frame::*)(const class easy3d::Vec<3, float> &, const class easy3d::Quat<float> &)) &easy3d::Frame::setTranslationAndRotation, "C++: easy3d::Frame::setTranslationAndRotation(const class easy3d::Vec<3, float> &, const class easy3d::Quat<float> &) --> void", pybind11::arg("translation"), pybind11::arg("rotation"));
		cl.def("setTranslationAndRotationWithConstraint", (void (easy3d::Frame::*)(class easy3d::Vec<3, float> &, class easy3d::Quat<float> &)) &easy3d::Frame::setTranslationAndRotationWithConstraint, "C++: easy3d::Frame::setTranslationAndRotationWithConstraint(class easy3d::Vec<3, float> &, class easy3d::Quat<float> &) --> void", pybind11::arg("translation"), pybind11::arg("rotation"));
		cl.def("translation", (class easy3d::Vec<3, float> (easy3d::Frame::*)() const) &easy3d::Frame::translation, "Returns the Frame translation, defined with respect to the\n        referenceFrame().\n\n        Use position() to get the result in the world coordinates. These two values\n        are identical when the referenceFrame() is  (default).\n\n        See also setTranslation() and setTranslationWithConstraint(). \n\nC++: easy3d::Frame::translation() const --> class easy3d::Vec<3, float>");
		cl.def("rotation", (class easy3d::Quat<float> (easy3d::Frame::*)() const) &easy3d::Frame::rotation, "Returns the current quat orientation. See setRotation(). \n\nC++: easy3d::Frame::rotation() const --> class easy3d::Quat<float>");
		cl.def("referenceFrame", (const class easy3d::Frame * (easy3d::Frame::*)() const) &easy3d::Frame::referenceFrame, "Returns the reference Frame, in which coordinates system the Frame is\n        defined.\n\n        The translation() and rotation() of the Frame are defined with respect to the\n        referenceFrame() coordinate system. A  referenceFrame() (default value)\n        means that the Frame is defined in the world coordinate system.\n\n        Use position() and orientation() to recursively convert values along the\n        referenceFrame() chain and to get values expressed in the world coordinate\n        system. The values match when the referenceFrame() is \n\n        Use setReferenceFrame() to set this value and create a Frame hierarchy.\n        Convenient functions allow you to convert 3D coordinates from one Frame to an\n        other: see coordinatesOf(), localCoordinatesOf(), coordinatesOfIn() and their\n        inverse functions.\n\n        Vectors can also be converted using transformOf(), transformOfIn,\n        localTransformOf() and their inverse functions. \n\nC++: easy3d::Frame::referenceFrame() const --> const class easy3d::Frame *", pybind11::return_value_policy::automatic);
		cl.def("setReferenceFrame", (void (easy3d::Frame::*)(const class easy3d::Frame *const)) &easy3d::Frame::setReferenceFrame, "C++: easy3d::Frame::setReferenceFrame(const class easy3d::Frame *const) --> void", pybind11::arg("refFrame"));
		cl.def("settingAsReferenceFrameWillCreateALoop", (bool (easy3d::Frame::*)(const class easy3d::Frame *const)) &easy3d::Frame::settingAsReferenceFrameWillCreateALoop, "C++: easy3d::Frame::settingAsReferenceFrameWillCreateALoop(const class easy3d::Frame *const) --> bool", pybind11::arg("frame"));
		cl.def("translate", (void (easy3d::Frame::*)(class easy3d::Vec<3, float> &)) &easy3d::Frame::translate, "C++: easy3d::Frame::translate(class easy3d::Vec<3, float> &) --> void", pybind11::arg("t"));
		cl.def("translate", (void (easy3d::Frame::*)(const class easy3d::Vec<3, float> &)) &easy3d::Frame::translate, "C++: easy3d::Frame::translate(const class easy3d::Vec<3, float> &) --> void", pybind11::arg("t"));
		cl.def("rotate", (void (easy3d::Frame::*)(class easy3d::Quat<float> &)) &easy3d::Frame::rotate, "C++: easy3d::Frame::rotate(class easy3d::Quat<float> &) --> void", pybind11::arg("q"));
		cl.def("rotate", (void (easy3d::Frame::*)(const class easy3d::Quat<float> &)) &easy3d::Frame::rotate, "C++: easy3d::Frame::rotate(const class easy3d::Quat<float> &) --> void", pybind11::arg("q"));
		cl.def("rotateAroundPoint", (void (easy3d::Frame::*)(class easy3d::Quat<float> &, const class easy3d::Vec<3, float> &)) &easy3d::Frame::rotateAroundPoint, "C++: easy3d::Frame::rotateAroundPoint(class easy3d::Quat<float> &, const class easy3d::Vec<3, float> &) --> void", pybind11::arg("rotation"), pybind11::arg("point"));
		cl.def("rotateAroundPoint", (void (easy3d::Frame::*)(const class easy3d::Quat<float> &, const class easy3d::Vec<3, float> &)) &easy3d::Frame::rotateAroundPoint, "C++: easy3d::Frame::rotateAroundPoint(const class easy3d::Quat<float> &, const class easy3d::Vec<3, float> &) --> void", pybind11::arg("rotation"), pybind11::arg("point"));
		cl.def("alignWithFrame", [](easy3d::Frame &o, const class easy3d::Frame *const a0) -> void { return o.alignWithFrame(a0); }, "", pybind11::arg("frame"));
		cl.def("alignWithFrame", [](easy3d::Frame &o, const class easy3d::Frame *const a0, bool const & a1) -> void { return o.alignWithFrame(a0, a1); }, "", pybind11::arg("frame"), pybind11::arg("move"));
		cl.def("alignWithFrame", (void (easy3d::Frame::*)(const class easy3d::Frame *const, bool, float)) &easy3d::Frame::alignWithFrame, "C++: easy3d::Frame::alignWithFrame(const class easy3d::Frame *const, bool, float) --> void", pybind11::arg("frame"), pybind11::arg("move"), pybind11::arg("threshold"));
		cl.def("projectOnLine", (void (easy3d::Frame::*)(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &)) &easy3d::Frame::projectOnLine, "C++: easy3d::Frame::projectOnLine(const class easy3d::Vec<3, float> &, const class easy3d::Vec<3, float> &) --> void", pybind11::arg("origin"), pybind11::arg("direction"));
		cl.def("coordinatesOf", (class easy3d::Vec<3, float> (easy3d::Frame::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Frame::coordinatesOf, "C++: easy3d::Frame::coordinatesOf(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("src"));
		cl.def("inverseCoordinatesOf", (class easy3d::Vec<3, float> (easy3d::Frame::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Frame::inverseCoordinatesOf, "C++: easy3d::Frame::inverseCoordinatesOf(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("src"));
		cl.def("localCoordinatesOf", (class easy3d::Vec<3, float> (easy3d::Frame::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Frame::localCoordinatesOf, "C++: easy3d::Frame::localCoordinatesOf(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("src"));
		cl.def("localInverseCoordinatesOf", (class easy3d::Vec<3, float> (easy3d::Frame::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Frame::localInverseCoordinatesOf, "C++: easy3d::Frame::localInverseCoordinatesOf(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("src"));
		cl.def("coordinatesOfIn", (class easy3d::Vec<3, float> (easy3d::Frame::*)(const class easy3d::Vec<3, float> &, const class easy3d::Frame *const) const) &easy3d::Frame::coordinatesOfIn, "C++: easy3d::Frame::coordinatesOfIn(const class easy3d::Vec<3, float> &, const class easy3d::Frame *const) const --> class easy3d::Vec<3, float>", pybind11::arg("src"), pybind11::arg("in"));
		cl.def("coordinatesOfFrom", (class easy3d::Vec<3, float> (easy3d::Frame::*)(const class easy3d::Vec<3, float> &, const class easy3d::Frame *const) const) &easy3d::Frame::coordinatesOfFrom, "C++: easy3d::Frame::coordinatesOfFrom(const class easy3d::Vec<3, float> &, const class easy3d::Frame *const) const --> class easy3d::Vec<3, float>", pybind11::arg("src"), pybind11::arg("from"));
		cl.def("transformOf", (class easy3d::Vec<3, float> (easy3d::Frame::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Frame::transformOf, "C++: easy3d::Frame::transformOf(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("src"));
		cl.def("inverseTransformOf", (class easy3d::Vec<3, float> (easy3d::Frame::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Frame::inverseTransformOf, "C++: easy3d::Frame::inverseTransformOf(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("src"));
		cl.def("localTransformOf", (class easy3d::Vec<3, float> (easy3d::Frame::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Frame::localTransformOf, "C++: easy3d::Frame::localTransformOf(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("src"));
		cl.def("localInverseTransformOf", (class easy3d::Vec<3, float> (easy3d::Frame::*)(const class easy3d::Vec<3, float> &) const) &easy3d::Frame::localInverseTransformOf, "C++: easy3d::Frame::localInverseTransformOf(const class easy3d::Vec<3, float> &) const --> class easy3d::Vec<3, float>", pybind11::arg("src"));
		cl.def("transformOfIn", (class easy3d::Vec<3, float> (easy3d::Frame::*)(const class easy3d::Vec<3, float> &, const class easy3d::Frame *const) const) &easy3d::Frame::transformOfIn, "C++: easy3d::Frame::transformOfIn(const class easy3d::Vec<3, float> &, const class easy3d::Frame *const) const --> class easy3d::Vec<3, float>", pybind11::arg("src"), pybind11::arg("in"));
		cl.def("transformOfFrom", (class easy3d::Vec<3, float> (easy3d::Frame::*)(const class easy3d::Vec<3, float> &, const class easy3d::Frame *const) const) &easy3d::Frame::transformOfFrom, "C++: easy3d::Frame::transformOfFrom(const class easy3d::Vec<3, float> &, const class easy3d::Frame *const) const --> class easy3d::Vec<3, float>", pybind11::arg("src"), pybind11::arg("from"));
		cl.def("constraint", (class easy3d::Constraint * (easy3d::Frame::*)() const) &easy3d::Frame::constraint, "Returns the current constraint applied to the Frame.\n\n        A  value (default) means that no Constraint is used to filter Frame\n        translation and rotation. See the Constraint class documentation for details.\n\n        You may have to use a  to convert the result to a Constraint\n        derived class. \n\nC++: easy3d::Frame::constraint() const --> class easy3d::Constraint *", pybind11::return_value_policy::automatic);
		cl.def("setConstraint", (void (easy3d::Frame::*)(class easy3d::Constraint *const)) &easy3d::Frame::setConstraint, "Sets the constraint() attached to the Frame.\n\n        A  value means no constraint. The previous constraint() should be\n        deleted by the calling method if needed. \n\nC++: easy3d::Frame::setConstraint(class easy3d::Constraint *const) --> void", pybind11::arg("constraint"));
		cl.def("matrix", (class easy3d::Mat4<float> (easy3d::Frame::*)() const) &easy3d::Frame::matrix, "C++: easy3d::Frame::matrix() const --> class easy3d::Mat4<float>");
		cl.def("worldMatrix", (class easy3d::Mat4<float> (easy3d::Frame::*)() const) &easy3d::Frame::worldMatrix, "C++: easy3d::Frame::worldMatrix() const --> class easy3d::Mat4<float>");
		cl.def("setFromMatrix", (void (easy3d::Frame::*)(const class easy3d::Mat4<float> &)) &easy3d::Frame::setFromMatrix, "C++: easy3d::Frame::setFromMatrix(const class easy3d::Mat4<float> &) --> void", pybind11::arg("m"));
		cl.def("inverse", (class easy3d::Frame (easy3d::Frame::*)() const) &easy3d::Frame::inverse, "C++: easy3d::Frame::inverse() const --> class easy3d::Frame");
		cl.def("worldInverse", (class easy3d::Frame (easy3d::Frame::*)() const) &easy3d::Frame::worldInverse, "Returns the inverse() of the Frame world transformation.\n\n        The orientation() of the new Frame is the quat::inverse() of the\n        original orientation. Its position() is the negated and inverse rotated image\n        of the original position.\n\n        The result Frame has a  referenceFrame() and a  constraint().\n\n        Use inverse() for a local (i.e. with respect to referenceFrame())\n        transformation inverse. \n\nC++: easy3d::Frame::worldInverse() const --> class easy3d::Frame");
	}
}
