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

#include "manipulated_camera_frame.h"

#include <algorithm>

#include "camera.h"


namespace easy3d {


	ManipulatedCameraFrame::ManipulatedCameraFrame()
		: zoomsOnPivotPoint_(true)
	{
	}

	/*! Equal operator. Calls ManipulatedFrame::operator=() and then copy
	 * attributes. */
	ManipulatedCameraFrame &ManipulatedCameraFrame::operator=(const ManipulatedCameraFrame &mcf)
	{
		ManipulatedFrame::operator=(mcf);
		setZoomsOnPivotPoint(mcf.zoomsOnPivotPoint_);
		return *this;
	}

	/*! Copy constructor. Performs a deep copy of all members using operator=(). */
	ManipulatedCameraFrame::ManipulatedCameraFrame(const ManipulatedCameraFrame &mcf)
		: ManipulatedFrame(mcf)
	{
		(*this) = (mcf);
	}

	////////////////////////////////////////////////////////////////////////////////
	//                 M o u s e    h a n d l i n g                               //
	////////////////////////////////////////////////////////////////////////////////

	void ManipulatedCameraFrame::action_rotate(int x, int y, int dx, int dy, Camera *const camera, bool screen /* = false */)
	{
		if (screen) {
			vec3 trans = camera->projectedCoordinatesOf(pivotPoint());

			float pre_x = float(x - dx);
			float pre_y = float(y - dy);
			const float prev_angle = atan2(pre_y - trans[1], pre_x - trans[0]);
			const float angle = atan2(y - trans[1], x - trans[0]);

			// The incremental rotation defined in the ManipulatedCameraFrame coordinate system.
			quat rot(vec3(0.0, 0.0, 1.0), angle - prev_angle);
			// Rotates the ManipulatedCameraFrame around its pivotPoint() instead of its origin.
			rotateAroundPoint(rot, pivotPoint());
		}
		else {
			vec3 trans = camera->projectedCoordinatesOf(pivotPoint());
			int pre_x = x - dx;
			int pre_y = y - dy;
			quat rot = deformedBallQuaternion(x, y, pre_x, pre_y, trans[0], trans[1], camera);
			// Rotates the ManipulatedCameraFrame around its pivotPoint() instead of its origin.
			rotateAroundPoint(rot, pivotPoint());
		}
		frameModified();
	}


	void ManipulatedCameraFrame::action_translate(int x, int y, int dx, int dy, Camera *const camera, bool screen /* = false */)
	{
		if (screen) {
			vec3 trans;
			int dir = mouseOriginalDirection(x, y, dx, dy);
			if (dir == 1)
				trans = vec3(-float(dx), 0.0f, 0.0f);
			else if (dir == -1)
				trans = vec3(0.0f, float(dy), 0.0f);

			switch (camera->type()) {
			case Camera::PERSPECTIVE:
				trans *= 2.0f * tan(camera->fieldOfView() / 2.0f) *
					fabs((camera->frame()->coordinatesOf(pivotPoint())).z) /
					camera->screenHeight();
				break;
			case Camera::ORTHOGRAPHIC: {
				float w, h;
				camera->getOrthoWidthHeight(w, h);
				trans[0] *= 2.0f * w / camera->screenWidth();
				trans[1] *= 2.0f * h / camera->screenHeight();
				break;
			}
			}

			translate(inverseTransformOf(translationSensitivity() * trans));
		}

		else {
			vec3 trans(-float(dx), float(dy), 0.0f);
			// Scale to fit the screen mouse displacement
			switch (camera->type())
			{
			case Camera::PERSPECTIVE:
				trans *= 2.0f * tan(camera->fieldOfView() / 2.0f) *
					fabs((camera->frame()->coordinatesOf(pivotPoint())).z) /
					camera->screenHeight();
				break;
			case Camera::ORTHOGRAPHIC: {
				float w, h;
				camera->getOrthoWidthHeight(w, h);
				trans[0] *= 2.0f * w / camera->screenWidth();
				trans[1] *= 2.0f * h / camera->screenHeight();
				break;
			}
			}
			translate(inverseTransformOf(translationSensitivity() * trans));
		}
		frameModified();
	}


	/*! This is an overload of ManipulatedFrame::wheelEvent().

The wheel behavior depends on the wheel binded action. Current possible actions
are QGLViewer::ZOOM, QGLViewer::MOVE_FORWARD, QGLViewer::MOVE_BACKWARD.
QGLViewer::ZOOM speed depends on wheelSensitivity() while
QGLViewer::MOVE_FORWARD and QGLViewer::MOVE_BACKWARD depend on flySpeed(). See
QGLViewer::setWheelBinding() to customize the binding. */
	void ManipulatedCameraFrame::action_zoom(int wheel_dy, Camera *const camera)
	{
		float delta = wheelDelta(wheel_dy);
		const float sceneRadius = camera->sceneRadius();
		if (zoomsOnPivotPoint_) {
			const vec3& direction = camera->pivotPoint() - position();
			if (direction.norm() > 0.02f * sceneRadius || delta > 0.0f)
				translate(delta * direction);
		}
		else {
			const float coef = std::max(fabs((camera->frame()->coordinatesOf(camera->pivotPoint())).z), 0.2f * sceneRadius);
			vec3 trans(0.0f, 0.0f, -coef * delta);
			translate(inverseTransformOf(trans));
		}

		frameModified();

		// #CONNECTION# startAction should always be called before
		if (previousConstraint_)
			setConstraint(previousConstraint_);
	}


	void ManipulatedCameraFrame::action_turn(float angle_radian, Camera *const camera) {
		// The rotation around current camera Y, proportional to the horizontal mouse position
		quat rot(vec3(0.0, 1.0, 0.0), angle_radian);
		rotate(rot);
		frameModified();
	}

}
