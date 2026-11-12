/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++ library
 *      for processing and rendering 3D data.
 *      Journal of Open Source Software, 6(64), 3255, 2021.
 * ------------------------------------------------------------------
 *
 * Easy3D is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License Version 3
 * as published by the Free Software Foundation.
 *
 * Easy3D is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 ********************************************************************/

/** ----------------------------------------------------------
 *
 * the code is adapted from libQGLViewer with modifications.
 *		- libQGLViewer (Version 2.7.1, Nov 17th, 2017)
 * The original code is available at
 * http://libqglviewer.com/
 *
 * libQGLViewer is a C++ library based on Qt that eases the
 * creation of OpenGL 3D viewers.
 *
 *----------------------------------------------------------*/

#include <easy3d/renderer/constraint.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/frame.h>
#include <easy3d/renderer/manipulated_camera_frame.h>


namespace easy3d {


	/*! Projects the float on the axis of direction \p direction that passes through the origin.
	\p direction does not need to be normalized (but must be non null). */
	static void project_on_axis(vec3& v, const vec3& direction) {
		Line3 line = Line3::from_point_and_direction(vec3(0, 0, 0), vec3(direction));
		v = line.projection(v);
	}

	/*! Projects the float on the plane whose normal is \p normal that passes through the origin.
	\p normal does not need to be normalized (but must be non null). */
	static void project_on_plane(vec3& v, const vec3& normal) {
		Plane3 plane(vec3(0, 0, 0), normal);
		v = plane.projection(v);
	}


	////////////////////////////////////////////////////////////////////////////////
	//                                  Constraint                                //
	////////////////////////////////////////////////////////////////////////////////

	/*! Default constructor.

	translation_constraint_type() and rotation_constraint_type() are set to
	AxisPlaneConstraint::FREE. translation_constraint_direction() and
	rotation_constraint_direction() are set to (0,0,0). */
	AxisPlaneConstraint::AxisPlaneConstraint()
		: translation_constraint_type_(FREE), rotation_constraint_type_(FREE) {
		// Do not use set since set_rotation_constraint_type needs a read.
	}

	/*! Simply calls set_translation_constraint_type() and
	 * set_translation_constraint_direction(). */
    void AxisPlaneConstraint::set_translation_constraint(Type type, const vec3 &direction) {
        set_translation_constraint_type(type);
        set_translation_constraint_direction(direction);
    }

	/*! Defines the translation_constraint_direction(). The coordinate system where \p
	 * direction is expressed depends on your class implementation. */
	void AxisPlaneConstraint::set_translation_constraint_direction(
		const vec3 &direction) {
		if ((translation_constraint_type() != AxisPlaneConstraint::FREE) &&
			(translation_constraint_type() != AxisPlaneConstraint::FORBIDDEN)) {
			const float norm = direction.norm();
			if (norm < 1E-8) {
				LOG(ERROR) << "AxisPlaneConstraint::setTranslationConstraintDir: null vector "
					"for translation constraint";
				translation_constraint_type_ = AxisPlaneConstraint::FREE;
			}
			else
				translation_constraint_dir_ = direction / norm;
		}
	}

	/*! Simply calls set_rotation_constraint_type() and
	 * set_rotation_constraint_direction(). */
	void AxisPlaneConstraint::set_rotation_constraint(Type type,
		const vec3 &direction) {
		set_rotation_constraint_type(type);
		set_rotation_constraint_direction(direction);
	}

	/*! Defines the rotation_constraint_direction(). The coordinate system where \p
	 * direction is expressed depends on your class implementation. */
	void AxisPlaneConstraint::set_rotation_constraint_direction(const vec3 &direction) {
		if ((rotation_constraint_type() != AxisPlaneConstraint::FREE) &&
			(rotation_constraint_type() != AxisPlaneConstraint::FORBIDDEN)) {
			const float norm = direction.norm();
			if (norm < 1E-8) {
				LOG(ERROR) << "AxisPlaneConstraint::setRotationConstraintDir: null vector for "
					"rotation constraint";
				rotation_constraint_type_ = AxisPlaneConstraint::FREE;
			}
			else
				rotation_constraint_dir_ = direction / norm;
		}
	}

	/*! Set the Type() of the rotation_constraint_type(). Default is
	 AxisPlaneConstraint::FREE.

	 Depending on this value, the Frame will freely rotate
	 (AxisPlaneConstraint::FREE), will only be able to rotate around an axis
	 (AxisPlaneConstraint::AXIS), or will not able to rotate at all
	 (AxisPlaneConstraint::FORBIDDEN).

	 Use Frame::set_orientation() to define the orientation of the constrained Frame
	 before it gets constrained.

	 \attention An AxisPlaneConstraint::PLANE Type() is not meaningful for
	 rotational constraints and will be ignored. */
	void AxisPlaneConstraint::set_rotation_constraint_type(Type type) {
		if (rotation_constraint_type() == AxisPlaneConstraint::PLANE) {
			LOG(ERROR) << "AxisPlaneConstraint::set_rotation_constraint_type: the PLANE type "
				"cannot be used for a rotation constraints";
			return;
		}

		rotation_constraint_type_ = type;
	}

	////////////////////////////////////////////////////////////////////////////////
	//                               LocalConstraint                              //
	////////////////////////////////////////////////////////////////////////////////

