/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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

#include <easy3d/viewer/frame.h>

#include <cmath>


namespace easy3d {


	/*! Creates a default Frame.

	  Its position() is (0,0,0) and it has an identity orientation() quat. The
	  referenceFrame() and the constraint() are \c NULL. */
    Frame::Frame() : constraint_(nullptr), referenceFrame_(nullptr) {}

	/*! Creates a Frame with a position() and an orientation().

	 See the vec3 and quat documentations for convenient constructors and
	 methods.

	 The Frame is defined in the world coordinate system (its referenceFrame() is \c
	 NULL). It has a \c NULL associated constraint(). */
	Frame::Frame(const vec3 &position, const quat &orientation)
        : t_(position), q_(orientation), constraint_(nullptr), referenceFrame_(nullptr) {}

	/*! Equal operator.

	  The referenceFrame() and constraint() pointers are copied.

	  \attention Signal and slot connections are not copied. */
	Frame &Frame::operator=(const Frame &frame) {
		// Automatic compiler generated version would not emit the modified() signals
		// as is done in setTranslationAndRotation.
		setTranslationAndRotation(frame.translation(), frame.rotation());
		setConstraint(frame.constraint());
		setReferenceFrame(frame.referenceFrame());
		return *this;
	}

	/*! Copy constructor.

	  The translation() and rotation() as well as constraint() and referenceFrame()
	  pointers are copied. */
	Frame::Frame(const Frame &frame) {
		(*this) = frame;
	}


    Frame::~Frame() {}

	/////////////////////////////// MATRICES //////////////////////////////////////

	/*! Returns the 4x4 OpenGL transformation matrix represented by the Frame.

	  This method should be used in conjunction with \c glMultMatrixd() to modify
	  the OpenGL modelview matrix from a Frame hierarchy. With this Frame hierarchy:
	  \code
	  Frame* body     = new Frame();
	  Frame* leftArm  = new Frame();
	  Frame* rightArm = new Frame();
	  leftArm->setReferenceFrame(body);
	  rightArm->setReferenceFrame(body);
	  \endcode

	  The associated OpenGL drawing code should look like:
	  \code
	  void Viewer::draw()
	  {
			glPushMatrix();
			glMultMatrixd(body->matrix());
			drawBody();

			glPushMatrix();
			glMultMatrixd(leftArm->matrix());
			drawArm();
			glPopMatrix();

			glPushMatrix();
			glMultMatrixd(rightArm->matrix());
			drawArm();
			glPopMatrix();

			glPopMatrix();
	  }
	  \endcode
	  Note the use of nested \c glPushMatrix() and \c glPopMatrix() blocks to
	  represent the frame hierarchy: \c leftArm and \c rightArm are both correctly
	  drawn with respect to the \c body coordinate system.

	  This matrix only represents the local Frame transformation (i.e. with respect
	  to the referenceFrame()). Use worldMatrix() to get the full Frame
	  transformation matrix (i.e. from the world to the Frame coordinate system).
	  These two match when the referenceFrame() is \c NULL.

	  The result is only valid until the next call to matrix(), getMatrix(),
	  worldMatrix() or getWorldMatrix(). Use it immediately (as above) or use
	  getMatrix() instead.

	  \attention The OpenGL format of the result is the transpose of the actual
	  mathematical European representation (translation is on the last \e line
	  instead of the last \e column).

	  \note The scaling factor of the 4x4 matrix is 1.0. */
	mat4 Frame::matrix() const
	{
		mat4 m = q_.matrix();
		m[12] = t_[0];
		m[13] = t_[1];
		m[14] = t_[2];
		return m;
	}


	/*! Returns a Frame representing the inverse of the Frame space transformation.

	  The rotation() of the new Frame is the quat::inverse() of the original
	  rotation. Its translation() is the negated inverse rotated image of the
	  original translation.

	  If a Frame is considered as a space rigid transformation (translation and
	  rotation), the inverse() Frame performs the inverse transformation.

	  Only the local Frame transformation (i.e. defined with respect to the
	  referenceFrame()) is inverted. Use worldInverse() for a global inverse.

	  The resulting Frame has the same referenceFrame() as the Frame and a \c NULL
	  constraint().

	  \note The scaling factor of the 4x4 matrix is 1.0. */
	Frame Frame::inverse() const {
		Frame fr(-(q_.inverse_rotate(t_)), q_.inverse());
		fr.setReferenceFrame(referenceFrame());
		return fr;
	}

