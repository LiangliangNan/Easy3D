#include <easy3d/core/quat.h>
#include <easy3d/core/vec.h>
#include <easy3d/fileio/translator.h>
#include <easy3d/renderer/frame.h>

#include <pybind11/pybind11.h>
#include <string>


#ifndef BINDER_PYBIND11_TYPE_CASTER
	#define BINDER_PYBIND11_TYPE_CASTER
	PYBIND11_DECLARE_HOLDER_TYPE(T, std::shared_ptr<T>, false)
	PYBIND11_DECLARE_HOLDER_TYPE(T, T*, false)
	PYBIND11_MAKE_OPAQUE(std::shared_ptr<void>)
#endif

// easy3d::Constraint file:easy3d/renderer/constraint.h line:139
struct PyCallBack_easy3d_Constraint : public easy3d::Constraint {
	using easy3d::Constraint::Constraint;

	void constrain_translation(class easy3d::Vec<3, float> & a0, class easy3d::Frame *const a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Constraint *>(this), "constrain_translation");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Constraint::constrain_translation(a0, a1);
	}
	void constrain_rotation(class easy3d::Quat<float> & a0, class easy3d::Frame *const a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::Constraint *>(this), "constrain_rotation");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return Constraint::constrain_rotation(a0, a1);
	}
};

// easy3d::AxisPlaneConstraint file:easy3d/renderer/constraint.h line:201
struct PyCallBack_easy3d_AxisPlaneConstraint : public easy3d::AxisPlaneConstraint {
	using easy3d::AxisPlaneConstraint::AxisPlaneConstraint;

	void constrain_translation(class easy3d::Vec<3, float> & a0, class easy3d::Frame *const a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::AxisPlaneConstraint *>(this), "constrain_translation");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return AxisPlaneConstraint::constrain_translation(a0, a1);
	}
	void constrain_rotation(class easy3d::Quat<float> & a0, class easy3d::Frame *const a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::AxisPlaneConstraint *>(this), "constrain_rotation");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return AxisPlaneConstraint::constrain_rotation(a0, a1);
	}
};

// easy3d::LocalConstraint file:easy3d/renderer/constraint.h line:331
struct PyCallBack_easy3d_LocalConstraint : public easy3d::LocalConstraint {
	using easy3d::LocalConstraint::LocalConstraint;

	void constrain_translation(class easy3d::Vec<3, float> & a0, class easy3d::Frame *const a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::LocalConstraint *>(this), "constrain_translation");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return LocalConstraint::constrain_translation(a0, a1);
	}
	void constrain_rotation(class easy3d::Quat<float> & a0, class easy3d::Frame *const a1) override {
		pybind11::gil_scoped_acquire gil;
		pybind11::function overload = pybind11::get_overload(static_cast<const easy3d::LocalConstraint *>(this), "constrain_rotation");
		if (overload) {
			auto o = overload.operator()<pybind11::return_value_policy::reference>(a0, a1);
			if (pybind11::detail::cast_is_temporary_value_reference<void>::value) {
				static pybind11::detail::override_caster_t<void> caster;
				return pybind11::detail::cast_ref<void>(std::move(o), caster);
			}
			return pybind11::detail::cast_safe<void>(std::move(o));
		}
		return LocalConstraint::constrain_rotation(a0, a1);
	}
};

