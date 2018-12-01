/****************************************************************************

 Copyright (C) 2002-2014 Gilles Debunne. All rights reserved.

 This file is part of the QGLViewer library version 2.7.1.

 http://www.libqglviewer.com - contact@libqglviewer.com

 This file may be used under the terms of the GNU General Public License
 versions 2.0 or 3.0 as published by the Free Software Foundation and
 appearing in the LICENSE file included in the packaging of this file.
 In addition, as a special exception, Gilles Debunne gives you certain
 additional rights, described in the file GPL_EXCEPTION in this package.

 libQGLViewer uses dual licensing. Commercial/proprietary software must
 purchase a libQGLViewer Commercial License.

 This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.

*****************************************************************************/

#include "constraint.h"
#include "camera.h"
#include "frame.h"
#include "manipulatedCameraFrame.h"


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

	translationConstraintType() and rotationConstraintType() are set to
	AxisPlaneConstraint::FREE. translationConstraintDirection() and
	rotationConstraintDirection() are set to (0,0,0). */
	AxisPlaneConstraint::AxisPlaneConstraint()
		: translationConstraintType_(FREE), rotationConstraintType_(FREE) {
		// Do not use set since setRotationConstraintType needs a read.
	}

	/*! Simply calls setTranslationConstraintType() and
	 * setTranslationConstraintDirection(). */
	void AxisPlaneConstraint::setTranslationConstraint(Type type,
		const vec3 &direction) {
		setTranslationConstraintType(type);
		setTranslationConstraintDirection(direction);
	}

	/*! Defines the translationConstraintDirection(). The coordinate system where \p
	 * direction is expressed depends on your class implementation. */
	void AxisPlaneConstraint::setTranslationConstraintDirection(
		const vec3 &direction) {
		if ((translationConstraintType() != AxisPlaneConstraint::FREE) &&
			(translationConstraintType() != AxisPlaneConstraint::FORBIDDEN)) {
			const float norm = direction.norm();
			if (norm < 1E-8) {
				std::cerr << "AxisPlaneConstraint::setTranslationConstraintDir: null vector "
					"for translation constraint" << std::endl;
				translationConstraintType_ = AxisPlaneConstraint::FREE;
			}
			else
				translationConstraintDir_ = direction / norm;
		}
	}

	/*! Simply calls setRotationConstraintType() and
	 * setRotationConstraintDirection(). */
	void AxisPlaneConstraint::setRotationConstraint(Type type,
		const vec3 &direction) {
		setRotationConstraintType(type);
		setRotationConstraintDirection(direction);
	}

	/*! Defines the rotationConstraintDirection(). The coordinate system where \p
	 * direction is expressed depends on your class implementation. */
	void AxisPlaneConstraint::setRotationConstraintDirection(const vec3 &direction) {
		if ((rotationConstraintType() != AxisPlaneConstraint::FREE) &&
			(rotationConstraintType() != AxisPlaneConstraint::FORBIDDEN)) {
			const float norm = direction.norm();
			if (norm < 1E-8) {
				std::cerr << "AxisPlaneConstraint::setRotationConstraintDir: null vector for "
					"rotation constraint" << std::endl;
				rotationConstraintType_ = AxisPlaneConstraint::FREE;
			}
			else
				rotationConstraintDir_ = direction / norm;
		}
	}

	/*! Set the Type() of the rotationConstraintType(). Default is
	 AxisPlaneConstraint::FREE.

	 Depending on this value, the Frame will freely rotate
	 (AxisPlaneConstraint::FREE), will only be able to rotate around an axis
	 (AxisPlaneConstraint::AXIS), or will not able to rotate at all
	 (AxisPlaneConstraint::FORBIDDEN).

	 Use Frame::setOrientation() to define the orientation of the constrained Frame
	 before it gets constrained.

	 \attention An AxisPlaneConstraint::PLANE Type() is not meaningful for
	 rotational constraints and will be ignored. */
	void AxisPlaneConstraint::setRotationConstraintType(Type type) {
		if (rotationConstraintType() == AxisPlaneConstraint::PLANE) {
			std::cerr << "AxisPlaneConstraint::setRotationConstraintType: the PLANE type "
				"cannot be used for a rotation constraints" << std::endl;
			return;
		}

		rotationConstraintType_ = type;
	}

	////////////////////////////////////////////////////////////////////////////////
	//                               LocalConstraint                              //
	////////////////////////////////////////////////////////////////////////////////

	/*! Depending on translationConstraintType(), constrain \p translation to be
	  along an axis or limited to a plane defined in the Frame local coordinate
	  system by translationConstraintDirection(). */
	void LocalConstraint::constrainTranslation(vec3 &translation,
		Frame *const frame) {
		vec3 proj;
		switch (translationConstraintType()) {
		case AxisPlaneConstraint::FREE:
			break;
		case AxisPlaneConstraint::PLANE:
			proj = frame->rotation().rotate(translationConstraintDirection());
			project_on_plane(translation, proj);
			break;
		case AxisPlaneConstraint::AXIS:
			proj = frame->rotation().rotate(translationConstraintDirection());
			project_on_axis(translation, proj);
			break;
		case AxisPlaneConstraint::FORBIDDEN:
			translation = vec3(0.0, 0.0, 0.0);
			break;
		}
	}

	/*! When rotationConstraintType() is AxisPlaneConstraint::AXIS, constrain \p
	  rotation to be a rotation around an axis whose direction is defined in the
	  Frame local coordinate system by rotationConstraintDirection(). */
	void LocalConstraint::constrainRotation(quat &rotation, Frame *const) {
		switch (rotationConstraintType()) {
		case AxisPlaneConstraint::FREE:
			break;
		case AxisPlaneConstraint::PLANE:
			break;
		case AxisPlaneConstraint::AXIS: {
			vec3 axis = rotationConstraintDirection();
			vec3 q = vec3(rotation[0], rotation[1], rotation[2]);
			project_on_axis(q, axis);
			rotation = quat(q, 2.0f * acos(rotation[3]));
		} break;
		case AxisPlaneConstraint::FORBIDDEN:
			rotation = quat(); // identity
			break;
		}
	}

	////////////////////////////////////////////////////////////////////////////////
	//                               WorldConstraint                              //
	////////////////////////////////////////////////////////////////////////////////

	/*! Depending on translationConstraintType(), constrain \p translation to be
	  along an axis or limited to a plane defined in the world coordinate system by
	  translationConstraintDirection(). */
	void WorldConstraint::constrainTranslation(vec3 &translation,
		Frame *const frame) {
		vec3 proj;
		switch (translationConstraintType()) {
		case AxisPlaneConstraint::FREE:
			break;
		case AxisPlaneConstraint::PLANE:
			if (frame->referenceFrame()) {
				proj = frame->referenceFrame()->transformOf(translationConstraintDirection());
				project_on_plane(translation, proj);
			}
			else
				project_on_plane(translation, translationConstraintDirection());
			break;
		case AxisPlaneConstraint::AXIS:
			if (frame->referenceFrame()) {
				proj = frame->referenceFrame()->transformOf(translationConstraintDirection());
				project_on_axis(translation, proj);
			}
			else
				project_on_axis(translation, translationConstraintDirection());
			break;
		case AxisPlaneConstraint::FORBIDDEN:
			translation = vec3(0.0, 0.0, 0.0);
			break;
		}
	}

	/*! When rotationConstraintType() is AxisPlaneConstraint::AXIS, constrain \p
	  rotation to be a rotation around an axis whose direction is defined in the
	  world coordinate system by rotationConstraintDirection(). */
	void WorldConstraint::constrainRotation(quat &rotation,
		Frame *const frame) {
		switch (rotationConstraintType()) {
		case AxisPlaneConstraint::FREE:
			break;
		case AxisPlaneConstraint::PLANE:
			break;
		case AxisPlaneConstraint::AXIS: {
			vec3 q(rotation[0], rotation[1], rotation[2]);
			vec3 axis = frame->transformOf(rotationConstraintDirection());
			project_on_axis(q, axis);
			rotation = quat(q, 2.0f*std::acos(rotation[3]));
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

	/*! Depending on translationConstraintType(), constrain \p translation to be
	  along an axis or limited to a plane defined in the camera() coordinate system
	  by translationConstraintDirection(). */
	void CameraConstraint::constrainTranslation(vec3 &translation,
		Frame *const frame) {
		vec3 proj;
		switch (translationConstraintType()) {
		case AxisPlaneConstraint::FREE:
			break;
		case AxisPlaneConstraint::PLANE:
			proj = camera()->frame()->inverseTransformOf(translationConstraintDirection());
			if (frame->referenceFrame())
				proj = frame->referenceFrame()->transformOf(proj);
			project_on_plane(translation, proj);
			break;
		case AxisPlaneConstraint::AXIS:
			proj = camera()->frame()->inverseTransformOf(translationConstraintDirection());
			if (frame->referenceFrame())
				proj = frame->referenceFrame()->transformOf(proj);
			project_on_axis(translation, proj);
			break;
		case AxisPlaneConstraint::FORBIDDEN:
			translation = vec3(0.0, 0.0, 0.0);
			break;
		}
	}

	/*! When rotationConstraintType() is AxisPlaneConstraint::AXIS, constrain \p
	  rotation to be a rotation around an axis whose direction is defined in the
	  camera() coordinate system by rotationConstraintDirection(). */
	void CameraConstraint::constrainRotation(quat &rotation,
		Frame *const frame) {
		switch (rotationConstraintType()) {
		case AxisPlaneConstraint::FREE:
			break;
		case AxisPlaneConstraint::PLANE:
			break;
		case AxisPlaneConstraint::AXIS: {
			vec3 axis = frame->transformOf(
				camera()->frame()->inverseTransformOf(rotationConstraintDirection()));
			vec3 q = vec3(rotation[0], rotation[1], rotation[2]);
			project_on_axis(q, axis);
			rotation = quat(q, 2.0f * acos(rotation[3]));
		} break;
		case AxisPlaneConstraint::FORBIDDEN:
			rotation = quat(); // identity
			break;
		}
	}


}