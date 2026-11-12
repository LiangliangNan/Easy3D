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
 *		- libQGLViewer (Version 2.7.1, Nov 17th, 2017)
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

	/**
	 * \brief A Frame that can be rotated and translated using the mouse.
	 * \class ManipulatedFrame easy3d/renderer/manipulated_frame.h
	 *
	 * \details A ManipulatedFrame converts the mouse motion into translation and orientation updates.
	 *      A ManipulatedFrame is used to move an object in the scene. Combined with object selection, its MouseGrabber
	 *      properties and a dynamic update of the scene, the ManipulatedFrame introduces great reactivity in your
	 *      applications.
	 * \see Frame, ManipulatedCameraFrame, Constraint
	 */
	class ManipulatedFrame : public Frame
	{
	public:
		/**
		 * \brief Default constructor.
		 */
		ManipulatedFrame();
		/*! Virtual destructor. Empty. */
		~ManipulatedFrame() override = default;
		/**
		 * \brief Copy constructor.
		 * \param mf The ManipulatedFrame to copy from.
		 */
		ManipulatedFrame(const ManipulatedFrame &mf);
		/**
		 * \brief Copy assignment operator.
		 * \param mf The ManipulatedFrame to assign from.
		 * \return A reference to this ManipulatedFrame.
		 */
		ManipulatedFrame &operator=(const ManipulatedFrame &mf);


		/*! @name Manipulation sensitivity */
		//@{
	public:
		/**
		 * \brief Sets the rotation sensitivity.
		 * \param sensitivity The new rotation sensitivity.
		 */
		void set_rotation_sensitivity(float sensitivity) {
			rotation_sensitivity_ = sensitivity;
		}
		/**
		 * \brief Sets the translation sensitivity.
		 * \param sensitivity The new translation sensitivity.
		 */
		void set_translation_sensitivity(float sensitivity) {
			translation_sensitivity_ = sensitivity;
		}
		/**
		 * \brief Sets the wheel sensitivity.
		 * \param sensitivity The new wheel sensitivity.
		 */
		void set_wheel_sensitivity(float sensitivity) {
			wheel_sensitivity_ = sensitivity;
		}
		/**
		 * \brief Sets the zoom sensitivity.
		 * \param sensitivity The new zoom sensitivity.
		 */
		void set_zoom_sensitivity(float sensitivity) { zoom_sensitivity_ = sensitivity; }

	public:
		/**
		 * \brief Returns the rotation sensitivity.
		 * \details The rotation sensitivity is the influence of a mouse displacement on the ManipulatedFrame rotation.
		 *		Default value is 1.0. With an identical mouse displacement, a higher value will generate a larger
		 *		rotation (and inversely for lower values). A 0.0 value will forbid ManipulatedFrame mouse rotation
		 *		(see also constraint()).
		 * \return The rotation sensitivity.
		 * \sa set_rotation_sensitivity(), translation_sensitivity(), zoom_sensitivity(), and wheel_sensitivity().
		 */
		float rotation_sensitivity() const { return rotation_sensitivity_; }

		/**
		 * \brief Returns the translation sensitivity.
		 * \details The translation sensitivity is the influence of a mouse displacement on the ManipulatedFrame
		 *		translation. Default value is 1.0. You should not have to modify this value, since with 1.0 the
		 *		ManipulatedFrame precisely stays under the mouse cursor. With an identical mouse displacement, a
		 *		higher value will generate a larger	translation (and inversely for lower values). A 0.0 value will
		 *		forbid ManipulatedFrame mouse translation (see also constraint()).
		 * \return The translation sensitivity.
		 * \sa set_translation_sensitivity(), rotation_sensitivity(), zoom_sensitivity(), and wheel_sensitivity().
		 */
		float translation_sensitivity() const { return translation_sensitivity_; }
		/**
		 * \brief Returns the zoom sensitivity.
		 * \details Default value is 1.0. A higher value will make the zoom faster. Use a negative value to invert the
		 *		zoom in and out directions.
		 * \return The zoom sensitivity.
		 * \sa set_zoom_sensitivity(), translation_sensitivity(), rotation_sensitivity(), and wheel_sensitivity().
		 */
		float zoom_sensitivity() const { return zoom_sensitivity_; }
		/**
		 * \brief Returns the wheel sensitivity.
		 * \details Default value is 1.0. A higher value will make the wheel action more efficient (usually meaning a
		 *		faster zoom). Use a negative value to invert the zoom in and out directions.
		 * \return The wheel sensitivity.
		 * \sa set_wheel_sensitivity(), translation_sensitivity(), rotation_sensitivity(), and zoom_sensitivity().
		 */
		float wheel_sensitivity() const { return wheel_sensitivity_; }
		//@}

		/*! @name Frame manipulation */
		//@{
	public:
		/**
		 * \brief Initiates the ManipulatedFrame mouse manipulation.
		 * \note This function should be called when an action (e.g., drag) starts, e.g., mouse pressed.
		 */
		virtual void action_start(); 
		/**
		 * \brief Stops the ManipulatedFrame mouse manipulation.
		 * \note This function should be called when an action (e.g., drag) ends, e.g., mouse released.
		 */
		virtual void action_end();

		/// \brief ScreenAxis constrains rotation or translation around/along the axis.
		enum ScreenAxis {
			NONE,		///< No constraint.
			HORIZONTAL,	///< Horizontal constraint.
			VERTICAL,	///< Vertical constraint.
			ORTHOGONAL	///< Orthogonal constraint.
		};
		/**
		 * \brief Rotates the frame based on mouse movement.
		 * \param mouse_x The current x position of the mouse.
		 * \param mouse_y The current y position of the mouse.
		 * \param mouse_dx The change in x position of the mouse.
		 * \param mouse_dy The change in y position of the mouse.
		 * \param camera The camera associated with the frame.
		 * \param axis The axis of rotation.
		 */
		virtual void action_rotate(int mouse_x, int mouse_y, int mouse_dx, int mouse_dy, Camera* camera, ScreenAxis axis);
		/**
		 * \brief Translates the frame based on mouse movement.
		 * \param mouse_x The current x position of the mouse.
		 * \param mouse_y The current y position of the mouse.
		 * \param mouse_dx The change in x position of the mouse.
		 * \param mouse_dy The change in y position of the mouse.
		 * \param camera The camera associated with the frame.
		 * \param axis The axis of translation.
		 */
		virtual void action_translate(int mouse_x, int mouse_y, int mouse_dx, int mouse_dy, Camera* camera, ScreenAxis axis);
		/**
		 * \brief Zooms the frame based on mouse wheel movement.
		 * \param dy_wheel The change in the mouse wheel position.
		 * \param camera The camera associated with the frame.
		 */
		virtual void action_zoom(int dy_wheel, Camera* camera);
		// @}

	protected:
        /* Returns a quaternion computed according to the mouse motion. Mouse positions
        are projected on a deformed ball, centered on (\p cx,\p cy), viewer size (\p w, \p h).*/
		quat deformed_ball_quaternion(int x, int y, int pre_x, int pre_y, float cx, float cy, int w, int h) const;

		Constraint *previousConstraint_; // When manipulation is without constraint.

		/* Returns a normalized wheel delta, proportional to wheel_sensitivity(). */
		float wheel_delta(int wheel_dy) const;

	private:
		// Sensitivity
		float rotation_sensitivity_;
		float translation_sensitivity_;
		float wheel_sensitivity_;
		float zoom_sensitivity_;
	};

}


#endif // EASY3D_RENDERER_MANIPULATED_FRAME_H
