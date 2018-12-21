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


#include <easy3d/frame.h>

#include <cmath>


namespace easy3d {

    Frame::Frame() : referenceFrame_(NULL) {}

	Frame::Frame(const vec3 &position, const quat &orientation)
        : t_(position), q_(orientation), referenceFrame_(NULL)
    {}

	Frame &Frame::operator=(const Frame &frame) {
		// Automatic compiler generated version would not emit the modified() signals
		// as is done in setTranslationAndRotation.
		setTranslationAndRotation(frame.translation(), frame.rotation());
		setReferenceFrame(frame.referenceFrame());
		return *this;
	}


	Frame::Frame(const Frame &frame) {
		(*this) = frame;
	}

	mat4 Frame::matrix() const
	{
		mat4 m = q_.matrix();
		m[12] = t_[0];
		m[13] = t_[1];
		m[14] = t_[2];
		return m;
	}


	Frame Frame::inverse() const {
		Frame fr(-(q_.inverse_rotate(t_)), q_.inverse());
		fr.setReferenceFrame(referenceFrame());
		return fr;
	}

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

	void Frame::setFromMatrix(const mat4& m) {
		float w = m(3, 3);
		if (fabs(w) < 1e-8) {
			std::cerr << "Frame::setFromMatrix: Null homogeneous coefficient" << std::endl;
			return;
		}

		mat3 rot;
		for (int i = 0; i < 3; ++i) {
			t_[i] = m(i, 3) / w;
			for (int j = 0; j < 3; ++j)
				rot(i, j) = m(i, j) / w;
		}
		q_.set_from_rotation_matrix(rot);
		frameModified();
	}


	void Frame::translate(const vec3 &t) {
		vec3 tbis = t;
		translate(tbis);
	}

	void Frame::translate(vec3 &t) {
		t_ += t;
		frameModified();
	}


	void Frame::rotate(const quat &q) {
		quat qbis = q;
		rotate(qbis);
	}

	void Frame::rotate(quat &q) {
		q_ *= q;
		q_.normalize(); // Prevents numerical drift
		frameModified();
	}

	void Frame::rotateAroundPoint(quat &rotation, const vec3 &point) {
		q_ *= rotation;
		q_.normalize(); // Prevents numerical drift
		vec3 trans = point +
			quat(inverseTransformOf(rotation.axis()), rotation.angle())
			.rotate(position() - point) -
			t_;
		t_ += trans;
		frameModified();
	}

	void Frame::rotateAroundPoint(const quat &rotation, const vec3 &point) {
		quat rot = rotation;
		rotateAroundPoint(rot, point);
	}


