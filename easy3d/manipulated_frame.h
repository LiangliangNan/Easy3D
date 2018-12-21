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


#ifndef EASY3D_MANIPULATED_FRAME_H
#define EASY3D_MANIPULATED_FRAME_H

#include <easy3d/frame.h>


namespace easy3d {


    class Camera;

	class ManipulatedFrame : public Frame
	{
	public:
		ManipulatedFrame();
		/*! Virtual destructor. Empty. */
		virtual ~ManipulatedFrame() {}

		ManipulatedFrame(const ManipulatedFrame &mf);
		ManipulatedFrame &operator=(const ManipulatedFrame &mf);


		void setRotationSensitivity(float sensitivity) {
			rotationSensitivity_ = sensitivity;
		}

		void setTranslationSensitivity(float sensitivity) {
			translationSensitivity_ = sensitivity;
		}

		void setWheelSensitivity(float sensitivity) {
			wheelSensitivity_ = sensitivity;
		}

		void setZoomSensitivity(float sensitivity) { zoomSensitivity_ = sensitivity; }

		float rotationSensitivity() const { return rotationSensitivity_; }

		float translationSensitivity() const { return translationSensitivity_; }

		float zoomSensitivity() const { return zoomSensitivity_; }

		float wheelSensitivity() const { return wheelSensitivity_; }

	public:
		
        virtual void action_rotate(int mouse_x, int mouse_y, int mouse_dx, int mouse_dy, Camera *const camera);
        virtual void action_translate(int mouse_x, int mouse_y, int mouse_dx, int mouse_dy, Camera *const camera);
		virtual void action_zoom(int dy_wheel, Camera *const camera);

	protected:
		quat deformedBallQuaternion(int x, int y, int pre_x, int pre_y, float cx, float cy, const Camera *const camera);
		int mouseOriginalDirection(int x, int y, int dx, int dy);
		float deltaWithPrevPos(int x, int y, int dx, int dy, Camera *const camera) const;
		float wheelDelta(int wheel_dy) const;

	private:
		// Sensitivity
		float rotationSensitivity_;
		float translationSensitivity_;
		float wheelSensitivity_;
		float zoomSensitivity_;

	private:
		friend class Viewer;
	};

}


#endif // EASY3D_MANIPULATED_FRAME_H
