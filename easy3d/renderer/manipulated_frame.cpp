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
 *		- libQGLViewer (version Version 2.7.1, Nov 17th, 2017)
 * The original code is available at
 * http://libqglviewer.com/
 *
 * libQGLViewer is a C++ library based on Qt that eases the
 * creation of OpenGL 3D viewers.
 *
 *----------------------------------------------------------*/

#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulated_camera_frame.h>


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

	float ManipulatedFrame::wheelDelta(int wheel_dy) const {
		static const float WHEEL_SENSITIVITY_COEFF = 0.1f;
		return static_cast<float>(wheel_dy) * wheelSensitivity() * WHEEL_SENSITIVITY_COEFF;
	}


	void ManipulatedFrame::action_start() {
	}


	void ManipulatedFrame::action_end() {
		if (previousConstraint_)
			setConstraint(previousConstraint_);
	}

	/*! Modifies the ManipulatedFrame according to the mouse motion.

	Actual behavior depends on mouse bindings. See the QGLViewer::MouseAction enum
	and the <a href="../mouse.html">QGLViewer mouse page</a> for details.

	The \p camera is used to fit the mouse motion with the display parameters (see
	Camera::screenWidth(), Camera::screenHeight(), Camera::fieldOfView()).

	Emits the manipulated() signal. */
	void ManipulatedFrame::action_rotate(int x, int y, int dx, int dy, Camera *const camera, ScreenAxis axis)
	{
        if (dx == 0 && dy == 0)
            return;

        //todo: not fully tested
        //todo: use the Constraint class (see action_rotate() in ManipulatedCameraFrame)
		vec3 trans = camera->projectedCoordinatesOf(position());
		DLOG_IF(has_nan(trans), ERROR)
						<< "projectedCoordinatesOf(position()): " << trans
						<< "\n\tposition(): " << position()
						<< "\n\tcamera position: " << camera->position()
						<< "\n\tcamera orientation: " << camera->orientation();
		if (has_nan(trans))
			return;

		const int w = camera->screenWidth();
		const int h = camera->screenHeight();

		quat rot;
		if (axis == NONE) {	// free rotation
            const int pre_x = x - dx;
            const int pre_y = y - dy;
			// The incremental rotation defined in the ManipulatedFrame coordinate system.
			rot = deformedBallQuaternion(x, y, pre_x, pre_y, trans[0], trans[1], w, h);
		}
		else {
#if 0 // old implementation (should work)
            const float prev_angle = std::atan2(pre_y - trans[1], pre_x - trans[0]);
            const float angle = std::atan2(y - trans[1], x - trans[0]);
            const vec3& axis = transformOf(camera->frame()->inverseTransformOf(vec3(0.0, 0.0, -1.0)));
            // The incremental rotation defined in the ManipulatedFrame coordinate system.
            quat rot(axis, angle - prev_angle);
            // Rotates the ManipulatedFrame around its origin.
            rotate(rot);
#else // new implementation
            if (axis == ORTHOGONAL) {
                const int pre_x = x - dx;
                const int pre_y = y - dy;
                const float prev_angle = std::atan2(static_cast<float>(pre_y) - trans[1], static_cast<float>(pre_x) - trans[0]);
                const float angle = std::atan2(static_cast<float>(y) - trans[1], static_cast<float>(x) - trans[0]);
                // The incremental rotation defined in the ManipulatedCameraFrame's coordinate system.
                rot = quat(vec3(0.0, 0.0, 1.0), angle - prev_angle);
            } else if (axis == VERTICAL) {
                const int pre_x = x - dx;
                const int pre_y = y;    // restricts the movement to be horizontal (so purl vertical rotation)
                rot = deformedBallQuaternion(x, y, pre_x, pre_y, trans.x, trans.y, w, h);
            } else if (axis == HORIZONTAL) {
                const int pre_x = x;    // restricts the movement to be vertical (so purl horizontal rotation)
                const int pre_y = y - dy;
                rot = deformedBallQuaternion(x, y, pre_x, pre_y, trans.x, trans.y, w, h);
            }
#endif
		}

        trans = vec3(-rot[0], -rot[1], -rot[2]);
        trans = camera->frame()->orientation().rotate(trans);
        trans = transformOf(trans);
        rot[0] = trans[0];
        rot[1] = trans[1];
        rot[2] = trans[2];
        // Rotates the ManipulatedFrame around its origin.
        rotate(rot);

        modified.send();
	}

	void ManipulatedFrame::action_translate(int x, int y, int dx, int dy, Camera *const camera, ScreenAxis axis)
	{
        if (dx == 0 && dy == 0)
            return;

        //todo: not fully tested
        //todo: use the Constraint class
		if (axis == NONE) {    // free translation
			vec3 trans(float(dx), -float(dy), 0.0);
			// Scale to fit the screen mouse displacement
			switch (camera->type()) {
				case Camera::PERSPECTIVE:
					trans *= 2.0f * std::tan(camera->fieldOfView() / 2.0f) *
							 std::fabs((camera->frame()->coordinatesOf(position())).z) /
                            static_cast<float>(camera->screenHeight());
					break;
				case Camera::ORTHOGRAPHIC: {
					float w, h;
					camera->getOrthoWidthHeight(w, h);
					trans[0] *= 2.0f * w / static_cast<float>(camera->screenWidth());
					trans[1] *= 2.0f * h / static_cast<float>(camera->screenHeight());
					break;
				}
			}
			// Transform to world coordinate system.
			trans = camera->frame()->orientation().rotate(translationSensitivity() * trans);
			// And then down to frame
			if (referenceFrame())
				trans = referenceFrame()->transformOf(trans);
			translate(trans);
		}
		else {  // new implementation
			vec3 trans;
			if (axis == HORIZONTAL)
				trans = vec3(float(dx), 0.0f, 0.0f);
			else if (axis == VERTICAL)
				trans = vec3(0.0f, float(-dy), 0.0f);
			else
				return;

			switch (camera->type()) {
			case Camera::PERSPECTIVE:
				trans *= 2.0 * std::tan(camera->fieldOfView() / 2.0f) *
					fabs((camera->frame()->coordinatesOf(position())).z) /
					camera->screenHeight();
				break;
			case Camera::ORTHOGRAPHIC: {
				float w, h;
				camera->getOrthoWidthHeight(w, h);
				trans[0] *= 2.0f * w / static_cast<float>(camera->screenWidth());
				trans[1] *= 2.0f * h / static_cast<float>(camera->screenHeight());
				break;
			    }
			}
			// Transform to world coordinate system.
			trans = camera->frame()->orientation().rotate(translationSensitivity() * trans);
			// And then down to frame
			if (referenceFrame())
				trans = referenceFrame()->transformOf(trans);

			translate(trans);
		}

        modified.send();
	}


	void ManipulatedFrame::action_zoom(int wheel_dy, Camera *const camera)
	{
		float delta = wheelDelta(wheel_dy);

		vec3 trans(0.0f, 0.0f, (camera->position() - position()).norm() * delta);

		trans = camera->frame()->orientation().rotate(trans);
		if (referenceFrame())
			trans = referenceFrame()->transformOf(trans);
		translate(trans);
        modified.send();

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
		const float size = 1.0f;
		const float size2 = size * size;
		const float size_limit = size2 * 0.5f;

		const float d = x * x + y * y;
		return d < size_limit ? std::sqrt(size2 - d) : size_limit / std::sqrt(d);
	}

#ifndef DOXYGEN
	/*! Returns a quaternion computed according to the mouse motion. Mouse positions
	are projected on a deformed ball, centered on (\p cx,\p cy). */
	quat ManipulatedFrame::deformedBallQuaternion(int x, int y, int pre_x, int pre_y, float cx, float cy, int w, int h) const {
		// Points on the deformed ball
		float px = rotationSensitivity() * (static_cast<float>(pre_x) - cx) / static_cast<float>(w);
		float py = rotationSensitivity() * (cy - static_cast<float>(pre_y)) / static_cast<float>(h);
		float dx = rotationSensitivity() * (static_cast<float>(x) - cx) / static_cast<float>(w);
		float dy = rotationSensitivity() * (cy - static_cast<float>(y)) / static_cast<float>(h);

		const vec3 p1(px, py, projectOnBall(px, py));
		const vec3 p2(dx, dy, projectOnBall(dx, dy));
		// Approximation of rotation angle
		// Should be divided by the projectOnBall size, but it is 1.0
		const vec3 axis = cross(p2, p1);
		const float angle = 5.0f * std::asin(std::sqrt(axis.length2() / p1.length2() / p2.length2()));
		return quat(axis, angle);
	}

}


#endif // DOXYGEN
