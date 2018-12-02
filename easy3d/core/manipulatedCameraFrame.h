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

#ifndef QGLVIEWER_MANIPULATED_CAMERA_FRAME_H
#define QGLVIEWER_MANIPULATED_CAMERA_FRAME_H

#include "manipulatedFrame.h"


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
	protected:
		virtual void action_rotate(int mouse_x, int mouse_y, int mouse_dx, int mouse_dy, Camera *const camera, bool screen = false);
		virtual void action_translate(int mouse_x, int mouse_y, int mouse_dx, int mouse_dy, Camera *const camera, bool screen = false);
		virtual void action_zoom(int dy_wheel, Camera *const camera);

	private:
		quat turnQuaternion(int dx, const Camera *const camera);
		quat pitchYawQuaternion(int dx, int dy, const Camera *const camera);

	private:
		// Inverse the direction of an horizontal mouse motion. Depends on the
		// projected screen orientation of the vertical axis when the mouse button is
		// pressed.
		bool constrainedRotationIsReversed_;

		bool zoomsOnPivotPoint_;

		vec3 pivotPoint_;

	private:
		friend class Camera;
		friend class Viewer;
	};


}

#endif // QGLVIEWER_MANIPULATED_CAMERA_FRAME_H
