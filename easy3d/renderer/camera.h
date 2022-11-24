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

#ifndef EASY3D_RENDERER_CAMERA_H
#define EASY3D_RENDERER_CAMERA_H


#include <easy3d/core/types.h>
#include <easy3d/core/signal.h>
#include <easy3d/renderer/frame.h>


namespace easy3d {


	class Viewer;
	class Frame;
	class ManipulatedCameraFrame;
    class KeyFrameInterpolator;

	/*! \brief A perspective or orthographic camera.
	  \class Camera easy3d/renderer/camera.h

	  \details A Camera defines some intrinsic parameters (fieldOfView(), position(), viewDirection(), upVector()...)
	  and useful positioning tools that ease its placement (showEntireScene(), fitSphere(), lookAt()...). It exports
	  its associated OpenGL projection and modelview matrices and can interactively be modified using the mouse.

	  <h3>Mouse manipulation</h3>

	  The position() and orientation() of the Camera are defined by a ManipulatedFrame (retrieved using frame()).
	  These methods are just convenient wrappers to the equivalent Frame methods. This also means that the Camera
	  frame() can be attached to a Frame::referenceFrame() which enables complex Camera setups.

	  If a pivotPoint() has been set, the Camera observes a scene and rotates around its pivotPoint().

	  <h3>Other functionalities</h3>

	  The type() of the Camera can be Camera::ORTHOGRAPHIC or Camera::PERSPECTIVE (see Type()). fieldOfView() is
	  meaningless with Camera::ORTHOGRAPHIC.

	  The near and far planes of the Camera are fitted to the scene and determined from scene radius, scene center,
	  and zClippingCoefficient() by the zNear() and zFar() methods. Reasonable values on the scene extends hence have
	  to be provided to the Camera to correctly display the scene. High level positioning methods also use this
	  information (showEntireScene(), centerScene()...).

	  A Camera holds KeyFrameInterpolator that can be used to save Camera positions and paths. You can interactively
	  addKeyFrameToPath() to a given path. Use playPath() to make the Camera follow the path.

	  Use cameraCoordinatesOf() and worldCoordinatesOf() to convert to and from the Camera frame() coordinate system.
	  projectedCoordinatesOf() and unprojectedCoordinatesOf() will convert from screen to 3D coordinates.
	  convertClickToLine() is very useful for analytical object selection.

	  A Camera can also be used outside of a viewer or even without OpenGL for its coordinate system conversion
	  capabilities. Note however that some of them explicitly rely on the presence of a Z-buffer.

	 * To use the camera, you need to do the following:
     *  - Create and (if necessary) setup camera in the constructor of your viewer.
     *      \code
     *      camera_ = new Camera;
     *      camera_->setType(Camera::PERSPECTIVE);
     *      camera_->setUpVector(vec3(0, 0, 1));        // Z pointing up
     *      camera_->setViewDirection(vec3(-1, 0, 0));  // X pointing out
     *      camera_->showEntireScene();                 // position and orient the camera
     *      camera_->connect(this, &Viewer::update);    // connect the viewer's update function
     *      \endcode
     *  - Call <code> camera_->setScreenWidthAndHeight() </code> at both
     *      - creation or initialization (i.e., before the viewer appears) of the application;
     *      - change of the window size.
     *  - Call <code> camera_->frame()->action_start() </code> on mouse down and
     *         <code> camera_->frame()->action_end() </code> on mouse up.
     *  - Call <code> camera_->frame()->action_rotate() </code> on mouse move for rotation and
     *         <code> camera_->frame()->action_translate() </code> on mouse move for translation and
     *         <code> camera_->frame()->action_zoom() </code> on mouse move for zoom
     *
     * To make the entire scene visible, call <code> camera_->setSceneBoundingBox() </code> and
     * <code> camera_->showEntireScene() </code> ;
     *
     * To retrieve the model view projection matrix, call <code> camera_->modelViewProjectionMatrix() </code>
	  */
    class Camera
	{
	public:
		Camera();
		virtual ~Camera();

		/// \brief Copy constructor
		Camera(const Camera &camera);
		Camera &operator=(const Camera &camera);