void bind_easy3d_fileio_translator(pybind11::module_& m)
{
	{ // easy3d::Translator file:easy3d/fileio/translator.h line:39
		pybind11::class_<easy3d::Translator, std::shared_ptr<easy3d::Translator>> cl(m, "Translator", "Manages the translation of all the models during the file IO.\n \n\n\n    ");

		pybind11::enum_<easy3d::Translator::Status>(cl, "Status", pybind11::arithmetic(), "")
			.value("DISABLED", easy3d::Translator::DISABLED)
			.value("TRANSLATE_USE_FIRST_POINT", easy3d::Translator::TRANSLATE_USE_FIRST_POINT)
			.value("TRANSLATE_USE_LAST_KNOWN_OFFSET", easy3d::Translator::TRANSLATE_USE_LAST_KNOWN_OFFSET)
			.export_values();

		cl.def_static("instance", (class easy3d::Translator * (*)()) &easy3d::Translator::instance, "Returns the instance of the translator\n\nC++: easy3d::Translator::instance() --> class easy3d::Translator *", pybind11::return_value_policy::automatic);
		cl.def("set_status", (void (easy3d::Translator::*)(enum easy3d::Translator::Status)) &easy3d::Translator::set_status, "Sets the status of the translator.\n \n\n The new status to be set.\n\nC++: easy3d::Translator::set_status(enum easy3d::Translator::Status) --> void", pybind11::arg("s"));
		cl.def("status", (enum easy3d::Translator::Status (easy3d::Translator::*)() const) &easy3d::Translator::status, "Checks the status of the translator.\n \n\n The status of the translator.\n\nC++: easy3d::Translator::status() const --> enum easy3d::Translator::Status");
		cl.def("set_translation", (void (easy3d::Translator::*)(const class easy3d::Vec<3, double> &)) &easy3d::Translator::set_translation, "Sets the translation vector. If enabled, this translation will be applied to models loaded later on.\n \n\n The translation vector.\n\nC++: easy3d::Translator::set_translation(const class easy3d::Vec<3, double> &) --> void", pybind11::arg("t"));
		cl.def("translation", (const class easy3d::Vec<3, double> & (easy3d::Translator::*)() const) &easy3d::Translator::translation, "Gets the translation vector.\n \n\n The translation vector.\n\nC++: easy3d::Translator::translation() const --> const class easy3d::Vec<3, double> &", pybind11::return_value_policy::automatic);
	}
	{ // easy3d::Constraint file:easy3d/renderer/constraint.h line:139
		pybind11::class_<easy3d::Constraint, std::shared_ptr<easy3d::Constraint>, PyCallBack_easy3d_Constraint> cl(m, "Constraint", "An interface class for Frame constraints.\n	  \n\n\n	  \n This class defines the interface for the Constraints that can be applied to a\n	  Frame to limit its motion. Use Frame::set_constraint() to associate a\n	  Constraint to a Frame (default is a  Frame::constraint()).\n\n	  How does it work ?\n\n	  The Constraint acts as a filter on the translation and rotation Frame\n	  increments. constrain_translation() and constrain_rotation() should be\n	  overloaded to specify the constraint behavior: the desired displacement is\n	  given as a parameter that can optionally be modified.\n\n	  Here is how the Frame::translate() and Frame::rotate() methods use the\n	  Constraint: \n\n\n\n\n\n\n\n\n\n\n\n\n\n\n	  The default behavior of constrain_translation() and constrain_rotation() is\n	  empty (meaning no filtering).\n\n	  The Frame which uses the Constraint is passed as a parameter to the\n	  constrain_translation() and constrain_rotation() methods, so that they can have\n	  access to its current state (mainly Frame::position() and\n	  Frame::orientation()). It is not  for versatility reasons, but\n	  directly modifying it should be avoided.\n\n	  \n Frame::set_translation(), Frame::set_rotation() and similar methods\n	  will actually indeed set the frame position and orientation, without taking\n	  the constraint into account. Use the  versions of these\n	  methods to enforce the Constraint.\n\n	  Implemented Constraints\n\n	  Classical axial and plane Constraints are provided for convenience: see the\n	  LocalConstraint, WorldConstraint and CameraConstraint classes' documentations.\n\n	  Try the constrainedFrame and\n	  constrainedCamera examples\n	  for an illustration.\n\n	  Creating new Constraints\n\n	  The implementation of a new Constraint class simply consists in overloading\n	  the filtering methods: \n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n	  Note that the translation (resp. rotation) parameter passed to\n	  constrain_translation() (resp. constrain_rotation()) is expressed in the \n Frame coordinate system. Here, we use the Frame::transform_of() and\n	  Frame::inverse_transform_of() method to convert it to and from the world\n	  coordinate system.\n\n	  Combined constraints can easily be achieved by creating a new class that\n	  applies the different constraint filters: \n\n\n\n\n\n\n\n\n	  ");
		cl.def( pybind11::init( [](){ return new easy3d::Constraint(); }, [](){ return new PyCallBack_easy3d_Constraint(); } ) );
		cl.def( pybind11::init( [](PyCallBack_easy3d_Constraint const &o){ return new PyCallBack_easy3d_Constraint(o); } ) );
		cl.def( pybind11::init( [](easy3d::Constraint const &o){ return new easy3d::Constraint(o); } ) );
		cl.def("constrain_translation", (void (easy3d::Constraint::*)(class easy3d::Vec<3, float> &, class easy3d::Frame *const)) &easy3d::Constraint::constrain_translation, "Filters the translation applied to the  This default\n		implementation is empty (no filtering).\n\n		Overload this method in your own Constraint class to define a new translation\n		constraint.  is the Frame to which is applied the translation. It is\n		not defined  but you should refrain from directly changing its value\n		in the constraint. Use its Frame::position() and update the \n		accordingly instead.\n\n		 is expressed in local frame coordinate system. Use\n		Frame::inverse_transform_of() to express it in the world coordinate system if\n		needed. \n\nC++: easy3d::Constraint::constrain_translation(class easy3d::Vec<3, float> &, class easy3d::Frame *const) --> void", pybind11::arg("translation"), pybind11::arg("frame"));
		cl.def("constrain_rotation", (void (easy3d::Constraint::*)(class easy3d::Quat<float> &, class easy3d::Frame *const)) &easy3d::Constraint::constrain_rotation, "Filters the rotation applied to the  This default implementation\n		is empty (no filtering).\n\n		Overload this method in your own Constraint class to define a new rotation\n		constraint. See constrain_translation() for details.\n\n		Use Frame::inverse_transform_of() on the  quat::axis() to\n		express  in the world coordinate system if needed. \n\nC++: easy3d::Constraint::constrain_rotation(class easy3d::Quat<float> &, class easy3d::Frame *const) --> void", pybind11::arg("rotation"), pybind11::arg("frame"));
		cl.def("assign", (class easy3d::Constraint & (easy3d::Constraint::*)(const class easy3d::Constraint &)) &easy3d::Constraint::operator=, "C++: easy3d::Constraint::operator=(const class easy3d::Constraint &) --> class easy3d::Constraint &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
	{ // easy3d::AxisPlaneConstraint file:easy3d/renderer/constraint.h line:201
		pybind11::class_<easy3d::AxisPlaneConstraint, std::shared_ptr<easy3d::AxisPlaneConstraint>, PyCallBack_easy3d_AxisPlaneConstraint, easy3d::Constraint> cl(m, "AxisPlaneConstraint", "	   \n An abstract class for Frame Constraints defined by an axis or a plane.\n	   \n\n\n	   \n AxisPlaneConstraint is an interface for (translation and/or rotation)\n	   Constraint that are defined by a direction. translation_constraint_type() and\n	   rotation_constraint_type() define how this direction should be interpreted: as\n	   an axis (AxisPlaneConstraint::AXIS) or as a plane normal\n	   (AxisPlaneConstraint::PLANE). See the Type() documentation for details.\n\n	   The three implementations of this class: LocalConstraint, WorldConstraint and\n	   CameraConstraint differ by the coordinate system in which this direction is\n	   expressed.\n\n	   Different implementations of this class are illustrated in the\n	   contrainedCamera and\n	   constrainedFrame examples.\n\n	   \n When applied, the rotational Constraint may not intuitively follow\n	   the mouse displacement. A solution would be to directly measure the rotation\n	   angle in screen coordinates, but that would imply to know the\n	   QGLViewer::camera(), so that we can compute the projected coordinates of the\n	   rotation center (as is done with the QGLViewer::SCREEN_ROTATE binding).\n	   However, adding an extra pointer to the QGLViewer::camera() in all the\n	   AxisPlaneConstraint derived classes (which the user would have to update in a\n	   multi-viewer application) was judged as an overkill. ");
		cl.def( pybind11::init( [](){ return new easy3d::AxisPlaneConstraint(); }, [](){ return new PyCallBack_easy3d_AxisPlaneConstraint(); } ) );
		cl.def( pybind11::init( [](PyCallBack_easy3d_AxisPlaneConstraint const &o){ return new PyCallBack_easy3d_AxisPlaneConstraint(o); } ) );
		cl.def( pybind11::init( [](easy3d::AxisPlaneConstraint const &o){ return new easy3d::AxisPlaneConstraint(o); } ) );

		pybind11::enum_<easy3d::AxisPlaneConstraint::Type>(cl, "Type", pybind11::arithmetic(), "Type lists the different types of translation and rotation constraints\n		that are available.\n\n		It specifies the meaning of the constraint direction (see\n		translation_constraint_direction() and rotation_constraint_direction()): as an\n		axis direction (AxisPlaneConstraint::AXIS) or a plane normal\n		(AxisPlaneConstraint::PLANE). AxisPlaneConstraint::FREE means no constraint\n		while AxisPlaneConstraint::FORBIDDEN completely forbids the translation and/or\n		the rotation.\n\n		See translation_constraint_type() and rotation_constraint_type().\n\n		\n The AxisPlaneConstraint::PLANE Type is not valid for rotational\n		constraint.\n\n		New derived classes can use their own extended enum for specific constraints:\n		\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n ")
			.value("FREE", easy3d::AxisPlaneConstraint::FREE)
			.value("AXIS", easy3d::AxisPlaneConstraint::AXIS)
			.value("PLANE", easy3d::AxisPlaneConstraint::PLANE)
			.value("FORBIDDEN", easy3d::AxisPlaneConstraint::FORBIDDEN)
			.export_values();

		cl.def("constrain_translation", (void (easy3d::AxisPlaneConstraint::*)(class easy3d::Vec<3, float> &, class easy3d::Frame *const)) &easy3d::AxisPlaneConstraint::constrain_translation, "Overloading of Constraint::constrain_translation(). Empty \n\nC++: easy3d::AxisPlaneConstraint::constrain_translation(class easy3d::Vec<3, float> &, class easy3d::Frame *const) --> void", pybind11::arg("translation"), pybind11::arg("frame"));
		cl.def("set_translation_constraint", (void (easy3d::AxisPlaneConstraint::*)(enum easy3d::AxisPlaneConstraint::Type, const class easy3d::Vec<3, float> &)) &easy3d::AxisPlaneConstraint::set_translation_constraint, "C++: easy3d::AxisPlaneConstraint::set_translation_constraint(enum easy3d::AxisPlaneConstraint::Type, const class easy3d::Vec<3, float> &) --> void", pybind11::arg("type"), pybind11::arg("direction"));
		cl.def("set_translation_constraint_type", (void (easy3d::AxisPlaneConstraint::*)(enum easy3d::AxisPlaneConstraint::Type)) &easy3d::AxisPlaneConstraint::set_translation_constraint_type, "Sets the Type() of the translation_constraint_type(). Default is\n AxisPlaneConstraint::FREE. \n\nC++: easy3d::AxisPlaneConstraint::set_translation_constraint_type(enum easy3d::AxisPlaneConstraint::Type) --> void", pybind11::arg("type"));
		cl.def("set_translation_constraint_direction", (void (easy3d::AxisPlaneConstraint::*)(const class easy3d::Vec<3, float> &)) &easy3d::AxisPlaneConstraint::set_translation_constraint_direction, "C++: easy3d::AxisPlaneConstraint::set_translation_constraint_direction(const class easy3d::Vec<3, float> &) --> void", pybind11::arg("direction"));
		cl.def("translation_constraint_type", (enum easy3d::AxisPlaneConstraint::Type (easy3d::AxisPlaneConstraint::*)() const) &easy3d::AxisPlaneConstraint::translation_constraint_type, "Returns the translation constraint Type().\n\n		Depending on this value, the Frame will freely translate\n		(AxisPlaneConstraint::FREE), will only be able to translate along an axis\n		direction (AxisPlaneConstraint::AXIS), will be forced to stay into a plane\n		(AxisPlaneConstraint::PLANE) or will not able to translate at all\n		(AxisPlaneConstraint::FORBIDDEN).\n\n		Use Frame::set_position() to define the position of the constrained Frame\n		before it gets constrained. \n\nC++: easy3d::AxisPlaneConstraint::translation_constraint_type() const --> enum easy3d::AxisPlaneConstraint::Type");
		cl.def("translation_constraint_direction", (class easy3d::Vec<3, float> (easy3d::AxisPlaneConstraint::*)() const) &easy3d::AxisPlaneConstraint::translation_constraint_direction, "Returns the direction used by the translation constraint.\n\n		It represents the axis direction (AxisPlaneConstraint::AXIS) or the plane\n		normal (AxisPlaneConstraint::PLANE) depending on the\n		translation_constraint_type(). It is undefined for AxisPlaneConstraint::FREE or\n		AxisPlaneConstraint::FORBIDDEN.\n\n		The AxisPlaneConstraint derived classes express this direction in different\n		coordinate system (camera for CameraConstraint, local for LocalConstraint, and\n		world for WorldConstraint). This value can be modified with\n		set_translation_constraint_direction(). \n\nC++: easy3d::AxisPlaneConstraint::translation_constraint_direction() const --> class easy3d::Vec<3, float>");
		cl.def("constrain_rotation", (void (easy3d::AxisPlaneConstraint::*)(class easy3d::Quat<float> &, class easy3d::Frame *const)) &easy3d::AxisPlaneConstraint::constrain_rotation, "Overloading of Constraint::constrain_rotation(). Empty. \n\nC++: easy3d::AxisPlaneConstraint::constrain_rotation(class easy3d::Quat<float> &, class easy3d::Frame *const) --> void", pybind11::arg("rotation"), pybind11::arg("frame"));
		cl.def("set_rotation_constraint", (void (easy3d::AxisPlaneConstraint::*)(enum easy3d::AxisPlaneConstraint::Type, const class easy3d::Vec<3, float> &)) &easy3d::AxisPlaneConstraint::set_rotation_constraint, "C++: easy3d::AxisPlaneConstraint::set_rotation_constraint(enum easy3d::AxisPlaneConstraint::Type, const class easy3d::Vec<3, float> &) --> void", pybind11::arg("type"), pybind11::arg("direction"));
		cl.def("set_rotation_constraint_type", (void (easy3d::AxisPlaneConstraint::*)(enum easy3d::AxisPlaneConstraint::Type)) &easy3d::AxisPlaneConstraint::set_rotation_constraint_type, "C++: easy3d::AxisPlaneConstraint::set_rotation_constraint_type(enum easy3d::AxisPlaneConstraint::Type) --> void", pybind11::arg("type"));
		cl.def("set_rotation_constraint_direction", (void (easy3d::AxisPlaneConstraint::*)(const class easy3d::Vec<3, float> &)) &easy3d::AxisPlaneConstraint::set_rotation_constraint_direction, "C++: easy3d::AxisPlaneConstraint::set_rotation_constraint_direction(const class easy3d::Vec<3, float> &) --> void", pybind11::arg("direction"));
		cl.def("rotation_constraint_type", (enum easy3d::AxisPlaneConstraint::Type (easy3d::AxisPlaneConstraint::*)() const) &easy3d::AxisPlaneConstraint::rotation_constraint_type, "Returns the rotation constraint Type(). \n\nC++: easy3d::AxisPlaneConstraint::rotation_constraint_type() const --> enum easy3d::AxisPlaneConstraint::Type");
		cl.def("rotation_constraint_direction", (class easy3d::Vec<3, float> (easy3d::AxisPlaneConstraint::*)() const) &easy3d::AxisPlaneConstraint::rotation_constraint_direction, "Returns the axis direction used by the rotation constraint.\n\n		This direction is defined only when rotation_constraint_type() is\n		AxisPlaneConstraint::AXIS.\n\n		The AxisPlaneConstraint derived classes express this direction in different\n		coordinate system (camera for CameraConstraint, local for LocalConstraint, and\n		world for WorldConstraint). This value can be modified with\n		set_rotation_constraint_direction(). \n\nC++: easy3d::AxisPlaneConstraint::rotation_constraint_direction() const --> class easy3d::Vec<3, float>");
		cl.def("assign", (class easy3d::AxisPlaneConstraint & (easy3d::AxisPlaneConstraint::*)(const class easy3d::AxisPlaneConstraint &)) &easy3d::AxisPlaneConstraint::operator=, "C++: easy3d::AxisPlaneConstraint::operator=(const class easy3d::AxisPlaneConstraint &) --> class easy3d::AxisPlaneConstraint &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
	{ // easy3d::LocalConstraint file:easy3d/renderer/constraint.h line:331
		pybind11::class_<easy3d::LocalConstraint, std::shared_ptr<easy3d::LocalConstraint>, PyCallBack_easy3d_LocalConstraint, easy3d::AxisPlaneConstraint> cl(m, "LocalConstraint", "An AxisPlaneConstraint defined in the Frame local coordinate system.\n	  \n\n\n	  \n The translation_constraint_direction() and rotation_constraint_direction() are\n	  expressed in the Frame local coordinate system (see Frame::reference_frame()).\n\n	  See the constrainedFrame\n	  example for an illustration. ");
		cl.def( pybind11::init( [](){ return new easy3d::LocalConstraint(); }, [](){ return new PyCallBack_easy3d_LocalConstraint(); } ) );
		cl.def( pybind11::init( [](PyCallBack_easy3d_LocalConstraint const &o){ return new PyCallBack_easy3d_LocalConstraint(o); } ) );
		cl.def( pybind11::init( [](easy3d::LocalConstraint const &o){ return new easy3d::LocalConstraint(o); } ) );
		cl.def("constrain_translation", (void (easy3d::LocalConstraint::*)(class easy3d::Vec<3, float> &, class easy3d::Frame *const)) &easy3d::LocalConstraint::constrain_translation, "C++: easy3d::LocalConstraint::constrain_translation(class easy3d::Vec<3, float> &, class easy3d::Frame *const) --> void", pybind11::arg("translation"), pybind11::arg("frame"));
		cl.def("constrain_rotation", (void (easy3d::LocalConstraint::*)(class easy3d::Quat<float> &, class easy3d::Frame *const)) &easy3d::LocalConstraint::constrain_rotation, "C++: easy3d::LocalConstraint::constrain_rotation(class easy3d::Quat<float> &, class easy3d::Frame *const) --> void", pybind11::arg("rotation"), pybind11::arg("frame"));
		cl.def("assign", (class easy3d::LocalConstraint & (easy3d::LocalConstraint::*)(const class easy3d::LocalConstraint &)) &easy3d::LocalConstraint::operator=, "C++: easy3d::LocalConstraint::operator=(const class easy3d::LocalConstraint &) --> class easy3d::LocalConstraint &", pybind11::return_value_policy::automatic, pybind11::arg(""));
	}
}
