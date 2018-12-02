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

#include "manipulatedCameraFrame.h"

#include <algorithm>

#include <GLFW/glfw3.h> // Include glfw3.h after our OpenGL definitions
#include "camera.h"


namespace easy3d {


	/*! Default constructor.

	 flySpeed() is set to 0.0 and sceneUpVector() is (0,1,0). The pivotPoint() is
	 set to (0,0,0).

	  \attention Created object is removeFromMouseGrabberPool(). */
	ManipulatedCameraFrame::ManipulatedCameraFrame()
		: sceneUpVector_(0.0, 1.0, 0.0)
		, rotatesAroundUpVector_(false)
		, zoomsOnPivotPoint_(false)
	{
	}

	/*! Equal operator. Calls ManipulatedFrame::operator=() and then copy
	 * attributes. */
	ManipulatedCameraFrame &ManipulatedCameraFrame::operator=(const ManipulatedCameraFrame &mcf)
	{
		ManipulatedFrame::operator=(mcf);

		setSceneUpVector(mcf.sceneUpVector());
		setRotatesAroundUpVector(mcf.rotatesAroundUpVector_);
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

	/*! Overloading of ManipulatedFrame::spin().

	Rotates the ManipulatedCameraFrame around its pivotPoint() instead of its
	origin. */
	void ManipulatedCameraFrame::spin() {
		rotateAroundPoint(spinningQuaternion(), pivotPoint());
	}


	/*! This method will be called by the Camera when its orientation is changed, so
	that the sceneUpVector (private) is changed accordingly. You should not need to
	call this method. */
	void ManipulatedCameraFrame::updateSceneUpVector() {
		sceneUpVector_ = inverseTransformOf(vec3(0.0, 1.0, 0.0));
	}

	////////////////////////////////////////////////////////////////////////////////
	//                 M o u s e    h a n d l i n g                               //
	////////////////////////////////////////////////////////////////////////////////

	void ManipulatedCameraFrame::zoom(float delta, const Camera *const camera) {
		const float sceneRadius = camera->sceneRadius();
		if (zoomsOnPivotPoint_) {
			vec3 direction = position() - camera->pivotPoint();
			if (direction.norm() > 0.02f * sceneRadius || delta > 0.0f)
				translate(delta * direction);
		}
		else {
			const float coef = std::max(fabs((camera->frame()->coordinatesOf(camera->pivotPoint())).z), 0.2f * sceneRadius);
			vec3 trans(0.0f, 0.0f, -coef * delta);
			translate(inverseTransformOf(trans));
		}
	}


	/*! Overloading of ManipulatedFrame::mouseMoveEvent().

	Motion depends on mouse binding (see <a href="../mouse.html">mouse page</a> for
	details). The resulting displacements are basically inverted from those of a
	ManipulatedFrame. */
	void ManipulatedCameraFrame::mouseMoveEvent(int x, int y, int dx, int dy, int button, int modifiers, Camera *const camera)
	{
		// #CONNECTION# QGLViewer::mouseMoveEvent does the update().
		if (modifiers == 0 && button == GLFW_MOUSE_BUTTON_LEFT)	// QGLViewer::ROTATE
		{
			quat rot;
			if (rotatesAroundUpVector_) {
				// Multiply by 2.0 to get on average about the same speed as with the
				// deformed ball
				float delta_x = 2.0f * rotationSensitivity() * (-dx) / camera->screenWidth();
				float delta_y = 2.0f * rotationSensitivity() * (-dy) / camera->screenHeight();
				if (constrainedRotationIsReversed_)
					delta_x = -delta_x;
				vec3 verticalAxis = transformOf(sceneUpVector_);
				rot = quat(verticalAxis, delta_x) * quat(vec3(1.0f, 0.0f, 0.0f), delta_y);
			}
			else {
				vec3 trans = camera->projectedCoordinatesOf(pivotPoint());
				int pre_x = x - dx;
				int pre_y = y - dy;
				rot = deformedBallQuaternion(x, y, pre_x, pre_y, trans[0], trans[1], camera);
			}
			setSpinningQuaternion(rot);
			spin();
		}
		else if (modifiers == 0 && button == GLFW_MOUSE_BUTTON_RIGHT)	// QGLViewer::TRANSLATE
		{
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

		else if (modifiers == GLFW_MOD_SHIFT && button == GLFW_MOUSE_BUTTON_LEFT) // SCREEN_ROTATE
		{
			vec3 trans = camera->projectedCoordinatesOf(pivotPoint());

			float pre_x = float(x - dx);
			float pre_y = float(y - dy);
			const float prev_angle = atan2(pre_y - trans[1], pre_x - trans[0]);
			const float angle = atan2(y - trans[1], x - trans[0]);

			quat rot(vec3(0.0, 0.0, 1.0), angle - prev_angle);
			setSpinningQuaternion(rot);
			spin();
			updateSceneUpVector();
		}

		else if (modifiers == GLFW_MOD_SHIFT && button == GLFW_MOUSE_BUTTON_RIGHT)  // SCREEN_TRANSLATE
		{
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
		else if (button == GLFW_MOUSE_BUTTON_MIDDLE && modifiers == 0) {
			int delta = 0; // just to make the zoom speed equivalent to the wheel effect
			if (dy < 0) delta = -1;
			else if (dy > 0) delta = 1;
			camera->frame()->wheelEvent(x, y, dx, delta, camera);
		}

		// 	case QGLViewer::MOVE_FORWARD: {
		// 		quat rot = pitchYawQuaternion(dx, dy, camera);
		// 		rotate(rot);
		// 		//#CONNECTION# wheelEvent MOVE_FORWARD case
		// 		// actual translation is made in flyUpdate().
		// 		// translate(inverseTransformOf(float(0.0, 0.0, -flySpeed())));
		// 		break;
		// 	}
		// 
		// 	case QGLViewer::MOVE_BACKWARD: {
		// 		quat rot = pitchYawQuaternion(dx, dy, camera);
		// 		rotate(rot);
		// 		// actual translation is made in flyUpdate().
		// 		// translate(inverseTransformOf(float(0.0, 0.0, flySpeed())));
		// 		break;
		// 	}
		// 
		// 	case QGLViewer::DRIVE: {
		// 		quat rot = turnQuaternion(dx, camera);
		// 		rotate(rot);
		// 		// actual translation is made in flyUpdate().
		// 		driveSpeed_ = 0.01 * (event->y() - pressPos_.y());
		// 		break;
		// 	}
		// 
		// 	case QGLViewer::LOOK_AROUND: {
		// 		quat rot = pitchYawQuaternion(dx, dy, camera);
		// 		rotate(rot);
		// 		break;
		// 	}



			//case QGLViewer::ROLL: {
			//	const float angle =
			//		M_PI * (event->x() - prevPos_.x()) / camera->screenWidth();
			//	quat rot(float(0.0, 0.0, 1.0), angle);
			//	rotate(rot);
			//	setSpinningQuaternion(rot);
			//	updateSceneUpVector();
			//	break;
			//}

			//case QGLViewer::ZOOM_ON_REGION:
			//case QGLViewer::NO_MOUSE_ACTION:
			//	break;
			//}

			//if (action_ != QGLViewer::NO_MOUSE_ACTION) {
			//	prevPos_ = event->pos();
			//	if (action_ != QGLViewer::ZOOM_ON_REGION)
			//		// ZOOM_ON_REGION should not emit manipulated().
			//		// prevPos_ is used to draw rectangle feedback.
			//		Q_EMIT manipulated();
			//}

		frameModified();
	}

	/*! This is an overload of ManipulatedFrame::mouseReleaseEvent(). The
	  QGLViewer::MouseAction is terminated. */
	void ManipulatedCameraFrame::mouseReleaseEvent(int x, int y, int button, int modifiers, Camera *const camera)
	{
		//if (action_ == QGLViewer::ZOOM_ON_REGION)
		//	camera->fitScreenRegion(QRect(pressPos_, event->pos()));

		ManipulatedFrame::mouseReleaseEvent(x, y, button, modifiers, camera);
	}



	/*! This is an overload of ManipulatedFrame::wheelEvent().

	The wheel behavior depends on the wheel binded action. Current possible actions
	are QGLViewer::ZOOM, QGLViewer::MOVE_FORWARD, QGLViewer::MOVE_BACKWARD.
	QGLViewer::ZOOM speed depends on wheelSensitivity() while
	QGLViewer::MOVE_FORWARD and QGLViewer::MOVE_BACKWARD depend on flySpeed(). See
	QGLViewer::setWheelBinding() to customize the binding. */
	void ManipulatedCameraFrame::wheelEvent(int x, int y, int dx, int dy, Camera *const camera)
	{
		zoom(wheelDelta(x, y, dx, dy), camera);
		frameModified();

		// #CONNECTION# startAction should always be called before
		if (previousConstraint_)
			setConstraint(previousConstraint_);
	}


	////////////////////////////////////////////////////////////////////////////////

	/*! Returns a quat that is a rotation around current camera Y,
	 * proportionnal to the horizontal mouse position. */
	quat ManipulatedCameraFrame::turnQuaternion(int dx, const Camera *const camera) {
		return quat(vec3(0.0, 1.0, 0.0), rotationSensitivity() * (-dx) / camera->screenWidth());
	}

	/*! Returns a quat that is the composition of two rotations, inferred from
	  the mouse pitch (X axis) and yaw (sceneUpVector() axis). */
	quat
		ManipulatedCameraFrame::pitchYawQuaternion(int dx, int dy, const Camera *const camera) {
		const quat rotX(vec3(1.0, 0.0, 0.0), rotationSensitivity() * (-dy) / camera->screenHeight());
		const quat rotY(transformOf(sceneUpVector()), rotationSensitivity() * (-dx) / camera->screenWidth());
		return rotY * rotX;
	}


}