	void Frame::setPosition(const vec3 &position) {
		if (referenceFrame())
			setTranslation(referenceFrame()->coordinatesOf(position));
		else
			setTranslation(position);
	}

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
		frameModified();
	}


	void Frame::setTranslationAndRotation(const vec3 &translation,
		const quat &rotation) {
		t_ = translation;
		q_ = rotation;
		frameModified();
	}

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

	quat Frame::orientation() const {
		quat res = rotation();
		const Frame *fr = referenceFrame();
		while (fr != NULL) {
			res = fr->rotation() * res;
			fr = fr->referenceFrame();
		}
		return res;
	}


	void Frame::setTranslationWithConstraint(vec3 &translation) {
		vec3 deltaT = translation - this->translation();
		setTranslation(this->translation() + deltaT);
		translation = this->translation();
	}

	void Frame::setRotationWithConstraint(quat &rotation) {
		quat deltaQ = this->rotation().inverse() * rotation;
		// Prevent numerical drift
		deltaQ.normalize();

		setRotation(this->rotation() * deltaQ);
		q_.normalize();
		rotation = this->rotation();
	}


	void Frame::setTranslationAndRotationWithConstraint(vec3 &translation,
		quat &rotation) {
		vec3 deltaT = translation - this->translation();
		quat deltaQ = this->rotation().inverse() * rotation;

		// Prevent numerical drift
		deltaQ.normalize();

		t_ += deltaT;
		q_ *= deltaQ;
		q_.normalize();

		translation = this->translation();
		rotation = this->rotation();

		frameModified();
	}


	void Frame::setPositionWithConstraint(vec3 &position) {
		if (referenceFrame())
			position = referenceFrame()->coordinatesOf(position);

		setTranslationWithConstraint(position);
	}

	void Frame::setOrientationWithConstraint(quat &orientation) {
		if (referenceFrame())
			orientation = referenceFrame()->orientation().inverse() * orientation;

		setRotationWithConstraint(orientation);
	}


	void Frame::setPositionAndOrientationWithConstraint(vec3 &position,
		quat &orientation) {
		if (referenceFrame()) {
			position = referenceFrame()->coordinatesOf(position);
			orientation = referenceFrame()->orientation().inverse() * orientation;
		}
		setTranslationAndRotationWithConstraint(position, orientation);
	}


	void Frame::setReferenceFrame(const Frame *const refFrame) {
		if (settingAsReferenceFrameWillCreateALoop(refFrame))
			std::cerr << "Frame::setReferenceFrame would create a loop in Frame hierarchy" << std::endl;
		else {
			bool identical = (referenceFrame_ == refFrame);
			referenceFrame_ = refFrame;
			if (!identical)
				frameModified();
		}
	}


	bool Frame::settingAsReferenceFrameWillCreateALoop(const Frame *const frame) {
		const Frame *f = frame;
		while (f != NULL) {
			if (f == this)
				return true;
			f = f->referenceFrame();
		}
		return false;
	}


	vec3 Frame::coordinatesOf(const vec3 &src) const {
		if (referenceFrame())
			return localCoordinatesOf(referenceFrame()->coordinatesOf(src));
		else
			return localCoordinatesOf(src);
	}


	vec3 Frame::inverseCoordinatesOf(const vec3 &src) const {
		const Frame *fr = this;
		vec3 res = src;
		while (fr != NULL) {
			res = fr->localInverseCoordinatesOf(res);
			fr = fr->referenceFrame();
		}
		return res;
	}

	vec3 Frame::localCoordinatesOf(const vec3 &src) const {
		return rotation().inverse_rotate(src - translation());
	}

	vec3 Frame::localInverseCoordinatesOf(const vec3 &src) const {
		return rotation().rotate(src) + translation();
	}

	vec3 Frame::coordinatesOfFrom(const vec3 &src, const Frame *const from) const {
		if (this == from)
			return src;
		else if (referenceFrame())
			return localCoordinatesOf(referenceFrame()->coordinatesOfFrom(src, from));
		else
			return localCoordinatesOf(from->inverseCoordinatesOf(src));
	}


	vec3 Frame::coordinatesOfIn(const vec3 &src, const Frame *const in) const {
		const Frame *fr = this;
		vec3 res = src;
		while ((fr != NULL) && (fr != in)) {
			res = fr->localInverseCoordinatesOf(res);
			fr = fr->referenceFrame();
		}

		if (fr != in)
			// in was not found in the branch of this, res is now expressed in the world
			// coordinate system. Simply convert to in coordinate system.
			res = in->coordinatesOf(res);

		return res;
	}


	vec3 Frame::transformOf(const vec3 &src) const {
		if (referenceFrame())
			return localTransformOf(referenceFrame()->transformOf(src));
		else
			return localTransformOf(src);
	}


	vec3 Frame::inverseTransformOf(const vec3 &src) const {
		const Frame *fr = this;
		vec3 res = src;
		while (fr != NULL) {
			res = fr->localInverseTransformOf(res);
			fr = fr->referenceFrame();
		}
		return res;
	}


	vec3 Frame::localTransformOf(const vec3 &src) const {
		return rotation().inverse_rotate(src);
	}


	vec3 Frame::localInverseTransformOf(const vec3 &src) const {
		return rotation().rotate(src);
	}

	vec3 Frame::transformOfFrom(const vec3 &src, const Frame *const from) const {
		if (this == from)
			return src;
		else if (referenceFrame())
			return localTransformOf(referenceFrame()->transformOfFrom(src, from));
		else
			return localTransformOf(from->inverseTransformOf(src));
	}


	vec3 Frame::transformOfIn(const vec3 &src, const Frame *const in) const {
		const Frame *fr = this;
		vec3 res = src;
		while ((fr != NULL) && (fr != in)) {
			res = fr->localInverseTransformOf(res);
			fr = fr->referenceFrame();
		}

		if (fr != in)
			// in was not found in the branch of this, res is now expressed in the world
			// coordinate system. Simply convert to in coordinate system.
			res = in->transformOf(res);

		return res;
    }

	void Frame::frameModified() {
		for (auto obs : observers_) {
			obs->onFrameModified();
		}
	}


}
