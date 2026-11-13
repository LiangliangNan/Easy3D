/** \cond LICENSE
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
 \endcond */

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

#ifndef EASY3D_RENDERER_CONSTRAINT_H
#define EASY3D_RENDERER_CONSTRAINT_H

#include <easy3d/core/types.h>


namespace easy3d {


	class Frame;
	class Camera;

	/*! \brief An interface class for Frame constraints.
	  \class Constraint easy3d/renderer/constraint.h

	  \details This class defines the interface for the Constraints that can be applied to a
	  Frame to limit its motion. Use Frame::set_constraint() to associate a
	  Constraint to a Frame (default is a \c NULL Frame::constraint()).

	  <h3>How does it work ?</h3>

	  The Constraint acts as a filter on the translation and rotation Frame
	  increments. constrain_translation() and constrain_rotation() should be
	  overloaded to specify the constraint behavior: the desired displacement is
	  given as a parameter that can optionally be modified.

	  Here is how the Frame::translate() and Frame::rotate() methods use the
	  Constraint: \code Frame::translate(vec3& T)
	  {
			if (constraint())
			  constraint()->constrain_translation(T, this);
			t += T;
	  }

	  Frame::rotate(quat& Q)
	  {
			if (constraint())
			  constraint()->constrain_rotation(Q, this);
			q *= Q;
	  }
	  \endcode

	  The default behavior of constrain_translation() and constrain_rotation() is
	  empty (meaning no filtering).

	  The Frame which uses the Constraint is passed as a parameter to the
	  constrain_translation() and constrain_rotation() methods, so that they can have
	  access to its current state (mainly Frame::position() and
	  Frame::orientation()). It is not \c const for versatility reasons, but
	  directly modifying it should be avoided.

	  \attention Frame::set_translation(), Frame::set_rotation() and similar methods
	  will actually indeed set the frame position and orientation, without taking
	  the constraint into account. Use the \e WithConstraint versions of these
	  methods to enforce the Constraint.

	  <h3>Implemented Constraints</h3>

	  Classical axial and plane Constraints are provided for convenience: see the
	  LocalConstraint, WorldConstraint and CameraConstraint classes' documentations.

	  Try the <a href="../examples/constrainedFrame.html">constrainedFrame</a> and
	  <a href="../examples/constrainedCamera.html">constrainedCamera</a> examples
	  for an illustration.

	  <h3>Creating new Constraints</h3>

	  The implementation of a new Constraint class simply consists in overloading
	  the filtering methods: \code
	  // This Constraint enforces that the Frame cannot have a negative z world
	  coordinate. class myConstraint : public Constraint
	  {
	  public:
			virtual void constrain_translation(vec3& t, Frame * const fr)
			  {
					// Express t in the world coordinate system.
					const vec3 tWorld = fr->inverse_transform_of(t);
			if (fr->position().z + tWorld.z < 0.0) // check the new fr z coordinate
			  t.z = fr->transform_of(-fr->position().z); // t.z is clamped so that
	  next z position is 0.0
			  }
	  };
	  \endcode

	  Note that the translation (resp. rotation) parameter passed to
	  constrain_translation() (resp. constrain_rotation()) is expressed in the \e
	  local Frame coordinate system. Here, we use the Frame::transform_of() and
	  Frame::inverse_transform_of() method to convert it to and from the world
	  coordinate system.

	  Combined constraints can easily be achieved by creating a new class that
	  applies the different constraint filters: \code
	  myConstraint::constrain_translation(vec3& v, Frame* const fr)
	  {
			constraint1->constrain_translation(v, fr);
			constraint2->constrain_translation(v, fr);
			// and so on, with possible branches, tests, loops...
	  }
	  \endcode
	  \see AxisPlaneConstraint, LocalConstraint, WorldConstraint, CameraConstraint, Frame
	  */
	class Constraint
	{
	public:
		/*! Virtual destructor. Empty. */
		virtual ~Constraint() = default;

