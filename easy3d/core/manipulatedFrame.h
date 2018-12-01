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

#ifndef QGLVIEWER_MANIPULATED_FRAME_H
#define QGLVIEWER_MANIPULATED_FRAME_H

#include "frame.h"


namespace easy3d {


	/*! \brief A ManipulatedFrame is a Frame that can be rotated and translated
	  using the mouse. \class ManipulatedFrame manipulatedFrame.h
	  QGLViewer/manipulatedFrame.h

	  It converts the mouse motion into a translation and an orientation updates. A
	  ManipulatedFrame is used to move an object in the scene. Combined with object
	  selection, its MouseGrabber properties and a dynamic update of the scene, the
	  ManipulatedFrame introduces a great reactivity in your applications.

	  A ManipulatedFrame is attached to a QGLViewer using
	  QGLViewer::setManipulatedFrame(): \code init() { setManipulatedFrame( new
	  ManipulatedFrame() ); }

	  draw()
	  {
			glPushMatrix();
			glMultMatrixd(manipulatedFrame()->matrix());
			// draw the manipulated object here
			glPopMatrix();
	  }
	  \endcode
	  See the <a href="../examples/manipulatedFrame.html">manipulatedFrame
	  example</a> for a complete application.

	  Mouse events are normally sent to the QGLViewer::camera(). You have to press
	  the QGLViewer::FRAME state key (default is \c Control) to move the
	  QGLViewer::manipulatedFrame() instead. See the <a href="../mouse.html">mouse
	  page</a> for a description of mouse button bindings.

	  <h3>Inherited functionalities</h3>

	  A ManipulatedFrame is an overloaded instance of a Frame. The powerful
	  coordinate system transformation functions (Frame::coordinatesOf(),
	  Frame::transformOf(), ...) can hence be applied to a ManipulatedFrame.

	  A ManipulatedFrame is also a MouseGrabber. If the mouse cursor gets within a
	  distance of 10 pixels from the projected position of the ManipulatedFrame, the
	  ManipulatedFrame becomes the new QGLViewer::mouseGrabber(). It can then be
	  manipulated directly, without any specific state key, object selection or GUI
	  intervention. This is very convenient to directly move some objects in the
	  scene (typically a light). See the <a
	  href="../examples/mouseGrabber.html">mouseGrabber example</a> as an
	  illustration. Note that QWidget::setMouseTracking() needs to be enabled in
	  order to use this feature (see the MouseGrabber documentation).

	  <h3>Advanced functionalities</h3>

	  A QGLViewer can handle at most one ManipulatedFrame at a time. If you want to
	  move several objects in the scene, you simply have to keep a list of the
	  different ManipulatedFrames, and to activate the right one (using
	  QGLViewer::setManipulatedFrame()) when needed. This can for instance be done
	  according to an object selection: see the <a href="../examples/luxo.html">luxo
	  example</a> for an illustration.

	  When the ManipulatedFrame is being manipulated using the mouse (mouse pressed
	  and not yet released), isManipulated() returns \c true. This might be used to
	  trigger a specific action or display (as is done with QGLViewer::fastDraw()).

	  The ManipulatedFrame also emits a manipulated() signal each time its state is
	  modified by the mouse. This signal is automatically connected to the
	  QGLViewer::update() slot when the ManipulatedFrame is attached to a viewer
	  using QGLViewer::setManipulatedFrame().

	  You can make the ManipulatedFrame spin() if you release the rotation mouse
	  button while moving the mouse fast enough (see spinningSensitivity()). See
	  also translationSensitivity() and rotationSensitivity() for sensitivity
	  tuning. \nosubgrouping */
	class ManipulatedFrame : public Frame
	{
	public:
		ManipulatedFrame();
		/*! Virtual destructor. Empty. */
		virtual ~ManipulatedFrame() {}

		ManipulatedFrame(const ManipulatedFrame &mf);
		ManipulatedFrame &operator=(const ManipulatedFrame &mf);


		/*! @name Manipulation sensitivity */
		//@{
	public:
		/*! Defines the rotationSensitivity(). */
		void setRotationSensitivity(float sensitivity) {
			rotationSensitivity_ = sensitivity;
		}
		/*! Defines the translationSensitivity(). */
		void setTranslationSensitivity(float sensitivity) {
			translationSensitivity_ = sensitivity;
		}
		/*! Defines the wheelSensitivity(). */
		void setWheelSensitivity(float sensitivity) {
			wheelSensitivity_ = sensitivity;
		}
		/*! Defines the zoomSensitivity(). */
		void setZoomSensitivity(float sensitivity) { zoomSensitivity_ = sensitivity; }

