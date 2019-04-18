/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work, 
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan. 
*           Easy3D: a lightweight, easy-to-use, and efficient C++ 
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
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


#ifndef EASY3D_MANIPULATED_CAMERA_FRAME_H
#define EASY3D_MANIPULATED_CAMERA_FRAME_H

#include <easy3d/viewer/manipulated_frame.h>


namespace easy3d {


	/*! \brief The ManipulatedCameraFrame class represents a ManipulatedFrame with
	  Camera specific mouse bindings. \class ManipulatedCameraFrame
	  manipulatedCameraFrame.h QGLViewer/manipulatedCameraFrame.h

	  A ManipulatedCameraFrame is a specialization of a ManipulatedFrame, designed
	  to be set as the Camera::frame(). Mouse motions are basically interpreted in a
	  negated way: when the mouse goes to the right, the ManipulatedFrame
	  translation goes to the right, while the ManipulatedCameraFrame has to go to
	  the \e left, so that the \e scene seems to move to the right.

	  A ManipulatedCameraFrame rotates around its pivotPoint(), which corresponds to
	  the associated Camera::pivotPoint().

	  A ManipulatedCameraFrame can also "fly" in the scene. It basically moves
	  forward, and turns according to the mouse motion. See flySpeed(),
	  sceneUpVector() and the QGLViewer::MOVE_FORWARD and QGLViewer::MOVE_BACKWARD
	  QGLViewer::MouseAction.

	  See the <a href="../mouse.html">mouse page</a> for a description of the
	  possible actions that can be performed using the mouse and their bindings.
	  \nosubgrouping */
	class ManipulatedCameraFrame : public ManipulatedFrame
	{
	public:
		ManipulatedCameraFrame();
		/*! Virtual destructor. Empty. */
		virtual ~ManipulatedCameraFrame() {}

		ManipulatedCameraFrame(const ManipulatedCameraFrame &mcf);
		ManipulatedCameraFrame &operator=(const ManipulatedCameraFrame &mcf);

		/*! @name Pivot point */
		//@{
	public:
		/*! Returns the point the ManipulatedCameraFrame pivot point, around which the
		camera rotates.

		It is defined in the world coordinate system. Default value is (0,0,0).

		When the ManipulatedCameraFrame is associated to a Camera,
		Camera::pivotPoint() also returns this value. This point can interactively be
		changed using the mouse (see Camera::setPivotPointFromPixel() and
		QGLViewer::RAP_FROM_PIXEL and QGLViewer::RAP_IS_CENTER in the <a
		href="../mouse.html">mouse page</a>). */
		vec3 pivotPoint() const { return pivotPoint_; }
		/*! Sets the pivotPoint(), defined in the world coordinate system. */
		void setPivotPoint(const vec3 &point) { pivotPoint_ = point; }

		/*! @name Camera manipulation */
		//@{
 	public:
		/*! Returns whether or not the QGLViewer::ZOOM action zooms on the pivot
		  point.

		  When set to \c false (default), a zoom action will move the camera along its
		  Camera::viewDirection(), i.e. back and forth along a direction perpendicular
		  to the projection screen.

		  setZoomsOnPivotPoint() to \c true will move the camera along an axis defined
		  by the Camera::pivotPoint() and its current position instead. As a result,
		  the projected position of the pivot point on screen will stay the same
		  during a zoom. */
		bool zoomsOnPivotPoint() const { return zoomsOnPivotPoint_; }
		/*! Sets the value of zoomsOnPivotPoint().

		   Default value is false. */
		void setZoomsOnPivotPoint(bool enabled) { zoomsOnPivotPoint_ = enabled; }

		/*! @name frame manipulation */
		//@{
    public:
		virtual void action_rotate(int mouse_x, int mouse_y, int mouse_dx, int mouse_dy, Camera *const camera, bool screen = false);
		virtual void action_translate(int mouse_x, int mouse_y, int mouse_dx, int mouse_dy, Camera *const camera, bool screen = false);
		virtual void action_zoom(int dy_wheel, Camera *const camera);
		virtual void action_turn(float angle_radian, Camera *const camera);		// The rotation around camera Y

	private:

		bool zoomsOnPivotPoint_;

		vec3 pivotPoint_;

	private:
		friend class Camera;
		friend class Viewer;
	};


}

#endif // EASY3D_MANIPULATED_CAMERA_FRAME_H