	/*! Returns the 4x4 OpenGL transformation matrix represented by the Frame.

	  This method should be used in conjunction with \c glMultMatrixd() to modify
	  the OpenGL modelview matrix from a Frame:
	  \code
	  // The modelview here corresponds to the world coordinate system.
	  Frame fr(pos, quat(from, to));
	  glPushMatrix();
	  glMultMatrixd(fr.worldMatrix());
	  // draw object in the fr coordinate system.
	  glPopMatrix();
	  \endcode

	  This matrix represents the global Frame transformation: the entire
	  referenceFrame() hierarchy is taken into account to define the Frame
	  transformation from the world coordinate system. Use matrix() to get the local
	  Frame transformation matrix (i.e. defined with respect to the
	  referenceFrame()). These two match when the referenceFrame() is \c NULL.

	  The OpenGL format of the result is the transpose of the actual mathematical
	  European representation (translation is on the last \e line instead of the
	  last \e column).

	  \attention The result is only valid until the next call to matrix(),
	  getMatrix(), worldMatrix() or getWorldMatrix(). Use it immediately (as above)
	  or use getWorldMatrix() instead.

	  \note The scaling factor of the 4x4 matrix is 1.0. */
	mat4 Frame::worldMatrix() const
	{
		// This test is done for efficiency reasons (creates lots of temp objects otherwise).
		if (referenceFrame())
		{
			static Frame fr;
			fr.setTranslation(position());
			fr.setRotation(orientation());
			return fr.matrix();
		}
		else
			return matrix();
	}


	/*! Sets the Frame from an OpenGL matrix representation (rotation in the upper
	 left 3x3 matrix and translation on the last line).

	 Hence, if a code fragment looks like:
	 \code
	 double m[16]={...};
	 glMultMatrixd(m);
	 \endcode
	 It is equivalent to write:
	 \code
	 Frame fr;
	 fr.setFromMatrix(m);
	 glMultMatrixd(fr.matrix());
	 \endcode

	 Using this conversion, you can benefit from the powerful Frame transformation
	 methods to translate points and vectors to and from the Frame coordinate system
	 to any other Frame coordinate system (including the world coordinate system).
	 See coordinatesOf() and transformOf().

	 Emits the modified() signal. See also matrix(), getMatrix() and
	 quat::setFromRotationMatrix().

	 \attention A Frame does not contain a scale factor. The possible scaling in \p
	 m will not be converted into the Frame by this method. */
	void Frame::setFromMatrix(const mat4& m) {
		float w = m(3, 3);
		if (fabs(w) < 1e-8) {
			LOG(ERROR) << "Frame::setFromMatrix: Null homogeneous coefficient";
			return;
		}

		mat3 rot;
		for (int i = 0; i < 3; ++i) {
			t_[i] = m(i, 3) / w;
			for (int j = 0; j < 3; ++j)
				rot(i, j) = m(i, j) / w;
		}
		q_.set_from_rotation_matrix(rot);
        trigger();
	}


	//////////////////// SET AND GET LOCAL TRANSLATION AND ROTATION
	//////////////////////////////////

	////////////////////////////////////////////////////////////////////////////////

	/*! Translates the Frame of \p t (defined in the Frame coordinate system).

	  The translation actually applied to the Frame may differ from \p t since it
	  can be filtered by the constraint(). Use translate(vec3&) or
	  setTranslationWithConstraint() to retrieve the filtered translation value. Use
	  setTranslation() to directly translate the Frame without taking the
	  constraint() into account.

	  See also rotate(const quat&). Emits the modified() signal. */
	void Frame::translate(const vec3 &t) {
		vec3 tbis = t;
		translate(tbis);
	}