		/*! Filters the translation applied to the \p frame. This default
		implementation is empty (no filtering).

		Overload this method in your own Constraint class to define a new translation
		constraint. \p frame is the Frame to which is applied the translation. It is
		not defined \c const, but you should refrain from directly changing its value
		in the constraint. Use its Frame::position() and update the \p translation
		accordingly instead.

		\p translation is expressed in local frame coordinate system. Use
		Frame::inverse_transform_of() to express it in the world coordinate system if
		needed.
		\param translation Desired translation in the local frame coordinates.
		\param frame Frame that receives the translation.
		*/
		virtual void constrain_translation(vec3 &translation, Frame *const frame) {
			(void)translation;
			(void)frame;
		}
		/*! Filters the rotation applied to the \p frame. This default implementation
		is empty (no filtering).

		Overload this method in your own Constraint class to define a new rotation
		constraint. See constrain_translation() for details.

		Use Frame::inverse_transform_of() on the \p rotation quat::axis() to
		express \p rotation in the world coordinate system if needed.
		\param rotation Desired rotation increment.
		\param frame Frame that receives the rotation.
		*/
		virtual void constrain_rotation(quat &rotation, Frame *const frame) {
			(void)(rotation);
			(void)(frame);
		}
	};

	/*!
	   \brief An abstract class for Frame Constraints defined by an axis or a plane.
	   \class AxisPlaneConstraint easy3d/renderer/constraint.h

	   \details AxisPlaneConstraint is an interface for (translation and/or rotation)
	   Constraint that are defined by a direction. translation_constraint_type() and
	   rotation_constraint_type() define how this direction should be interpreted: as
	   an axis (AxisPlaneConstraint::AXIS) or as a plane normal
	   (AxisPlaneConstraint::PLANE). See the Type() documentation for details.

	   The three implementations of this class: LocalConstraint, WorldConstraint and
	   CameraConstraint differ by the coordinate system in which this direction is
	   expressed.

	   Different implementations of this class are illustrated in the
	   <a href="../examples/constrainedCamera.html">contrainedCamera</a> and
	   <a href="../examples/constrainedFrame.html">constrainedFrame</a> examples.
	   \see Constraint, LocalConstraint, WorldConstraint, CameraConstraint

	   \attention When applied, the rotational Constraint may not intuitively follow
	   the mouse displacement. A solution would be to directly measure the rotation
	   angle in screen coordinates, but that would imply to know the
	   QGLViewer::camera(), so that we can compute the projected coordinates of the
	   rotation center (as is done with the QGLViewer::SCREEN_ROTATE binding).
	   However, adding an extra pointer to the QGLViewer::camera() in all the
	   AxisPlaneConstraint derived classes (which the user would have to update in a
	   multi-viewer application) was judged as an overkill. */
	class AxisPlaneConstraint : public Constraint {
	public:
		AxisPlaneConstraint();
		/*! Virtual destructor. Empty. */
        ~AxisPlaneConstraint() override = default;

		/*! Type lists the different types of translation and rotation constraints
		that are available.

		It specifies the meaning of the constraint direction (see
		translation_constraint_direction() and rotation_constraint_direction()): as an
		axis direction (AxisPlaneConstraint::AXIS) or a plane normal
		(AxisPlaneConstraint::PLANE). AxisPlaneConstraint::FREE means no constraint
		while AxisPlaneConstraint::FORBIDDEN completely forbids the translation and/or
		the rotation.

		See translation_constraint_type() and rotation_constraint_type().

		\attention The AxisPlaneConstraint::PLANE Type is not valid for rotational
		constraint.

		New derived classes can use their own extended enum for specific constraints:
		\code
		class MyAxisPlaneConstraint : public AxisPlaneConstraint
		{
		public:
		  enum MyType { FREE, AXIS, PLANE, FORBIDDEN, CUSTOM };
		  virtual void constrain_translation(vec3 &translation, Frame *const frame)
		  {
				// translation_constraint_type() is simply an int. CUSTOM Type is
		handled seamlessly. switch (translation_constraint_type())
				{
				  case MyAxisPlaneConstraint::FREE: ... break;
				  case MyAxisPlaneConstraint::CUSTOM: ... break;
				}
		  };

		  MyAxisPlaneConstraint* c = new MyAxisPlaneConstraint();
		  // Note the Type conversion
		  c->set_translation_constraint_type(AxisPlaneConstraint::Type(MyAxisPlaneConstraint::CUSTOM));
		};
		\endcode */
		enum Type { FREE, AXIS, PLANE, FORBIDDEN };

