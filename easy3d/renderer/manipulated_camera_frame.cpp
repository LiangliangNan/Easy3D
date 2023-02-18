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

#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/camera.h>


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

	void ManipulatedCameraFrame::action_rotate(int x, int y, int dx, int dy, Camera *const camera, ScreenAxis axis)
	{
        if (dx == 0 && dy == 0)
            return;

        const vec3& trans = camera->projectedCoordinatesOf(pivotPoint());
        DLOG_IF(has_nan(trans), ERROR)
                        << "projectedCoordinatesOf(pivotPoint()): " << trans
                        << "\n\tpivotPoint():    " << pivotPoint()
                        << "\n\tcamera position: " << camera->position()
                        << "\n\tcamera orientation: " << camera->orientation();
        if (has_nan(trans))
            return;

        const int w = camera->screenWidth();
        const int h = camera->screenHeight();

        static CameraConstraint cons(camera);
        switch (axis) {
            case NONE:
                setConstraint(nullptr);
                break;
            case ORTHOGONAL:
                cons.setRotationConstraint(AxisPlaneConstraint::AXIS, vec3(0, 0, 1));
                setConstraint(&cons);
                break;
            case VERTICAL:
                cons.setRotationConstraint(AxisPlaneConstraint::AXIS, vec3(0, 1, 0));
                setConstraint(&cons);
                break;
            case HORIZONTAL:
                cons.setRotationConstraint(AxisPlaneConstraint::AXIS, vec3(1, 0, 0));
                setConstraint(&cons);
                break;
        }

        const int pre_x = x - dx;
        const int pre_y = y - dy;
        const quat &rot = deformedBallQuaternion(x, y, pre_x, pre_y, trans.x, trans.y, w, h);
        // Rotates the ManipulatedCameraFrame around its pivotPoint() instead of its origin.
        rotateAroundPoint(rot, pivotPoint());

        modified.send();
	}


	void ManipulatedCameraFrame::action_translate(int x, int y, int dx, int dy, Camera *const camera, ScreenAxis axis)
	{
        //todo: not fully tested
        //todo: use the Constraint class
        if (dx == 0 && dy == 0)
            return;

        if (axis == NONE) {    // free translation
            vec3 trans(-static_cast<float>(dx), static_cast<float>(dy), 0.0f);
            // Scale to fit the screen mouse displacement
            switch (camera->type())
            {
                case Camera::PERSPECTIVE:
                    trans *= 2.0f * std::tan(camera->fieldOfView() / 2.0f) *
                             fabs((camera->frame()->coordinatesOf(pivotPoint())).z) /
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
            translate(inverseTransformOf(translationSensitivity() * trans));
        }
        else {
			vec3 trans;
            if (axis == HORIZONTAL)
                trans = vec3(-static_cast<float>(dx), 0.0f, 0.0f);
            else if (axis == VERTICAL)
                trans = vec3(0.0f, static_cast<float>(dy), 0.0f);
            else
                return;

			switch (camera->type()) {
			case Camera::PERSPECTIVE:
				trans *= 2.0f * std::tan(camera->fieldOfView() / 2.0f) *
					fabs((camera->frame()->coordinatesOf(pivotPoint())).z) /
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

			translate(inverseTransformOf(translationSensitivity() * trans));
		}

        modified.send();
	}


	void ManipulatedCameraFrame::action_zoom(int wheel_dy, Camera *const camera)
	{
		const float delta = wheelDelta(wheel_dy);
		if (zoomsOnPivotPoint_) {
			const vec3& direction = camera->pivotPoint() - position();	

            // Liangliang:
            //  - We don't want to move the camera too close to (when zoom in) or
            //    too far away from (when zoom out) the pivot point. So the camera
            //    position is maintained to be always within an acceptable range.
            //  - To avoid deadlock, no action is taken if the camera will go
            //    beyond the range.

            // the new camera position
            const vec3& new_pos = position() + delta * direction;
            const vec3& offset = camera->pivotPoint() - new_pos;

            // the camera should not go behind the pivot point
            if (dot(offset, camera->viewDirection()) <= 0)
                return;

#if 0 //  we allow to zoom out (to avoid lock)
            const float sceneRadius = camera->sceneRadius();
            // don't get too close to and don't get too far away from the pivot point.
            if ( (offset.norm() <= 0.01f * sceneRadius) || (offset.norm() >= 100.0f * sceneRadius))
                return;
#endif
            // now it is safe
            translate(delta * direction);
        } else {
            // Liangliang:
            //  - We don't want to move the camera too close to (when zoom in) or
            //    too far away from (when zoom out) the scene center. So the camera
            //    position is maintained to be always within an acceptable range.
            //  - To avoid deadlock, no action is taken if the camera will go
            //    beyond the range.

            // in the camera space
            const vec3& target = camera->frame()->coordinatesOf(camera->sceneCenter());
            const float coef = target.z;//std::max<float>(fabs(target.z), 0.2f * sceneRadius);
            const vec3 trans_eye(0.0f, 0.0f, coef * delta);

            // translation in the world space
            const vec3 offset = inverseTransformOf(trans_eye);

            // compute the target point if we do move the camera
            ManipulatedCameraFrame frame = *camera->frame();
            frame.translate(offset);

#if 0 //  we allow to zoom out (to avoid lock)
            const float sceneRadius = camera->sceneRadius();
            const vec3& new_target = frame.coordinatesOf(camera->sceneCenter());
            // don't get too close to and don't get too far away from the pivot point.
            if ( new_target.z >= -0.01f * sceneRadius || new_target.z <= -100.0f * sceneRadius)
                return;
#endif

            // now it is safe
            translate(offset);
		}

        modified.send();

		// #CONNECTION# startAction should always be called before
		if (previousConstraint_)
			setConstraint(previousConstraint_);
	}


	void ManipulatedCameraFrame::action_turn(float angle_radian, Camera *const camera) {
		// The rotation around current camera Y, proportional to the horizontal mouse position
        const quat rot(vec3(0.0, 1.0, 0.0), angle_radian);
		rotate(rot);
        modified.send();
	}

}
