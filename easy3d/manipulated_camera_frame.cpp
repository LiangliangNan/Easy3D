/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D: software for processing and rendering
*   meshes and point clouds.
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
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


#include <easy3d/manipulated_camera_frame.h>

#include <algorithm>

#include <easy3d/camera.h>


namespace easy3d {


	ManipulatedCameraFrame::ManipulatedCameraFrame()
	{
	}


	ManipulatedCameraFrame &ManipulatedCameraFrame::operator=(const ManipulatedCameraFrame &mcf)
	{
		ManipulatedFrame::operator=(mcf);
		return *this;
	}

	ManipulatedCameraFrame::ManipulatedCameraFrame(const ManipulatedCameraFrame &mcf)
		: ManipulatedFrame(mcf)
	{
		(*this) = (mcf);
	}

    void ManipulatedCameraFrame::action_rotate(int x, int y, int dx, int dy, Camera *const camera)
	{
        vec3 trans = camera->projectedCoordinatesOf(camera->sceneCenter());
        int pre_x = x - dx;
        int pre_y = y - dy;
        quat rot = deformedBallQuaternion(x, y, pre_x, pre_y, trans[0], trans[1], camera);
        rotateAroundPoint(rot, camera->sceneCenter());
		frameModified();
	}


    void ManipulatedCameraFrame::action_translate(int x, int y, int dx, int dy, Camera *const camera)
	{
        vec3 trans(-float(dx), float(dy), 0.0f);
        // Scale to fit the screen mouse displacement
        switch (camera->type())
        {
        case Camera::PERSPECTIVE:
            trans *= 2.0f * tan(camera->fieldOfView() / 2.0f) *
                fabs((camera->frame()->coordinatesOf(camera->sceneCenter())).z) /
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
		frameModified();
	}


	void ManipulatedCameraFrame::action_zoom(int wheel_dy, Camera *const camera)
	{
		float delta = wheelDelta(wheel_dy);
		const float sceneRadius = camera->sceneRadius();
        const float coef = std::max<float>(fabs((camera->frame()->coordinatesOf(camera->sceneCenter())).z), 0.2f * sceneRadius);
        vec3 trans(0.0f, 0.0f, -coef * delta);
        translate(inverseTransformOf(trans));

		frameModified();
	}

}
