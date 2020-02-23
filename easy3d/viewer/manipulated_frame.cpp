/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

/** ----------------------------------------------------------
 *
 * the code is adapted from libQGLViewer with modifications.
 *		- libQGLViewer (version Version 2.7.1, Nov 17th, 2017)
 * The original code is available at
 * http://libqglviewer.com/
 *
 * libQGLViewer is a C++ library based on Qt that eases the
 * creation of OpenGL 3D viewers.
 *
 *----------------------------------------------------------*/

#include <easy3d/viewer/manipulated_frame.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/manipulated_camera_frame.h>


namespace easy3d {

	/*! Default constructor.

	  The translation is set to (0,0,0), with an identity rotation (0,0,0,1) (see
	  Frame constructor for details).

	  The different sensitivities are set to their default values (see
	  rotationSensitivity(), translationSensitivity(), spinningSensitivity() and
	  wheelSensitivity()). */
	ManipulatedFrame::ManipulatedFrame()
	{
		// #CONNECTION# initFromDOMElement and accessor docs
		setRotationSensitivity(1.0);
		setTranslationSensitivity(1.0);
		setWheelSensitivity(1.0);
		setZoomSensitivity(1.0);

        previousConstraint_ = nullptr;
	}

	/*! Equal operator. Calls Frame::operator=() and then copy attributes. */
	ManipulatedFrame &ManipulatedFrame::operator=(const ManipulatedFrame &mf) {
		Frame::operator=(mf);

		setRotationSensitivity(mf.rotationSensitivity());
		setTranslationSensitivity(mf.translationSensitivity());
		setWheelSensitivity(mf.wheelSensitivity());
		setZoomSensitivity(mf.zoomSensitivity());

		dirIsFixed_ = false;

		return *this;
	}

	/*! Copy constructor. Performs a deep copy of all attributes using operator=().
	 */
	ManipulatedFrame::ManipulatedFrame(const ManipulatedFrame &mf)
		: Frame(mf)
	{
		(*this) = mf;
	}

	////////////////////////////////////////////////////////////////////////////////
	//                 M o u s e    h a n d l i n g                               //
	////////////////////////////////////////////////////////////////////////////////

	/*! Return 1 if mouse motion was started horizontally and -1 if it was more
	vertical. Returns 0 if this could not be determined yet (perfect diagonal
	motion, rare). */
	int ManipulatedFrame::mouseOriginalDirection(int x, int y, int dx, int dy) {
		static bool horiz = true; // Two simultaneous manipulatedFrame require two mice !

		if (!dirIsFixed_) {
			dirIsFixed_ = abs(dx) != abs(dy);
			horiz = abs(dx) > abs(dy);
		}

		if (dirIsFixed_)
			if (horiz)
				return 1;
			else
				return -1;
		else
			return 0;
	}

	float ManipulatedFrame::deltaWithPrevPos(int x, int y, int dx, int dy, Camera *const camera) const {
		float delta_x = float(dx) / camera->screenWidth();
		float delta_y = float(dy) / camera->screenHeight();

		float value = fabs(delta_x) > fabs(delta_y) ? delta_x : delta_y;
		return value * zoomSensitivity();
	}

	float ManipulatedFrame::wheelDelta(int wheel_dy) const {
		static const float WHEEL_SENSITIVITY_COEF = 0.1f;
		return wheel_dy * wheelSensitivity() * WHEEL_SENSITIVITY_COEF;
	}


	void ManipulatedFrame::action_start() {
		dirIsFixed_ = false;
	}


	void ManipulatedFrame::action_end()
	{
		dirIsFixed_ = false;
		if (previousConstraint_)
			setConstraint(previousConstraint_);
	}

	/*! Modifies the ManipulatedFrame according to the mouse motion.

	Actual behavior depends on mouse bindings. See the QGLViewer::MouseAction enum
	and the <a href="../mouse.html">QGLViewer mouse page</a> for details.

	The \p camera is used to fit the mouse motion with the display parameters (see
	Camera::screenWidth(), Camera::screenHeight(), Camera::fieldOfView()).

	Emits the manipulated() signal. */
	void ManipulatedFrame::action_rotate(int x, int y, int dx, int dy, Camera *const camera, bool screen /* = false */)
	{
		if (screen) {
            const vec3 &trans = camera->projectedCoordinatesOf(position());
            DLOG_IF(ERROR, has_nan(trans))
                            << "camera->projectedCoordinatesOf(position()): " << trans << ", position(): "
                            << position();

			float pre_x = float(x - dx);
			float pre_y = float(y - dy);
			const float prev_angle = atan2(pre_y - trans[1], pre_x - trans[0]);
			const float angle = atan2(y - trans[1], x - trans[0]);
            const vec3& axis = transformOf(camera->frame()->inverseTransformOf(vec3(0.0, 0.0, -1.0)));
			// The incremental rotation defined in the ManipulatedFrame coordinate system.
			quat rot(axis, angle - prev_angle);
			// Rotates the ManipulatedFrame around its origin.
			rotate(rot);
		}
		else {
            vec3 trans = camera->projectedCoordinatesOf(position());
            DLOG_IF(ERROR, has_nan(trans))
                            << "camera->projectedCoordinatesOf(position()): " << trans << ", position(): "
                            << position();

			int pre_x = x - dx;
			int pre_y = y - dy;
			// The incremental rotation defined in the ManipulatedFrame coordinate system.
            quat rot = deformedBallQuaternion(x, y, pre_x, pre_y, trans[0], trans[1], camera);
			trans = vec3(-rot[0], -rot[1], -rot[2]);
			trans = camera->frame()->orientation().rotate(trans);
			trans = transformOf(trans);
			rot[0] = trans[0];
			rot[1] = trans[1];
			rot[2] = trans[2];
			// Rotates the ManipulatedFrame around its origin.
			rotate(rot);
		}
	}