		/**
		 * \brief Enumerates the two possible types of Camera.
		 * \details See type() and setType(). This type mainly defines different Camera projection matrix (see
		 * loadProjectionMatrix()). Many other methods (convertClickToLine(), projectedCoordinatesOf(),
		 * pixelGLRatio()...) are affected by this Type.
		 */
		enum Type { PERSPECTIVE, ORTHOGRAPHIC };

		/*! @name Position and orientation */
		//@{
	public:
		vec3 position() const;
		vec3 upVector() const;
		vec3 viewDirection() const;
		vec3 rightVector() const;
		quat orientation() const;

        // -----------------------------------------------------------------

        /**
         * \brief Sets the Camera's position() and orientation() from an OpenGL ModelView matrix.
         * \details This enables a Camera initialisation from another OpenGL application. After this method has been
         * called, getModelViewMatrix() returns a matrix equivalent to \p mv.
         * Only the orientation() and position() of the Camera are modified.
         */
		void set_from_model_view_matrix(const mat4& mv) const;

        /** \brief Defines the position(), orientation() and fieldOfView() of the camera from calibrated camera
         *      intrinsic and extrinsic parameters. This is an overload of set_from_calibration().
         *  \param fx, fy: the focal length
         *  \param cx, cy: the principal point
         *  \param skew: distortion
         *  \param R: the rotation matrix. It denotes the coordinate system transformation from 3D world coordinates
         *      to 3D camera coordinates.
         *  \param t: the camera translation. It is the position of the origin of the world coordinate system expressed
         *      in the camera coordinate system. \note t is often mistakenly considered the position of the camera. The
         *      position C of the camera expressed in world coordinates is C = -inverse(rot) * t = -transpose(rot) * t.
         *  \param convert: \c true to convert from vision convention to OpenGL convention (i.e., invert Y and Z axes).
         *                  This is because the camera coordinates of computer vision goes X right, Y down, Z forward,
         *                  while the camera coordinates of OpenGL goes X right, Y up, Z inward.
         *  \attention This function assumes the camera parameters were obtained by standard camera calibration, in
         *      which image coordinates are denoted in pixels, with the origin point (0, 0) corresponding to the
         *      top-left corner of the image. The X axis starts at the left edge of an image and goes towards the right
         *      edge. The Y axis starts at the top of the image towards image bottom. All image pixels have non-negative
         *      coordinates.
         *  \sa set_from_calibration.
         */
        // Some useful transformations
        //     - camera position: -transpose(R) * t. Because when transforming from world to camera: P' = R * P + t.
        //     - view direction: transpose(R) * vec3(0, 0, -1), which is actually the negative Z axis of the frame.
        void set_from_calibration(float fx, float fy, float skew, float cx, float cy,
                                  const mat3& R, const vec3& t, bool convert = true);

        /** \brief Defines the position(), orientation() and fieldOfView() of the camera from calibrated camera
         *      intrinsic and extrinsic parameters. This is an overload of set_from_calibration().
         *  \param proj The projection matrix, which can be computed as P = K * M * [R|t], where R is a
         *              3x3 matrix representing the camera rotation and T is a 3-vector describing the camera
         *              translation (rotation first then translation).
         *  \attention: M is a 3 by 3 matrix. In case you need to convert from the vision convention to the
         *              OpenGL convention (i.e., invert Y and Z axes), M(1, 1) and M(2, 2) must be set to -1, i.e.,
         *              M(1, 1) = -1;   // invert the Y axis
         *              M(2, 2) = -1;   // invert the Z axis
         *              This is because the camera coordinates in computer vision goes X right, Y down, Z forward,
         *              while the camera coordinates in OpenGL goes X right, Y up, Z inward.
         *  \attention This function assumes the camera parameters were obtained by standard camera calibration, in
         *      which image coordinates are denoted in pixels, with the origin point (0, 0) corresponding to the
         *      top-left corner of the image. The X axis starts at the left edge of an image and goes towards the right
         *      edge. The Y axis starts at the top of the image towards image bottom. All image pixels have non-negative
         *      coordinates.
         *  \sa set_from_calibration.
         */
        void set_from_calibration(const mat34& proj);