	/*! Depending on translation_constraint_type(), constrain \p translation to be
	  along an axis or limited to a plane defined in the Frame local coordinate
	  system by translation_constraint_direction(). */
	void LocalConstraint::constrain_translation(vec3 &translation,
		Frame *const frame) {
		vec3 proj;
		switch (translation_constraint_type()) {
		case AxisPlaneConstraint::FREE:
			break;
		case AxisPlaneConstraint::PLANE:
			proj = frame->rotation().rotate(translation_constraint_direction());
			project_on_plane(translation, proj);
			break;
		case AxisPlaneConstraint::AXIS:
			proj = frame->rotation().rotate(translation_constraint_direction());
			project_on_axis(translation, proj);
			break;
		case AxisPlaneConstraint::FORBIDDEN:
			translation = vec3(0.0, 0.0, 0.0);
			break;
		}
	}

	/*! When rotation_constraint_type() is AxisPlaneConstraint::AXIS, constrain \p
	  rotation to be a rotation around an axis whose direction is defined in the
	  Frame local coordinate system by rotation_constraint_direction(). */
	void LocalConstraint::constrain_rotation(quat &rotation, Frame *const) {
		switch (rotation_constraint_type()) {
		case AxisPlaneConstraint::FREE:
		case AxisPlaneConstraint::PLANE:
			break;
		case AxisPlaneConstraint::AXIS: {
			vec3 axis = rotation_constraint_direction();
			vec3 q = vec3(rotation[0], rotation[1], rotation[2]);
			project_on_axis(q, axis);
			rotation = quat(q, 2.0f * std::acos(rotation[3]));
		} break;
		case AxisPlaneConstraint::FORBIDDEN:
			rotation = quat(); // identity
			break;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	//                               WorldConstraint                              //
	////////////////////////////////////////////////////////////////////////////////

	/*! Depending on translation_constraint_type(), constrain \p translation to be
	  along an axis or limited to a plane defined in the world coordinate system by
	  translation_constraint_direction(). */
	void WorldConstraint::constrain_translation(vec3 &translation,
		Frame *const frame) {
		vec3 proj;
		switch (translation_constraint_type()) {
		case AxisPlaneConstraint::FREE:
			break;
		case AxisPlaneConstraint::PLANE:
			if (frame->reference_frame()) {
				proj = frame->reference_frame()->transform_of(translation_constraint_direction());
				project_on_plane(translation, proj);
			}
			else
				project_on_plane(translation, translation_constraint_direction());
			break;
		case AxisPlaneConstraint::AXIS:
			if (frame->reference_frame()) {
				proj = frame->reference_frame()->transform_of(translation_constraint_direction());
				project_on_axis(translation, proj);
			}
			else
				project_on_axis(translation, translation_constraint_direction());
			break;
		case AxisPlaneConstraint::FORBIDDEN:
			translation = vec3(0.0, 0.0, 0.0);
			break;
		}
	}

	/*! When rotation_constraint_type() is AxisPlaneConstraint::AXIS, constrain \p
	  rotation to be a rotation around an axis whose direction is defined in the
	  world coordinate system by rotation_constraint_direction(). */
	void WorldConstraint::constrain_rotation(quat &rotation,
		Frame *const frame) {
		switch (rotation_constraint_type()) {
		case AxisPlaneConstraint::FREE:
		case AxisPlaneConstraint::PLANE:
			break;
		case AxisPlaneConstraint::AXIS: {
			vec3 q(rotation[0], rotation[1], rotation[2]);
			vec3 axis = frame->transform_of(rotation_constraint_direction());
			project_on_axis(q, axis);
			rotation = quat(q, 2.0f * std::acos(rotation[3]));
			break;
		}
		case AxisPlaneConstraint::FORBIDDEN:
			rotation = quat(); // identity
			break;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	//                               CameraConstraint //
	////////////////////////////////////////////////////////////////////////////////

	/*! Creates a CameraConstraint, whose constrained directions are defined in the
	  \p camera coordinate system. */
	CameraConstraint::CameraConstraint(const Camera *const camera)
		: AxisPlaneConstraint(), camera_(camera) {}

	/*! Depending on translation_constraint_type(), constrain \p translation to be
	  along an axis or limited to a plane defined in the camera() coordinate system
	  by translation_constraint_direction(). */
	void CameraConstraint::constrain_translation(vec3 &translation,
		Frame *const frame) {
		vec3 proj;
		switch (translation_constraint_type()) {
		case AxisPlaneConstraint::FREE:
			break;
		case AxisPlaneConstraint::PLANE:
			proj = camera()->frame()->inverse_transform_of(translation_constraint_direction());
			if (frame->reference_frame())
				proj = frame->reference_frame()->transform_of(proj);
			project_on_plane(translation, proj);
			break;
		case AxisPlaneConstraint::AXIS:
			proj = camera()->frame()->inverse_transform_of(translation_constraint_direction());
			if (frame->reference_frame())
				proj = frame->reference_frame()->transform_of(proj);
			project_on_axis(translation, proj);
			break;
		case AxisPlaneConstraint::FORBIDDEN:
			translation = vec3(0.0, 0.0, 0.0);
			break;
		}
	}

	/*! When rotation_constraint_type() is AxisPlaneConstraint::AXIS, constrain \p
	  rotation to be a rotation around an axis whose direction is defined in the
	  camera() coordinate system by rotation_constraint_direction(). */
	void CameraConstraint::constrain_rotation(quat &rotation,
		Frame *const frame) {
		switch (rotation_constraint_type()) {
		case AxisPlaneConstraint::FREE:
		case AxisPlaneConstraint::PLANE:
			break;
		case AxisPlaneConstraint::AXIS: {
			vec3 axis = frame->transform_of(
				camera()->frame()->inverse_transform_of(rotation_constraint_direction()));
			vec3 q = vec3(rotation[0], rotation[1], rotation[2]);
			project_on_axis(q, axis);
			rotation = quat(q, 2.0f * std::acos(rotation[3]));
		} break;
		case AxisPlaneConstraint::FORBIDDEN:
			rotation = quat(); // identity
			break;
		}
	}


}
