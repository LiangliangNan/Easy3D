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

#ifndef EASY3D_CAMERA_H
#define EASY3D_CAMERA_H


#include <easy3d/types.h>
#include <easy3d/frame.h>


namespace easy3d {

	class Viewer;
	class Frame;
	class ManipulatedCameraFrame;


	class Camera : public FrameObserver
	{
	public:
		Camera();
		virtual ~Camera();

		Camera(const Camera &camera);
		Camera &operator=(const Camera &camera);

		enum Type { PERSPECTIVE, ORTHOGRAPHIC };

	public:
		vec3 position() const;
		vec3 upVector() const;
		vec3 viewDirection() const;
		vec3 rightVector() const;
		quat orientation() const;

		void setFromModelViewMatrix(const mat4& mv);
		void setFromProjectionMatrix(const mat34& proj);

	public:
		void setPosition(const vec3 &pos);
		void setOrientation(const quat &q);
		void setOrientation(float theta, float phi);
		void setUpVector(const vec3 &up, bool noMove = true);
		void setViewDirection(const vec3 &direction);

	public:
		void lookAt(const vec3 &target);
		void showEntireScene();
		void fitSphere(const vec3 &center, float radius);
		void fitBoundingBox(const vec3 &min, const vec3 &max);
		void fitScreenRegion(int xmin, int ymin, int xmax, int ymax);
		void centerScene();

	public:
		Type type() const { return type_; }

		float fieldOfView() const { return fieldOfView_; }

		float horizontalFieldOfView() const {
			return 2.0f * atan(tan(fieldOfView() / 2.0f) * aspectRatio());
		}

		float aspectRatio() const {
			return screenWidth_ / static_cast<float>(screenHeight_);
		}

		int screenWidth() const { return screenWidth_; }


		int screenHeight() const { return screenHeight_; }
		float pixelGLRatio(const vec3 &position) const;

		float zNearCoefficient() const { return zNearCoef_; }

		float zClippingCoefficient() const { return zClippingCoef_; }

		virtual float zNear() const;
        virtual float zFar() const;
        virtual void getOrthoWidthHeight(float &halfWidth, float &halfHeight) const;

	public:
		void setType(Type type);

		void setFieldOfView(float fov);

		void setHorizontalFieldOfView(float hfov) {
			setFieldOfView(2.0f * atan(tan(hfov / 2.0f) / aspectRatio()));
		}

		void setFOVToFitScene();

		void setAspectRatio(float aspect) {
			setScreenWidthAndHeight(int(100.0 * aspect), 100);
		}

		void setScreenWidthAndHeight(int width, int height);

		void setZNearCoefficient(float coef) {
			zNearCoef_ = coef;
			projectionMatrixIsUpToDate_ = false;
		}

        void setZClippingCoefficient(float coef) {
			zClippingCoef_ = coef;
			projectionMatrixIsUpToDate_ = false;
		}

	public:

		float sceneRadius() const { return sceneRadius_; }

		vec3 sceneCenter() const { return sceneCenter_; }
		float distanceToSceneCenter() const;

	public:
		void setSceneRadius(float radius);
		void setSceneCenter(const vec3 &center);
		void setSceneBoundingBox(const vec3 &min, const vec3 &max);

	public:
		ManipulatedCameraFrame *frame() const { return frame_; }
		void setFrame(ManipulatedCameraFrame *const mcf);

	public:
		void computeProjectionMatrix();
		void computeModelViewMatrix();

		const mat4& projectionMatrix() const;
		const mat4& modelViewMatrix() const;
		mat4 modelViewProjectionMatrix() const;

	public:
		vec3 cameraCoordinatesOf(const vec3 &src) const;
		vec3 worldCoordinatesOf(const vec3 &src) const;

	public:
		vec3 projectedCoordinatesOf(const vec3 &src, const Frame *frame = NULL) const;
		vec3 unprojectedCoordinatesOf(const vec3 &src, const Frame *frame = NULL) const;
		void convertClickToLine(int x, int y, vec3 &orig, vec3 &dir) const;

	private:
		void onFrameModified();

	private:
		ManipulatedCameraFrame *frame_;

		// Camera parameters
		int screenWidth_, screenHeight_; // size of the window, in pixels
		float fieldOfView_;              // in radians
		vec3  sceneCenter_;
		float sceneRadius_; // OpenGL units
		float zNearCoef_;
		float zClippingCoef_;
		float orthoCoef_;
		Type type_;                 // PERSPECTIVE or ORTHOGRAPHIC
		mat4 modelViewMatrix_;		// Buffered model view matrix.
		bool modelViewMatrixIsUpToDate_;
		mat4 projectionMatrix_;		// Buffered projection matrix.
		bool projectionMatrixIsUpToDate_;

		friend class Viewer;
	};

}

#endif // EASY3D_CAMERA_H