        // -----------------------------------------------------------------

        /**
         * \brief Temporally change the projection matrix.
         * @param proj The projection matrix.
         * @attention This function directly changes the projection matrix. It actually doesn't change any of the camera
         *      parameters (i.e., position, orientation, fov, zFar, zNear, etc.). This is only useful if you want to
         *      temporally use a different projection matrix for rendering (e.g., grabbing a large snapshot from the
         *      framebuffer).
         */
        void set_projection_matrix(const mat4& proj);

        /**
         * \brief Temporally change the modelview matrix.
         * @param mv The modelview matrix.
         * @attention This function directly changes the modelview matrix. It actually doesn't change any of the camera
         *      parameters (i.e., position, orientation, fov, zFar, zNear, etc.). This is only useful if you want to
         *      temporally use a different projection matrix for rendering (e.g., grabbing a large snapshot from the
         *      framebuffer).
         */
        void set_modelview_matrix(const mat4& mv);

        // -----------------------------------------------------------------

	public:
		void setPosition(const vec3 &pos) const;
		void setOrientation(const quat &q) const;
		void setOrientation(float theta, float phi) const;
		void setUpVector(const vec3 &up, bool noMove = true) const;
		void setViewDirection(const vec3 &direction) const;
		//@}

		/*! @name Positioning tools */
	public:
		void lookAt(const vec3 &target) const;
		void showEntireScene() const;
		void fitSphere(const vec3 &center, float radius) const;
		void fitBoundingBox(const vec3 &min, const vec3 &max) const;

        /**
         * \brief Moves the Camera so that the rectangular screen region fits the screen.
         * \details The rectangular screen region is defined in pixel units, with origin in the upper left corner.
         * The Camera is translated (its orientation() is unchanged) so that \p rectangle is entirely visible.
         * Since the pixel coordinates only define a \e frustum in 3D, it's the intersection of this frustum with
         * a plane (orthogonal to the viewDirection() and passing through the sceneCenter()) that is used to define
         * the 3D rectangle that is eventually fitted.
         */
		void fitScreenRegion(int xmin, int ymin, int xmax, int ymax) const;
		void centerScene() const;

    public:
        /// Return the keyframe interpolator.
        KeyFrameInterpolator *keyframe_interpolator() const { return interpolationKfi_; }

        /// Perform keyframe interpolation to look at \p point.
        void interpolateToLookAt(const vec3& point);
        /// Perform keyframe interpolation to fit the scene bounding box.
        void interpolateToFitScene();
        /// Perform keyframe interpolation to \p frame, done in \p duration second.
        void interpolateTo(const Frame &frame, float duration);

	public:
		/**
		 * \brief Returns the Camera::Type of the Camera. Set by setType(). Mainly used by loadProjectionMatrix().
		 * \details A Camera::PERSPECTIVE Camera uses a classical projection mainly defined by its fieldOfView().
		 * With a Camera::ORTHOGRAPHIC type(), the fieldOfView() is meaningless and the width and height of the Camera
		 * frustum are inferred from the distance to the pivotPoint() using getOrthoWidthHeight(). Both types use
		 * zNear() and zFar() (to define their clipping planes) and aspectRatio() (for frustum shape).
		 */
		Type type() const { return type_; }

		/**
		 * \brief Returns the vertical field of view of the Camera (in radians).
		 * \details Value is set using setFieldOfView(). Default value is pi/4 radians. This value is meaningless if
		 * the Camera type() is Camera::ORTHOGRAPHIC.
		 * The field of view corresponds the one used in \c gluPerspective (see manual). It sets the Y (vertical)
		 * aperture of the Camera. The X (horizontal) angle is inferred from the window aspect ratio (see aspectRatio()
		 * and horizontalFieldOfView()).
		 * Use setFOVToFitScene() to adapt the fieldOfView() to a given scene.
		 */
		float fieldOfView() const { return fieldOfView_; }