	void ManipulatedFrame::action_translate(int x, int y, int dx, int dy, Camera *const camera, bool screen /* = false */)
	{
		if (screen)	{
			vec3 trans;
			int dir = mouseOriginalDirection(x, y, dx, dy);
			if (dir == 1)
				trans = vec3(float(dx), 0.0f, 0.0f);
			else if (dir == -1)
				trans = vec3(0.0f, float(-dy), 0.0f);

			switch (camera->type()) {
			case Camera::PERSPECTIVE:
				trans *= 2.0 * tan(camera->fieldOfView() / 2.0f) *
					fabs((camera->frame()->coordinatesOf(position())).z) /
					camera->screenHeight();
				break;
			case Camera::ORTHOGRAPHIC: {
				float w, h;
				camera->getOrthoWidthHeight(w, h);
				trans[0] *= 2.0f * float(w) / camera->screenWidth();
				trans[1] *= 2.0f * float(h) / camera->screenHeight();
				break;
			}
			}
			// Transform to world coordinate system.
			trans =
				camera->frame()->orientation().rotate(translationSensitivity() * trans);
			// And then down to frame
			if (referenceFrame())
				trans = referenceFrame()->transformOf(trans);

			translate(trans);
		}
		else {
			vec3 trans(float(dx), -float(dy), 0.0);
			// Scale to fit the screen mouse displacement
			switch (camera->type()) {
			case Camera::PERSPECTIVE:
				trans *= 2.0f * tan(camera->fieldOfView() / 2.0f) *
					fabs((camera->frame()->coordinatesOf(position())).z) /
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
			// Transform to world coordinate system.
			trans =
				camera->frame()->orientation().rotate(translationSensitivity() * trans);
			// And then down to frame
			if (referenceFrame())
				trans = referenceFrame()->transformOf(trans);
			translate(trans);
		}

        trigger();
	}


	void ManipulatedFrame::action_zoom(int wheel_dy, Camera *const camera)
	{
		float delta = wheelDelta(wheel_dy);

		vec3 trans(0.0f, 0.0f, (camera->position() - position()).norm() * delta);

		trans = camera->frame()->orientation().rotate(trans);
		if (referenceFrame())
			trans = referenceFrame()->transformOf(trans);
		translate(trans);
        trigger();

		// #CONNECTION# startAction should always be called before
		if (previousConstraint_)
			setConstraint(previousConstraint_);
	}

	////////////////////////////////////////////////////////////////////////////////

	/*! Returns "pseudo-distance" from (x,y) to ball of radius size.
	\arg for a point inside the ball, it is proportional to the euclidean distance
	to the ball \arg for a point outside the ball, it is proportional to the inverse
	of this distance (tends to zero) on the ball, the function is continuous. */
	static float projectOnBall(float x, float y) {
		// If you change the size value, change angle computation in
		// deformedBallQuaternion().
		const float size = 1.0;
		const float size2 = size * size;
		const float size_limit = size2 * 0.5;

		const float d = x * x + y * y;
		return d < size_limit ? sqrt(size2 - d) : size_limit / sqrt(d);
	}

#ifndef DOXYGEN
	/*! Returns a quaternion computed according to the mouse motion. Mouse positions
	are projected on a deformed ball, centered on (\p cx,\p cy). */
	quat ManipulatedFrame::deformedBallQuaternion(int x, int y, int pre_x, int pre_y, float cx, float cy, const Camera *const camera) {
		// Points on the deformed ball
		float px = rotationSensitivity() * (pre_x - cx) / camera->screenWidth();
		float py = rotationSensitivity() * (cy - pre_y) / camera->screenHeight();
		float dx = rotationSensitivity() * (x - cx) / camera->screenWidth();
		float dy = rotationSensitivity() * (cy - y) / camera->screenHeight();

		const vec3 p1(px, py, projectOnBall(px, py));
		const vec3 p2(dx, dy, projectOnBall(dx, dy));
		// Approximation of rotation angle
		// Should be divided by the projectOnBall size, but it is 1.0
		const vec3 axis = cross(p2, p1);
		const float angle = 5.0f * asin(sqrt(axis.length2() / p1.length2() / p2.length2()));
		return quat(axis, angle);
	}

}


#endif // DOXYGEN
