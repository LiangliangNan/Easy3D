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


#ifndef EASY3D_FRAME_H
#define EASY3D_FRAME_H

#include <easy3d/types.h>

#include <set>


namespace easy3d {


	class FrameObserver {
	public:
        virtual ~FrameObserver() {}
		virtual void onFrameModified() = 0;
	};

	class Frame
	{
	public:
		Frame();
		virtual ~Frame() {}

		Frame(const Frame &frame);
		Frame &operator=(const Frame &frame);

	public:
		Frame(const vec3 &position, const quat &orientation);

		void setPosition(const vec3 &position);
		void setPositionWithConstraint(vec3 &position);

		void setOrientation(const quat &orientation);
		void setOrientationWithConstraint(quat &orientation);

        void setPositionAndOrientation(const vec3 &position, const quat &orientation);
        void setPositionAndOrientationWithConstraint(vec3 &position, quat &orientation);

		vec3 position() const;
		quat orientation() const;

	public:

		void setTranslation(const vec3 &translation) {
			t_ = translation;
			frameModified();
		}
		void setTranslationWithConstraint(vec3 &translation);

		void setRotation(const quat &rotation) {
			q_ = rotation;
			frameModified();
		}
		void setRotationWithConstraint(quat &rotation);

		void setTranslationAndRotation(const vec3 &translation, const quat &rotation);
		void setTranslationAndRotationWithConstraint(vec3 &translation, quat &rotation);

		vec3 translation() const { return t_; }

		quat rotation() const { return q_; }

	public:
		const Frame *referenceFrame() const { return referenceFrame_; }
		void setReferenceFrame(const Frame *const refFrame);
		bool settingAsReferenceFrameWillCreateALoop(const Frame *const frame);

		void translate(vec3 &t);
		void translate(const vec3 &t);

		void rotate(quat &q);
		void rotate(const quat &q);

		void rotateAroundPoint(quat &rotation, const vec3 &point);
		void rotateAroundPoint(const quat &rotation, const vec3 &point);

		vec3 coordinatesOf(const vec3 &src) const;
		vec3 inverseCoordinatesOf(const vec3 &src) const;
		vec3 localCoordinatesOf(const vec3 &src) const;
		vec3 localInverseCoordinatesOf(const vec3 &src) const;
		vec3 coordinatesOfIn(const vec3 &src, const Frame *const in) const;
		vec3 coordinatesOfFrom(const vec3 &src, const Frame *const from) const;

		vec3 transformOf(const vec3 &src) const;
		vec3 inverseTransformOf(const vec3 &src) const;
		vec3 localTransformOf(const vec3 &src) const;
		vec3 localInverseTransformOf(const vec3 &src) const;
		vec3 transformOfIn(const vec3 &src, const Frame *const in) const;
		vec3 transformOfFrom(const vec3 &src, const Frame *const from) const;

	public:
		mat4 matrix() const;
		mat4 worldMatrix() const;

		void setFromMatrix(const mat4& m);

		Frame inverse() const;

		Frame worldInverse() const {
			return Frame(-(orientation().inverse_rotate(position())),
				orientation().inverse());
		}

	public:
		void addObserver(FrameObserver* obs) { observers_.insert(obs); }
		void removeObserver(FrameObserver* obs) { observers_.erase(obs); }

	protected:
		// Should be called whenever the position() or orientation() of the Frame is modified.
		// Then all observers will be notified the modification of the frame.
		void frameModified();

	private:
		std::set< FrameObserver* >	observers_;

	private:
		vec3 t_;
		quat q_;

		const Frame *referenceFrame_;
	};

}

#endif // EASY3D_FRAME_H