		/*! \name Translation constraint */
		//@{
		/*! Overloading of Constraint::constrain_translation(). Empty.
		 * \param translation Proposed translation, unchanged by this implementation.
		 * \param frame Frame receiving the translation.
		 */
        void constrain_translation(vec3 &translation, Frame *const frame) override {
			(void)(translation);
			(void)(frame);
		}

		/**
		 * \brief Set translation constraint type and direction.
		 * \param type Translation constraint mode.
		 * \param direction Axis direction or plane normal, depending on type.
		 */
		void set_translation_constraint(Type type, const vec3 &direction);
		/*! Sets the Type() of the translation_constraint_type(). Default is
		 * AxisPlaneConstraint::FREE.
		 * \param type Translation constraint mode.
		 */
		void set_translation_constraint_type(Type type) {
			translation_constraint_type_ = type;
		};
		/// \brief Set the translation constraint direction.
		/// \param direction Axis direction or plane normal.
		void set_translation_constraint_direction(const vec3 &direction);

		/*! Returns the translation constraint Type().

		Depending on this value, the Frame will freely translate
		(AxisPlaneConstraint::FREE), will only be able to translate along an axis
		direction (AxisPlaneConstraint::AXIS), will be forced to stay into a plane
		(AxisPlaneConstraint::PLANE) or will not able to translate at all
		(AxisPlaneConstraint::FORBIDDEN).

		Use Frame::set_position() to define the position of the constrained Frame
		before it gets constrained.
		\return Translation constraint type.
		*/
		Type translation_constraint_type() const { return translation_constraint_type_; };
		/*! Returns the direction used by the translation constraint.

		It represents the axis direction (AxisPlaneConstraint::AXIS) or the plane
		normal (AxisPlaneConstraint::PLANE) depending on the
		translation_constraint_type(). It is undefined for AxisPlaneConstraint::FREE or
		AxisPlaneConstraint::FORBIDDEN.

		The AxisPlaneConstraint derived classes express this direction in different
		coordinate system (camera for CameraConstraint, local for LocalConstraint, and
		world for WorldConstraint). This value can be modified with
		set_translation_constraint_direction().
		\return Translation axis direction or plane normal.
		*/
		vec3 translation_constraint_direction() const {
			return translation_constraint_dir_;
		};
		//@}

		/*! \name Rotation constraint */
		//@{
		/*! Overloading of Constraint::constrain_rotation(). Empty.
		 * \param rotation Proposed rotation, unchanged by this implementation.
		 * \param frame Frame receiving the rotation.
		 */
        void constrain_rotation(quat &rotation, Frame *const frame) override {
			(void)(rotation);
			(void)(frame);
		}

		/**
		 * \brief Set rotation constraint type and axis.
		 * \param type Rotation constraint mode.
		 * \param direction Rotation axis.
		 */
		void set_rotation_constraint(Type type, const vec3 &direction);
		/// \brief Set the rotation constraint mode.
		/// \param type Rotation constraint mode.
		void set_rotation_constraint_type(Type type);
		/// \brief Set the rotation constraint axis.
		/// \param direction Axis in the constraint's coordinate system.
		void set_rotation_constraint_direction(const vec3 &direction);

		/*! Returns the rotation constraint Type().
		 * \return Rotation constraint type.
		 */
		Type rotation_constraint_type() const { return rotation_constraint_type_; };
		/*! Returns the axis direction used by the rotation constraint.

		This direction is defined only when rotation_constraint_type() is
		AxisPlaneConstraint::AXIS.

		The AxisPlaneConstraint derived classes express this direction in different
		coordinate system (camera for CameraConstraint, local for LocalConstraint, and
		world for WorldConstraint). This value can be modified with
		set_rotation_constraint_direction().
		\return Rotation axis direction.
		*/
		vec3 rotation_constraint_direction() const { return rotation_constraint_dir_; };
		//@}