		/**
		 * \brief Returns the horizontal field of view of the Camera (in radians).
		 * \details Value is set using setHorizontalFieldOfView() or setFieldOfView(). These values are always linked
		 * by:
		 *      \code
		 *      horizontalFieldOfView() = 2.0 * std::atan( std::tan(fieldOfView()/2.0) * aspectRatio() ).
		 *      \endcode
		 */
		float horizontalFieldOfView() const {
			return 2.0f * std::atan(std::tan(fieldOfView() / 2.0f) * aspectRatio());
		}

		/**
		 * \brief Returns the Camera aspect ratio defined by screenWidth() / screenHeight().
		 * \details When the Camera is attached to a Viewer, these values and hence the aspectRatio() are automatically
		 * fitted to the viewer's window aspect ratio using setScreenWidthAndHeight().
		 */
		float aspectRatio() const {
			return static_cast<float>(screenWidth_) / static_cast<float>(screenHeight_);
		}

		/**
		 * \brief Returns the width (in pixels) of the Camera screen.
		 * \details Set using setScreenWidthAndHeight(). This value is automatically fitted to the Viewer's window
		 * dimensions when the Camera is attached to a viewer.
		 */
		int screenWidth() const { return screenWidth_; }

		/**
		 * \brief Returns the height (in pixels) of the Camera screen.
		 * \details Set using setScreenWidthAndHeight(). This value is automatically fitted to the viewer's window
		 * dimensions when the Camera is attached to a Viewer.
		 */
		int screenHeight() const { return screenHeight_; }
		float pixelGLRatio(const vec3 &position) const;

		/**
		 * \brief Returns the coefficient which is used to set zNear() when the Camera is inside the sphere defined by
		 * sceneCenter() and zClippingCoefficient() * sceneRadius().
		 * \details In that case, the zNear() value is set to zNearCoefficient() * zClippingCoefficient() * sceneRadius().
		 * See the zNear() documentation for details.
		 * Default value is 0.001, which is appropriate for most applications. In case you need a high dynamic ZBuffer
		 * precision, you can increase this value (~0.1).
		 * A lower value will prevent clipping of very close objects at the expense of a worst Z precision.
		 * Only meaningful when Camera type is Camera::PERSPECTIVE.
		 */
		float zNearCoefficient() const { return zNearCoef_; }

		/**
		 * \brief Returns the coefficient used to position the near and far clipping planes.
		 * \details The near (resp. far) clipping plane is positioned at a distance equal to
		 * zClippingCoefficient() * sceneRadius() in front of (resp. behind) the sceneCenter(). This guarantees an
		 * optimal use of the z-buffer range and minimizes aliasing. See the zNear() and zFar() documentations.
		 * Default value is square root of 3.0 (so that a cube of size sceneRadius() is not clipped).
		 * However, since the sceneRadius() is used for other purposes (see showEntireScene(), flySpeed(), ...)
		 * and you may want to change this value to define more precisely the location of the clipping planes.
		 * See also zNearCoefficient().
		 * For a total control on clipping planes' positions, an other option is to overload the zNear() and zFar()
		 * methods.
		 * \attention When you visualize the Camera paths, set a larger value (suggested value is 5.0) so that the
		 * Camera paths are not clipped. Don't forget to restore the previous zClippingCoefficient() value back when
		 * you leave this mode. */
		float zClippingCoefficient() const { return zClippingCoef_; }

		virtual float zNear() const;
		virtual float zFar() const;
		virtual void getOrthoWidthHeight(float &halfWidth, float &halfHeight) const;
		void getFrustumPlanesCoefficients(float coef[6][4]) const;
        /// Return the 6 plane equations of the Camera frustum.
		void getFrustumPlanesCoefficients2(float coef[6][4]) const; // my version

	public:
		void setType(Type type);

		void setFieldOfView(float fov);

		/**
		 * \brief Sets the horizontalFieldOfView() of the Camera (in radians).
		 * \details horizontalFieldOfView() and fieldOfView() are linked by the aspectRatio().
		 * This method actually calls setFieldOfView(( 2.0 * std::atan (std::tan(hfov / 2.0) / aspectRatio()) )) so that a call
		 * to horizontalFieldOfView() returns the expected value. */
		void setHorizontalFieldOfView(float hfov) {
			setFieldOfView(2.0f * std::atan(std::tan(hfov / 2.0f) / aspectRatio()));
		}