	/*! Same as translate(const vec3&) but \p t may be modified to satisfy the
	  translation constraint(). Its new value corresponds to the translation that
	  has actually been applied to the Frame. */
	void Frame::translate(vec3 &t) {
		if (constraint())
			constraint()->constrainTranslation(t, this);
		t_ += t;
        trigger();
	}

	/*! Rotates the Frame by \p q (defined in the Frame coordinate system): R = R*q.

	  The rotation actually applied to the Frame may differ from \p q since it can
	  be filtered by the constraint(). Use rotate(quat&) or
	  setRotationWithConstraint() to retrieve the filtered rotation value. Use
	  setRotation() to directly rotate the Frame without taking the constraint()
	  into account.

	  See also translate(const vec3&). Emits the modified() signal. */
	void Frame::rotate(const quat &q) {
		quat qbis = q;
		rotate(qbis);
	}

	/*! Same as rotate(const quat&) but \p q may be modified to satisfy the
	  rotation constraint(). Its new value corresponds to the rotation that has
	  actually been applied to the Frame. */
	void Frame::rotate(quat &q) {
		if (constraint())
			constraint()->constrainRotation(q, this);
		q_ *= q;
		q_.normalize(); // Prevents numerical drift
        trigger();
	}

	/*! Makes the Frame rotate() by \p rotation around \p point.

	  \p point is defined in the world coordinate system, while the \p rotation axis
	  is defined in the Frame coordinate system.

	  If the Frame has a constraint(), \p rotation is first constrained using
	  Constraint::constrainRotation(). The translation which results from the
	  filtered rotation around \p point is then computed and filtered using
	  Constraint::constrainTranslation(). The new \p rotation value corresponds to
	  the rotation that has actually been applied to the Frame.

	  Emits the modified() signal. */
	void Frame::rotateAroundPoint(quat &rotation, const vec3 &point) {
		if (constraint())
			constraint()->constrainRotation(rotation, this);
		q_ *= rotation;
		q_.normalize(); // Prevents numerical drift
		vec3 trans = point +
			quat(inverseTransformOf(rotation.axis()), rotation.angle())
			.rotate(position() - point) -
			t_;
		if (constraint())
			constraint()->constrainTranslation(trans, this);
		t_ += trans;
        trigger();
	}

	/*! Same as rotateAroundPoint(), but with a \c const \p rotation quat.
	  Note that the actual rotation may differ since it can be filtered by the
	  constraint(). */
	void Frame::rotateAroundPoint(const quat &rotation, const vec3 &point) {
		quat rot = rotation;
		rotateAroundPoint(rot, point);
	}

	//////////////////// SET AND GET WORLD POSITION AND ORIENTATION
	//////////////////////////////////

	/*! Sets the position() of the Frame, defined in the world coordinate system.
	Emits the modified() signal.

	Use setTranslation() to define the \e local frame translation (with respect to
	the referenceFrame()). The potential constraint() of the Frame is not taken into
	account, use setPositionWithConstraint() instead. */
	void Frame::setPosition(const vec3 &position) {
		if (referenceFrame())
			setTranslation(referenceFrame()->coordinatesOf(position));
		else
			setTranslation(position);
	}

	/*! Same as successive calls to setPosition() and then setOrientation().

	Only one modified() signal is emitted, which is convenient if this signal is
	connected to a Viewer::update() slot. See also setTranslationAndRotation()
	and setPositionAndOrientationWithConstraint(). */
	void Frame::setPositionAndOrientation(const vec3 &position,
		const quat &orientation) {
		if (referenceFrame()) {
			t_ = referenceFrame()->coordinatesOf(position);
			q_ = referenceFrame()->orientation().inverse() * orientation;
		}
		else {
			t_ = position;
			q_ = orientation;
		}
        trigger();
	}

	/*! Same as successive calls to setTranslation() and then setRotation().

	Only one modified() signal is emitted, which is convenient if this signal is
	connected to a Viewer::update() slot. See also setPositionAndOrientation()
	and setTranslationAndRotationWithConstraint(). */
	void Frame::setTranslationAndRotation(const vec3 &translation,
		const quat &rotation) {
		t_ = translation;
		q_ = rotation;
        trigger();
	}