	public:
		/*! Returns the influence of a mouse displacement on the ManipulatedFrame
		rotation.

		Default value is 1.0. With an identical mouse displacement, a higher value
		will generate a larger rotation (and inversely for lower values). A 0.0 value
		will forbid ManipulatedFrame mouse rotation (see also constraint()).

		See also setRotationSensitivity(), translationSensitivity(),
		spinningSensitivity() and wheelSensitivity(). */
		float rotationSensitivity() const { return rotationSensitivity_; }
		/*! Returns the influence of a mouse displacement on the ManipulatedFrame
		translation.

		Default value is 1.0. You should not have to modify this value, since with 1.0
		the ManipulatedFrame precisely stays under the mouse cursor.

		With an identical mouse displacement, a higher value will generate a larger
		translation (and inversely for lower values). A 0.0 value will forbid
		ManipulatedFrame mouse translation (see also constraint()).

		\note When the ManipulatedFrame is used to move a \e Camera (see the
		ManipulatedCameraFrame class documentation), after zooming on a small region
		of your scene, the camera may translate too fast. For a camera, it is the
		Camera::pivotPoint() that exactly matches the mouse displacement. Hence,
		instead of changing the translationSensitivity(), solve the problem by
		(temporarily) setting the Camera::pivotPoint() to a point on the zoomed region
		(see the QGLViewer::RAP_FROM_PIXEL mouse binding in the <a
		href="../mouse.html">mouse page</a>).

		See also setTranslationSensitivity(), rotationSensitivity(),
		spinningSensitivity() and wheelSensitivity(). */
		float translationSensitivity() const { return translationSensitivity_; }

		/*! Returns the zoom sensitivity.

		Default value is 1.0. A higher value will make the zoom faster.
		Use a negative value to invert the zoom in and out directions.

		See also setZoomSensitivity(), translationSensitivity(), rotationSensitivity()
		wheelSensitivity() and spinningSensitivity(). */
		float zoomSensitivity() const { return zoomSensitivity_; }
		/*! Returns the mouse wheel sensitivity.

		Default value is 1.0. A higher value will make the wheel action more efficient
		(usually meaning a faster zoom). Use a negative value to invert the zoom in
		and out directions.

		See also setWheelSensitivity(), translationSensitivity(),
		rotationSensitivity() zoomSensitivity() and spinningSensitivity(). */
		float wheelSensitivity() const { return wheelSensitivity_; }
		//@}

	public:
		/*! Returns the incremental rotation that is applied by spin() to the
		 ManipulatedFrame orientation when it isSpinning().

		 Default value is a null rotation (identity quat). Use
		 setSpinningQuaternion() to change this value.

		 The spinningQuaternion() axis is defined in the ManipulatedFrame coordinate
		 system. You can use Frame::transformOfFrom() to convert this axis from an
		 other Frame coordinate system. */
		quat spinningQuaternion() const { return spinningQuaternion_; }

		/*! Defines the spinningQuaternion(). Its axis is defined in the
		ManipulatedFrame coordinate system. */
		void setSpinningQuaternion(const quat &q) {
			spinningQuaternion_ = q;
		}

	protected:
		virtual void spin();
		//@}

		/*! @name Mouse event handlers */
		//@{
	protected:
		virtual void mousePressEvent(int x, int y, int button, int modifiers, Camera *const camera);
		virtual void mouseMoveEvent(int x, int y, int dx, int dy, int button, int modifiers, Camera *const camera);
		virtual void mouseReleaseEvent(int x, int y, int button, int modifiers, Camera *const camera);
		virtual void wheelEvent(int x, int y, int dx, int dy, Camera *const camera);
		virtual void mouseDoubleClickEvent(int x, int y, int button, int modifiers, Camera *const camera);

		//@}

#ifndef DOXYGEN
	protected:
		quat deformedBallQuaternion(int x, int y, int pre_x, int pre_y, float cx, float cy, const Camera *const camera);

		Constraint *previousConstraint_; // When manipulation is without Contraint.

		int mouseOriginalDirection(int x, int y, int dx, int dy);

		/*! Returns a screen scaled delta from event's position to prevPos_, along the
				X or Y direction, whichever has the largest magnitude. */
		float deltaWithPrevPos(int x, int y, int dx, int dy, Camera *const camera) const;
		/*! Returns a normalized wheel delta, proportionnal to wheelSensitivity(). */
		float wheelDelta(int x, int y, int dx, int dy) const;

	private:
		void zoom(float delta, const Camera *const camera);

#endif // DOXYGEN

	private:
		// Sensitivity
		float rotationSensitivity_;
		float translationSensitivity_;
		float wheelSensitivity_;
		float zoomSensitivity_;

		// Mouse speed and spinning
		quat spinningQuaternion_;

		// Whether the SCREEN_TRANS direction (horizontal or vertical) is fixed or
		// not.
		bool dirIsFixed_;

	private:
		friend class BasicViewer;
	};

}


#endif // QGLVIEWER_MANIPULATED_FRAME_H