		void setFOVToFitScene();

		/**
		 * \brief Defines the Camera aspectRatio().
		 * \details This value is actually inferred from the screenWidth() / screenHeight() ratio.
		 * You should use setScreenWidthAndHeight() instead.
		 * This method might however be convenient when the Camera is not associated with a Viewer. It actually sets
		 * the screenHeight() to 100 and the screenWidth() accordingly. See also setFOVToFitScene().
		 * \note If you absolutely need an aspectRatio() that does not correspond to your viewer's window dimensions,
		 * overload loadProjectionMatrix() or multiply the created OpenGL projection matrix by a scaled diagonal matrix
		 * in your viewer's draw() method. */
		void setAspectRatio(float aspect) {
            setScreenWidthAndHeight(int(100.0f * aspect), 100);
		}

		void setScreenWidthAndHeight(int width, int height);

		/**
		 * \brief Sets the zNearCoefficient() value.
		 */
		void setZNearCoefficient(float coef) {
			zNearCoef_ = coef;
			projectionMatrixIsUpToDate_ = false;
		}

        /**
         * \brief Sets the zClippingCoefficient() value.
         */
		void setZClippingCoefficient(float coef) {
			zClippingCoef_ = coef;
			projectionMatrixIsUpToDate_ = false;
		}
		//@}

		/*! @name Scene radius and center */
		//@{
	public:
		/**
		 * \brief Returns the radius of the scene observed by the Camera.
		 * \details You need to provide such an approximation of the scene dimensions so that the Camera can adapt
		 * its zNear() and zFar() values. See the sceneCenter() documentation.
		 * \see setSceneBoundingBox().
		 */
		float sceneRadius() const { return sceneRadius_; }

		/**
		 * \brief Returns the position of the scene center, defined in the world coordinate system.
		 * \details The scene observed by the Camera should be roughly centered on this position, and included in a
		 * sceneRadius() sphere. This approximate description of the scene permits a zNear() and zFar() clipping planes
		 * definition, and allows convenient positioning methods such as showEntireScene().
		 * Default value is (0,0,0) (world origin). Use setSceneCenter() to change it.
		 * \see setSceneBoundingBox().
		 */
		vec3 sceneCenter() const { return sceneCenter_; }
		float distanceToSceneCenter() const;

	public:
		void setSceneRadius(float radius);
		void setSceneCenter(const vec3 &center);
		void setSceneBoundingBox(const vec3 &min, const vec3 &max);
		//@}

	public:
		// The camera will move around the pivot point if set.
		void setPivotPoint(const vec3 &point);
		vec3 pivotPoint() const;

	public:
		/**
		 * \brief Returns the ManipulatedFrame attached to the Camera.
		 * \details This ManipulatedFrame defines its position() and orientation() and can translate mouse events into
		 * Camera displacement. Set using setFrame().
		 */
		ManipulatedCameraFrame *frame() const { return frame_; }
	public:
		void setFrame(ManipulatedCameraFrame *const mcf);
		//@}

	public:
		void computeProjectionMatrix();
		void computeModelViewMatrix();

		const mat4& projectionMatrix() const;
		const mat4& modelViewMatrix() const;
		mat4 modelViewProjectionMatrix() const;

		/*! @name World to Camera coordinate systems conversions */
		//@{
	public:
		vec3 cameraCoordinatesOf(const vec3 &src) const;
		vec3 worldCoordinatesOf(const vec3 &src) const;
		//@}

		/*! @name 2D screen to 3D world coordinate systems conversions */
		//@{
	public:
        vec3 projectedCoordinatesOf(const vec3 &src, const Frame *frame = nullptr) const;
        vec3 unprojectedCoordinatesOf(const vec3 &src, const Frame *frame = nullptr) const;
		void convertClickToLine(int x, int y, vec3 &orig, vec3 &dir) const;
		//@}

	private:
        void modified();

	private:
		// Frame
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

        // Key frame interpolation
        KeyFrameInterpolator* interpolationKfi_;

	public:
        /// A signal indicating the frame has been modified.
	    Signal<> frame_modified;
	};

}

#endif // EASY3D_RENDERER_CAMERA_H