	/*! Sets the orientation() of the Frame, defined in the world coordinate system.
	Emits the modified() signal.

	Use setRotation() to define the \e local frame rotation (with respect to the
	referenceFrame()). The potential constraint() of the Frame is not taken into
	account, use setOrientationWithConstraint() instead. */
	void Frame::setOrientation(const quat &orientation) {
		if (referenceFrame())
			setRotation(referenceFrame()->orientation().inverse() * orientation);
		else
			setRotation(orientation);
	}


	/*! Returns the position of the Frame, defined in the world coordinate system.
	   See also orientation(), setPosition() and translation(). */
	vec3 Frame::position() const {
		if (referenceFrame_)
			return inverseCoordinatesOf(vec3(0.0, 0.0, 0.0));
		else
			return t_;
	}

	/*! Returns the orientation of the Frame, defined in the world coordinate
	  system. See also position(), setOrientation() and rotation(). */
	quat Frame::orientation() const {
		quat res = rotation();
		const Frame *fr = referenceFrame();
        while (fr != nullptr) {
			res = fr->rotation() * res;
			fr = fr->referenceFrame();
		}
		return res;
	}

	////////////////////// C o n s t r a i n t   V e r s i o n s
	/////////////////////////////

	/*! Same as setTranslation(), but \p translation is modified so that the
	  potential constraint() of the Frame is satisfied.

	  Emits the modified() signal. See also setRotationWithConstraint() and
	  setPositionWithConstraint(). */
	void Frame::setTranslationWithConstraint(vec3 &translation) {
		vec3 deltaT = translation - this->translation();
		if (constraint())
			constraint()->constrainTranslation(deltaT, this);

		setTranslation(this->translation() + deltaT);
		translation = this->translation();
	}

	/*! Same as setRotation(), but \p rotation is modified so that the potential
	  constraint() of the Frame is satisfied.

	  Emits the modified() signal. See also setTranslationWithConstraint() and
	  setOrientationWithConstraint(). */
	void Frame::setRotationWithConstraint(quat &rotation) {
		quat deltaQ = this->rotation().inverse() * rotation;
		if (constraint())
			constraint()->constrainRotation(deltaQ, this);

		// Prevent numerical drift
		deltaQ.normalize();

		setRotation(this->rotation() * deltaQ);
		q_.normalize();
		rotation = this->rotation();
	}

	/*! Same as setTranslationAndRotation(), but \p translation and \p orientation
	  are modified to satisfy the constraint(). Emits the modified() signal. */
	void Frame::setTranslationAndRotationWithConstraint(vec3 &translation,
		quat &rotation) {
		vec3 deltaT = translation - this->translation();
		quat deltaQ = this->rotation().inverse() * rotation;

		if (constraint()) {
			constraint()->constrainTranslation(deltaT, this);
			constraint()->constrainRotation(deltaQ, this);
		}

		// Prevent numerical drift
		deltaQ.normalize();

		t_ += deltaT;
		q_ *= deltaQ;
		q_.normalize();

		translation = this->translation();
		rotation = this->rotation();

        trigger();
	}

	/*! Same as setPosition(), but \p position is modified so that the potential
	  constraint() of the Frame is satisfied. See also
	  setOrientationWithConstraint() and setTranslationWithConstraint(). */
	void Frame::setPositionWithConstraint(vec3 &position) {
		if (referenceFrame())
			position = referenceFrame()->coordinatesOf(position);

		setTranslationWithConstraint(position);
	}

	/*! Same as setOrientation(), but \p orientation is modified so that the
	  potential constraint() of the Frame is satisfied. See also
	  setPositionWithConstraint() and setRotationWithConstraint(). */
	void Frame::setOrientationWithConstraint(quat &orientation) {
		if (referenceFrame())
			orientation = referenceFrame()->orientation().inverse() * orientation;

		setRotationWithConstraint(orientation);
	}

	/*! Same as setPositionAndOrientation() but \p position and \p orientation are
	modified to satisfy the constraint. Emits the modified() signal. */
	void Frame::setPositionAndOrientationWithConstraint(vec3 &position,
		quat &orientation) {
		if (referenceFrame()) {
			position = referenceFrame()->coordinatesOf(position);
			orientation = referenceFrame()->orientation().inverse() * orientation;
		}
		setTranslationAndRotationWithConstraint(position, orientation);
	}