	private:
		// int and not Type to allow for overloading and new types definition.
		Type translation_constraint_type_;
		Type rotation_constraint_type_;

		vec3 translation_constraint_dir_;
		vec3 rotation_constraint_dir_;
	};

	/*! \brief An AxisPlaneConstraint defined in the Frame local coordinate system.
	  \class LocalConstraint easy3d/renderer/constraint.h
	  \see AxisPlaneConstraint, WorldConstraint, CameraConstraint

	  \details The translation_constraint_direction() and rotation_constraint_direction() are
	  expressed in the Frame local coordinate system (see Frame::reference_frame()).

	  See the <a href="../examples/constrainedFrame.html">constrainedFrame</a>
	  example for an illustration. */
	class LocalConstraint : public AxisPlaneConstraint {
	public:
		/*! Virtual destructor. Empty. */
        ~LocalConstraint() override = default;

        /// Apply the local-coordinate translation constraint.
        /// \param translation Proposed translation, modified to satisfy the constraint.
        /// \param frame Constrained frame.
        void constrain_translation(vec3 &translation, Frame *const frame) override;
        /// Apply the local-coordinate rotation constraint.
        /// \param rotation Proposed rotation, modified to satisfy the constraint.
        /// \param frame Constrained frame.
        void constrain_rotation(quat &rotation, Frame *const frame) override;
	};

	/*! \brief An AxisPlaneConstraint defined in the world coordinate system.
			\class WorldConstraint easy3d/renderer/constraint.h
			\see AxisPlaneConstraint, LocalConstraint, CameraConstraint

	  \details The translation_constraint_direction() and rotation_constraint_direction() are
	  expressed in world coordinate system.

	  See the <a href="../examples/constrainedFrame.html">constrainedFrame</a> and
	  <a href="../examples/multiView.html">multiView</a> examples for an
	  illustration. */
	class WorldConstraint : public AxisPlaneConstraint {
	public:
		/*! Virtual destructor. Empty. */
        ~WorldConstraint() override = default;

        /// Apply the world-coordinate translation constraint.
        /// \param translation Proposed translation, modified to satisfy the constraint.
        /// \param frame Constrained frame.
        void constrain_translation(vec3 &translation, Frame *const frame) override;
        /// Apply the world-coordinate rotation constraint.
        /// \param rotation Proposed rotation, modified to satisfy the constraint.
        /// \param frame Constrained frame.
        void constrain_rotation(quat &rotation, Frame *const frame) override;
	};

	/*! \brief An AxisPlaneConstraint defined in the camera coordinate system.
	  \class CameraConstraint easy3d/renderer/constraint.h
	  \see AxisPlaneConstraint, LocalConstraint, WorldConstraint

	  \details The translation_constraint_direction() and rotation_constraint_direction() are
	  expressed in the associated camera() coordinate system.

	  See the <a href="../examples/constrainedFrame.html">constrainedFrame</a> and
	  <a href="../examples/constrainedCamera.html">constrainedCamera</a> examples
	  for an illustration. */
	class CameraConstraint : public AxisPlaneConstraint {
	public:
		/**
		 * \brief Create a constraint expressed in a camera coordinate system.
		 * \param camera Camera defining the constraint coordinate system.
		 */
		explicit CameraConstraint(const Camera *const camera);
		/*! Virtual destructor. Empty. */
        ~CameraConstraint() override = default;

        /// Apply the camera-coordinate translation constraint.
        /// \param translation Proposed translation, modified to satisfy the constraint.
        /// \param frame Constrained frame.
        void constrain_translation(vec3 &translation, Frame *const frame) override;
        /// Apply the camera-coordinate rotation constraint.
        /// \param rotation Proposed rotation, modified to satisfy the constraint.
        /// \param frame Constrained frame.
        void constrain_rotation(quat &rotation, Frame *const frame) override;

		/*! Returns the associated Camera. Set using the CameraConstraint constructor.
		 * \return Non-owning pointer to the associated camera.
		 */
		const Camera *camera() const { return camera_; };

	private:
		const Camera *const camera_;
	};


}

#endif // EASY3D_RENDERER_CONSTRAINT_H
