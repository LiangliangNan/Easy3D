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

#ifndef EASY3D_RENDERER_MANIPULATED_FRAME_H
#define EASY3D_RENDERER_MANIPULATED_FRAME_H

#include <easy3d/renderer/frame.h>


namespace easy3d {


	/*! \brief A Frame that can be rotated and translated using the mouse.
	 * \class ManipulatedFrame easy3d/renderer/manipulated_frame.h
	 *
	 * \details A ManipulatedFrame converts the mouse motion into a translation and an orientation updates.
	 * A ManipulatedFrame is used to move an object in the scene. Combined with object selection, its MouseGrabber
	 * properties and a dynamic update of the scene, the ManipulatedFrame introduces a great reactivity in your
	 * applications.

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
	  using QGLViewer::setManipulatedFrame().*/
	class ManipulatedFrame : public Frame
	{
	public:
		ManipulatedFrame();
		/*! Virtual destructor. Empty. */
		~ManipulatedFrame() override = default;

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

		/*! @name Frame manipulation */
		//@{
	public:
		/// Initiates the ManipulatedFrame mouse manipulation.
		/// \note This function should be called when an action (e.g., drag) starts, e.g., mouse pressed.
		virtual void action_start(); 
		/// Stops the ManipulatedFrame mouse manipulation.
        /// \note This function should be called when an action (e.g., drag) ends, e.g., mouse released.
		virtual void action_end();

		enum ScreenAxis { NONE, HORIZONTAL, VERTICAL, ORTHOGONAL };
		virtual void action_rotate(int mouse_x, int mouse_y, int mouse_dx, int mouse_dy, Camera* camera, ScreenAxis axis);
		virtual void action_translate(int mouse_x, int mouse_y, int mouse_dx, int mouse_dy, Camera* camera, ScreenAxis axis);
		virtual void action_zoom(int dy_wheel, Camera* camera);
		// @}

#ifndef DOXYGEN
	protected:
        /*! Returns a quaternion computed according to the mouse motion. Mouse positions
        are projected on a deformed ball, centered on (\p cx,\p cy), viewer size (\p w, \p h).*/
		quat deformedBallQuaternion(int x, int y, int pre_x, int pre_y, float cx, float cy, int w, int h) const;

		Constraint *previousConstraint_; // When manipulation is without constraint.

		/*! Returns a normalized wheel delta, proportional to wheelSensitivity(). */
		float wheelDelta(int wheel_dy) const;

#endif // DOXYGEN

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


#endif // EASY3D_RENDERER_MANIPULATED_FRAME_H