	///////////////////////////// REFERENCE FRAMES
	//////////////////////////////////////////

	/*! Sets the referenceFrame() of the Frame.

	The Frame translation() and rotation() are then defined in the referenceFrame()
	coordinate system. Use position() and orientation() to express these in the
	world coordinate system.

	Emits the modified() signal if \p refFrame differs from the current
	referenceFrame().

	Using this method, you can create a hierarchy of Frames. This hierarchy needs to
	be a tree, which root is the world coordinate system (i.e. a \c NULL
	referenceFrame()). A warning is printed and no action is performed if setting \p
	refFrame as the referenceFrame() would create a loop in the Frame hierarchy (see
	settingAsReferenceFrameWillCreateALoop()). */
	void Frame::setReferenceFrame(const Frame *const refFrame) {
		if (settingAsReferenceFrameWillCreateALoop(refFrame))
			LOG(ERROR) << "Frame::setReferenceFrame would create a loop in Frame hierarchy";
		else {
			bool identical = (referenceFrame_ == refFrame);
			referenceFrame_ = refFrame;
			if (!identical)
                trigger();
		}
	}

	/*! Returns \c true if setting \p frame as the Frame's referenceFrame() would
	  create a loop in the Frame hierarchy. */
	bool Frame::settingAsReferenceFrameWillCreateALoop(const Frame *const frame) {
		const Frame *f = frame;
        while (f != nullptr) {
			if (f == this)
				return true;
			f = f->referenceFrame();
		}
		return false;
	}

	///////////////////////// FRAME TRANSFORMATIONS OF 3D POINTS
	/////////////////////////////////

	/*! Returns the Frame coordinates of a point \p src defined in the world
	 coordinate system (converts from world to Frame).

	 inverseCoordinatesOf() performs the inverse convertion. transformOf() converts
	 3D vectors instead of 3D coordinates.

	 See the <a href="../examples/frameTransform.html">frameTransform example</a>
	 for an illustration. */
	vec3 Frame::coordinatesOf(const vec3 &src) const {
		if (referenceFrame())
			return localCoordinatesOf(referenceFrame()->coordinatesOf(src));
		else
			return localCoordinatesOf(src);
	}

	/*! Returns the world coordinates of the point whose position in the Frame
	  coordinate system is \p src (converts from Frame to world).

	  coordinatesOf() performs the inverse convertion. Use inverseTransformOf() to
	  transform 3D vectors instead of 3D coordinates. */
	vec3 Frame::inverseCoordinatesOf(const vec3 &src) const {
		const Frame *fr = this;
		vec3 res = src;
        while (fr != nullptr) {
			res = fr->localInverseCoordinatesOf(res);
			fr = fr->referenceFrame();
		}
		return res;
	}

	/*! Returns the Frame coordinates of a point \p src defined in the
	  referenceFrame() coordinate system (converts from referenceFrame() to Frame).

	  localInverseCoordinatesOf() performs the inverse convertion. See also
	  localTransformOf(). */
	vec3 Frame::localCoordinatesOf(const vec3 &src) const {
		return rotation().inverse_rotate(src - translation());
	}

	/*! Returns the referenceFrame() coordinates of a point \p src defined in the
	 Frame coordinate system (converts from Frame to referenceFrame()).

	 localCoordinatesOf() performs the inverse convertion. See also
	 localInverseTransformOf(). */
	vec3 Frame::localInverseCoordinatesOf(const vec3 &src) const {
		return rotation().rotate(src) + translation();
	}

	/*! Returns the Frame coordinates of the point whose position in the \p from
	  coordinate system is \p src (converts from \p from to Frame).

	  coordinatesOfIn() performs the inverse transformation. */
	vec3 Frame::coordinatesOfFrom(const vec3 &src, const Frame *const from) const {
		if (this == from)
			return src;
		else if (referenceFrame())
			return localCoordinatesOf(referenceFrame()->coordinatesOfFrom(src, from));
		else
			return localCoordinatesOf(from->inverseCoordinatesOf(src));
	}

	/*! Returns the \p in coordinates of the point whose position in the Frame
	  coordinate system is \p src (converts from Frame to \p in).

	  coordinatesOfFrom() performs the inverse transformation. */
	vec3 Frame::coordinatesOfIn(const vec3 &src, const Frame *const in) const {
		const Frame *fr = this;
		vec3 res = src;
        while ((fr != nullptr) && (fr != in)) {
			res = fr->localInverseCoordinatesOf(res);
			fr = fr->referenceFrame();
		}

		if (fr != in)
			// in was not found in the branch of this, res is now expressed in the world
			// coordinate system. Simply convert to in coordinate system.
			res = in->coordinatesOf(res);

		return res;
	}

	///////////////////////// FRAME TRANSFORMATIONS OF VECTORS
	/////////////////////////////////

	/*! Returns the Frame transform of a vector \p src defined in the world
	 coordinate system (converts vectors from world to Frame).

	 inverseTransformOf() performs the inverse transformation. coordinatesOf()
	 converts 3D coordinates instead of 3D vectors (here only the rotational part of
	 the transformation is taken into account).

	 See the <a href="../examples/frameTransform.html">frameTransform example</a>
	 for an illustration. */
	vec3 Frame::transformOf(const vec3 &src) const {
		if (referenceFrame())
			return localTransformOf(referenceFrame()->transformOf(src));
		else
			return localTransformOf(src);
	}

	/*! Returns the world transform of the vector whose coordinates in the Frame
	  coordinate system is \p src (converts vectors from Frame to world).

	  transformOf() performs the inverse transformation. Use inverseCoordinatesOf()
	  to transform 3D coordinates instead of 3D vectors. */
	vec3 Frame::inverseTransformOf(const vec3 &src) const {
		const Frame *fr = this;
		vec3 res = src;
        while (fr != nullptr) {
			res = fr->localInverseTransformOf(res);
			fr = fr->referenceFrame();
		}
		return res;
	}

	/*! Returns the Frame transform of a vector \p src defined in the
	  referenceFrame() coordinate system (converts vectors from referenceFrame() to
	  Frame).

	  localInverseTransformOf() performs the inverse transformation. See also
	  localCoordinatesOf(). */
	vec3 Frame::localTransformOf(const vec3 &src) const {
		return rotation().inverse_rotate(src);
	}

	/*! Returns the referenceFrame() transform of a vector \p src defined in the
	 Frame coordinate system (converts vectors from Frame to referenceFrame()).

	 localTransformOf() performs the inverse transformation. See also
	 localInverseCoordinatesOf(). */
	vec3 Frame::localInverseTransformOf(const vec3 &src) const {
		return rotation().rotate(src);
	}

	/*! Returns the Frame transform of the vector whose coordinates in the \p from
	  coordinate system is \p src (converts vectors from \p from to Frame).

	  transformOfIn() performs the inverse transformation. */
	vec3 Frame::transformOfFrom(const vec3 &src, const Frame *const from) const {
		if (this == from)
			return src;
		else if (referenceFrame())
			return localTransformOf(referenceFrame()->transformOfFrom(src, from));
		else
			return localTransformOf(from->inverseTransformOf(src));
	}

	/*! Returns the \p in transform of the vector whose coordinates in the Frame
	  coordinate system is \p src (converts vectors from Frame to \p in).

	  transformOfFrom() performs the inverse transformation. */
	vec3 Frame::transformOfIn(const vec3 &src, const Frame *const in) const {
		const Frame *fr = this;
		vec3 res = src;
        while ((fr != nullptr) && (fr != in)) {
			res = fr->localInverseTransformOf(res);
			fr = fr->referenceFrame();
		}

		if (fr != in)
			// in was not found in the branch of this, res is now expressed in the world
			// coordinate system. Simply convert to in coordinate system.
			res = in->transformOf(res);

		return res;
	}

	/////////////////////////////////   ALIGN   /////////////////////////////////

	/*! Aligns the Frame with \p frame, so that two of their axis are parallel.

	If one of the X, Y and Z axis of the Frame is almost parallel to any of the X,
	Y, or Z axis of \p frame, the Frame is rotated so that these two axis actually
	become parallel.

	If, after this first rotation, two other axis are also almost parallel, a second
	alignment is performed. The two frames then have identical orientations, up to
	90 degrees rotations.

	\p threshold measures how close two axis must be to be considered parallel. It
	is compared with the absolute values of the dot product of the normalized axis.
	As a result, useful range is sqrt(2)/2 (systematic alignment) to 1 (no
	alignment).

	When \p move is set to \c true, the Frame position() is also affected by the
	alignment. The new Frame's position() is such that the \p frame position
	(computed with coordinatesOf(), in the Frame coordinates system) does not
	change.

	\p frame may be \c NULL and then represents the world coordinate system (same
	convention than for the referenceFrame()).

	The rotation (and translation when \p move is \c true) applied to the Frame are
	filtered by the possible constraint(). */
	void Frame::alignWithFrame(const Frame *const frame, bool move,
		float threshold) {
		vec3 directions[2][3];
		for (unsigned short d = 0; d < 3; ++d) {
			vec3 dir((d == 0) ? 1.0f : 0.0f, (d == 1) ? 1.0f : 0.0f, (d == 2) ? 1.0f : 0.0f);
			if (frame)
				directions[0][d] = frame->inverseTransformOf(dir);
			else
				directions[0][d] = dir;
			directions[1][d] = inverseTransformOf(dir);
		}

		float maxProj = 0.0;
		float proj;
		unsigned short index[2];
		index[0] = index[1] = 0;
		for (unsigned short i = 0; i < 3; ++i)
			for (unsigned short j = 0; j < 3; ++j)
				if ((proj = fabs(dot(directions[0][i], directions[1][j]))) >= maxProj)
				{
					index[0] = i;
					index[1] = j;
					maxProj = proj;
				}

		Frame old;
		old = *this;

		float coef = dot(directions[0][index[0]], directions[1][index[1]]);
		if (fabs(coef) >= threshold) {
			const vec3 axis = cross(directions[0][index[0]], directions[1][index[1]]);
			float angle = asin(axis.norm());
			if (coef >= 0.0)
				angle = -angle;
			rotate(rotation().inverse() * quat(axis, angle) * orientation());

			// Try to align an other axis direction
			unsigned short d = (index[1] + 1) % 3;
			vec3 dir((d == 0) ? 1.0f : 0.0f, (d == 1) ? 1.0f : 0.0f, (d == 2) ? 1.0f : 0.0f);
			dir = inverseTransformOf(dir);

			float max = 0.0;
			for (unsigned short i = 0; i < 3; ++i) {
				float proj = fabs(dot(directions[0][i], dir));
				if (proj > max) {
					index[0] = i;
					max = proj;
				}
			}

			if (max >= threshold) {
				const vec3 axis = cross(directions[0][index[0]], dir);
				float angle = asin(axis.norm());
				if (dot(directions[0][index[0]], dir) >= 0.0f)
					angle = -angle;
				rotate(rotation().inverse() * quat(axis, angle) * orientation());
			}
		}

		if (move) {
			vec3 center;
			if (frame)
				center = frame->position();

			translate(center - orientation().rotate(old.coordinatesOf(center)) -
				translation());
		}
	}

	/*! Translates the Frame so that its position() lies on the line defined by \p
	origin and \p direction (defined in the world coordinate system).

	Simply uses an orthogonal projection. \p direction does not need to be
	normalized. */
	void Frame::projectOnLine(const vec3 &origin, const vec3 &direction) {
		// If you are trying to find a bug here, because of memory problems, you waste
		// your time. This is a bug in the gcc 3.3 compiler. Compile the library in
		// debug mode and test. Uncommenting this line also seems to solve the
		// problem. Horrible. cout << "position = " << position() << endl; If you
		// found a problem or are using a different compiler, please let me know.
		const vec3& shift = origin - position();
		const Line3& line = Line3::from_point_and_direction(vec3(0, 0, 0), vec3(direction));
		const vec3& proj = line.projection(shift);
		translate(shift - proj);
	}

}
