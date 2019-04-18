/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work, 
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan. 
*           Easy3D: a lightweight, easy-to-use, and efficient C++ 
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
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


#include <easy3d/core/types.h>
#include <easy3d/viewer/frame.h>


namespace easy3d {

    /* To use the camera, you need to do the following:
    * - Call setScreenWidthAndHeight() at both
    *     (1) creation or initialization (i.e., before the viewer appears) of
    *         the application;
    *     (2) change of the window size.
    * - Call frame()->action_start() on mouse down and
    *        frame()->action_end() on mouse up.
    * - Call frame()->action_rotate() on mouse move for rotation and
    *        frame()->action_translate() on mouse move for translation and
    *        frame()->action_zoom() on mouse move for zoom
    * To make the entire sence visible, call
    * 		  setSceneBoundingBox() and showEntireScene();
    *
    * To retrieve the model view projection matrix, call modelViewProjectionMatrix()
    */

	class Viewer;
	class Frame;
	class ManipulatedCameraFrame;

	/*! \brief A perspective or orthographic camera.
	  \class Camera camera.h Viewer/camera.h

	  A Camera defines some intrinsic parameters (fieldOfView(), position(),
	  viewDirection(), upVector()...) and useful positioning tools that ease its
	  placement (showEntireScene(), fitSphere(), lookAt()...). It exports its
	  associated OpenGL projection and modelview matrices and can interactively be
	  modified using the mouse.

	  <h3>Mouse manipulation</h3>

	  The position() and orientation() of the Camera are defined by a
	  ManipulatedFrame (retrieved using frame()). These methods are just
	  convenient wrappers to the equivalent Frame methods. This also means that the
	  Camera frame() can be attached to a Frame::referenceFrame() which enables
	  complex Camera setups.

	  Different displacements can be performed using the mouse. The list of possible
	  actions is defined by the Viewer::MouseAction enum. Use
	  Viewer::setMouseBinding() to attach a specific action to an arbitrary mouse
	  button-state key binding. These actions are detailed in the <a
	  href="../mouse.html">mouse page</a>.

	  The default button binding are: Viewer::ROTATE (left), Viewer::ZOOM
	  (middle) and Viewer::TRANSLATE (right). With this configuration, the Camera
	  \e observes a scene and rotates around its pivotPoint(). You can switch
	  between this mode and a fly mode using the Viewer::CAMERA_MODE (see
	  Viewer::toggleCameraMode()) keyboard shortcut (default is 'Space').

	  <h3>Other functionalities</h3>

	  The type() of the Camera can be Camera::ORTHOGRAPHIC or Camera::PERSPECTIVE
	  (see Type()). fieldOfView() is meaningless with Camera::ORTHOGRAPHIC.

	  The near and far planes of the Camera are fitted to the scene and determined
	  from Viewer::sceneRadius(), Viewer::sceneCenter() and
	  zClippingCoefficient() by the zNear() and zFar() methods. Reasonable values on
	  the scene extends hence have to be provided to the Viewer in order for the
	  Camera to correctly display the scene. High level positioning methods also use
	  this information (showEntireScene(), centerScene()...).

	  A Camera holds KeyFrameInterpolator that can be used to save Camera positions
	  and paths. You can interactively addKeyFrameToPath() to a given path using the
	  default \c Alt+F[1-12] shortcuts. Use playPath() to make the Camera follow the
	  path (default shortcut is F[1-12]). See the <a
	  href="../keyboard.html">keyboard page</a> for details on key customization.

	  Use cameraCoordinatesOf() and worldCoordinatesOf() to convert to and from the
	  Camera frame() coordinate system. projectedCoordinatesOf() and
	  unprojectedCoordinatesOf() will convert from screen to 3D coordinates.
	  convertClickToLine() is very useful for analytical object selection.

	  Stereo display is possible on machines with quad buffer capabilities (with
	  Camera::PERSPECTIVE type() only). Test the <a
	  href="../examples/stereoViewer.html">stereoViewer example</a> to check.

	  A Camera can also be used outside of a Viewer or even without OpenGL for
	  its coordinate system conversion capabilities. Note however that some of them
	  explicitly rely on the presence of a Z-buffer. \nosubgrouping */
	class Camera : public FrameObserver
	{
	public:
		Camera();
		virtual ~Camera();

		Camera(const Camera &camera);
		Camera &operator=(const Camera &camera);

		/*! Enumerates the two possible types of Camera.

		See type() and setType(). This type mainly defines different Camera projection
		matrix (see loadProjectionMatrix()). Many other methods (convertClickToLine(), 
		projectedCoordinatesOf(), pixelGLRatio()...) are affected by this Type. */
		enum Type { PERSPECTIVE, ORTHOGRAPHIC };

		/*! @name Position and orientation */
		//@{
	public:
		vec3 position() const;
		vec3 upVector() const;
		vec3 viewDirection() const;
		vec3 rightVector() const;
		quat orientation() const;

		void setFromModelViewMatrix(const mat4& mv);
        
        /*! Defines the position(), orientation() and fieldOfView() of the camera from calibrated
         *  camera instrinc and extrinsic parameters.
         *  \param fx and fy: the focal length
         *  \param cx and cy: the principal point
         *  \param skew: distortion
         *  \param rot: the rotation in angle-axis format, i.e., direction is the axis and length
         *              is the angle (in radian)
         *  \param t: the camera translation
         */
        void set_from_calibration(float fx, float fy, float skew, float cx, float cy,
                                  const vec3& rot, const vec3& t);
        
        /*! Does the same thing as set_from_calibration().
         *  \param proj The projection matrix computed as P = K * M * [R : T], where R is a
         *              3x3 matrix representing the camera rotation and T is a 3-vector
         *              describing the camera translation. Rotation first then translation!
         *  \attention: M is a 3 by 4 matrix [1, 0, 0, 0; 0, -1, 0, 0; 0, 0, -1, 0] converting
         *              from vision convention to OpenGL convention, i.e., inverting Y and Z axes.
         */
        void set_from_projection_matrix(const mat34& proj);
        
        // -----------------------------------------------------------------
        // Change the matrices directly and temporally.
        // NOTE: This is only useful if you want to temporally use different
        //       matrices for rendering (e.g., grabbing a large snapshot from
        //       the framebuffer. etc.). It actually doesn't change any
        //       parameters (position, orientation, fov, zFar, zNear, etc.) of
        //       the camera.
        // TODO: Modify the camera parameters accordingly (This is actually the
        //         reverse version of the two "compute" functions. See
        //         setFromModelViewMatrix(), and setFromProjectionMatrix()).
        void set_projection_matrix(const mat4& proj);
        void set_modelview_matrix(const mat4& mv);
        // -----------------------------------------------------------------

	public:
		void setPosition(const vec3 &pos);
		void setOrientation(const quat &q);
		void setOrientation(float theta, float phi);
		void setUpVector(const vec3 &up, bool noMove = true);
		void setViewDirection(const vec3 &direction);
		//@}

		/*! @name Positioning tools */
		//@{
	public:
		void lookAt(const vec3 &target);
		void showEntireScene();
		void fitSphere(const vec3 &center, float radius);
		void fitBoundingBox(const vec3 &min, const vec3 &max);
		void fitScreenRegion(int xmin, int ymin, int xmax, int ymax);
		void centerScene();

		/*! @name Frustum */
		//@{
	public:
		/*! Returns the Camera::Type of the Camera.

		Set by setType(). Mainly used by loadProjectionMatrix().

		A Camera::PERSPECTIVE Camera uses a classical projection mainly defined by its
		fieldOfView().

		With a Camera::ORTHOGRAPHIC type(), the fieldOfView() is meaningless and the
		width and height of the Camera frustum are inferred from the distance to the
		pivotPoint() using getOrthoWidthHeight().

		Both types use zNear() and zFar() (to define their clipping planes) and
		aspectRatio() (for frustum shape). */
		Type type() const { return type_; }

		/*! Returns the vertical field of view of the Camera (in radians).

		Value is set using setFieldOfView(). Default value is pi/4 radians. This value
		is meaningless if the Camera type() is Camera::ORTHOGRAPHIC.

		The field of view corresponds the one used in \c gluPerspective (see manual).
		It sets the Y (vertical) aperture of the Camera. The X (horizontal) angle is
		inferred from the window aspect ratio (see aspectRatio() and
		horizontalFieldOfView()).

		Use setFOVToFitScene() to adapt the fieldOfView() to a given scene. */
		float fieldOfView() const { return fieldOfView_; }

		/*! Returns the horizontal field of view of the Camera (in radians).

		Value is set using setHorizontalFieldOfView() or setFieldOfView(). These
		values are always linked by: \code horizontalFieldOfView() = 2.0 * atan (
		tan(fieldOfView()/2.0) * aspectRatio() ). \endcode */
		float horizontalFieldOfView() const {
			return 2.0f * atan(tan(fieldOfView() / 2.0f) * aspectRatio());
		}

		/*! Returns the Camera aspect ratio defined by screenWidth() / screenHeight().

		When the Camera is attached to a Viewer, these values and hence the
		aspectRatio() are automatically fitted to the viewer's window aspect ratio
		using setScreenWidthAndHeight(). */
		float aspectRatio() const {
			return screenWidth_ / static_cast<float>(screenHeight_);
		}
		/*! Returns the width (in pixels) of the Camera screen.

		Set using setScreenWidthAndHeight(). This value is automatically fitted to the
		Viewer's window dimensions when the Camera is attached to a Viewer. See
		also QOpenGLWidget::width() */
		int screenWidth() const { return screenWidth_; }
		/*! Returns the height (in pixels) of the Camera screen.

		Set using setScreenWidthAndHeight(). This value is automatically fitted to the
		Viewer's window dimensions when the Camera is attached to a Viewer. See
		also QOpenGLWidget::height() */
		int screenHeight() const { return screenHeight_; }
		float pixelGLRatio(const vec3 &position) const;

		/*! Returns the coefficient which is used to set zNear() when the Camera is
		inside the sphere defined by sceneCenter() and zClippingCoefficient() *
		sceneRadius().

		In that case, the zNear() value is set to zNearCoefficient() *
		zClippingCoefficient() * sceneRadius(). See the zNear() documentation for
		details.

		Default value is 0.005, which is appropriate for most applications. In case
		you need a high dynamic ZBuffer precision, you can increase this value (~0.1).
		A lower value will prevent clipping of very close objects at the expense of a
		worst Z precision.

		Only meaningful when Camera type is Camera::PERSPECTIVE. */
		float zNearCoefficient() const { return zNearCoef_; }
		/*! Returns the coefficient used to position the near and far clipping planes.

		The near (resp. far) clipping plane is positioned at a distance equal to
		zClippingCoefficient() * sceneRadius() in front of (resp. behind) the
		sceneCenter(). This garantees an optimal use of the z-buffer range and
		minimizes aliasing. See the zNear() and zFar() documentations.

		Default value is square root of 3.0 (so that a cube of size sceneRadius() is
		not clipped).

		However, since the sceneRadius() is used for other purposes (see
		showEntireScene(), flySpeed(),
		...) and you may want to change this value to define more precisely the
		location of the clipping planes. See also zNearCoefficient().

		For a total control on clipping planes' positions, an other option is to
		overload the zNear() and zFar() methods. See the <a
		href="../examples/standardCamera.html">standardCamera example</a>.

		\attention When Viewer::cameraPathAreEdited(), this value is set to 5.0 so
		that the Camera paths are not clipped. The previous zClippingCoefficient()
		value is restored back when you leave this mode. */
		float zClippingCoefficient() const { return zClippingCoef_; }

		virtual float zNear() const;
		virtual float zFar() const;
		virtual void getOrthoWidthHeight(float &halfWidth, float &halfHeight) const;
		void getFrustumPlanesCoefficients(float coef[6][4]) const;
		void getFrustumPlanesCoefficients2(float coef[6][4]) const; // my version

	public:
		void setType(Type type);

		void setFieldOfView(float fov);

		/*! Sets the horizontalFieldOfView() of the Camera (in radians).

		horizontalFieldOfView() and fieldOfView() are linked by the aspectRatio().
		This method actually calls setFieldOfView(( 2.0 * atan (tan(hfov / 2.0) /
		aspectRatio()) )) so that a call to horizontalFieldOfView() returns the
		expected value. */
		void setHorizontalFieldOfView(float hfov) {
			setFieldOfView(2.0f * atan(tan(hfov / 2.0f) / aspectRatio()));
		}

		void setFOVToFitScene();

		/*! Defines the Camera aspectRatio().

		This value is actually inferred from the screenWidth() / screenHeight() ratio.
		You should use setScreenWidthAndHeight() instead.

		This method might however be convenient when the Camera is not associated with
		a Viewer. It actually sets the screenHeight() to 100 and the screenWidth()
		accordingly. See also setFOVToFitScene().

		\note If you absolutely need an aspectRatio() that does not correspond to your
		viewer's window dimensions, overload loadProjectionMatrix() or multiply the
		created GL_PROJECTION matrix by a scaled diagonal matrix in your
		Viewer::draw() method. */
		void setAspectRatio(float aspect) {
            setScreenWidthAndHeight(int(100.0f * aspect), 100);
		}

		void setScreenWidthAndHeight(int width, int height);
		/*! Sets the zNearCoefficient() value. */
		void setZNearCoefficient(float coef) {
			zNearCoef_ = coef;
			projectionMatrixIsUpToDate_ = false;
		}
		/*! Sets the zClippingCoefficient() value. */
		void setZClippingCoefficient(float coef) {
			zClippingCoef_ = coef;
			projectionMatrixIsUpToDate_ = false;
		}
		//@}

		/*! @name Scene radius and center */
		//@{
	public:
		/*! Returns the radius of the scene observed by the Camera.

		You need to provide such an approximation of the scene dimensions so that the
		Camera can adapt its zNear() and zFar() values. See the sceneCenter()
		documentation.

		See also setSceneBoundingBox().

		Note that Viewer::sceneRadius() (resp. Viewer::setSceneRadius()) simply
		call this method (resp. setSceneRadius()) on its associated
		Viewer::camera(). */
		float sceneRadius() const { return sceneRadius_; }

		/*! Returns the position of the scene center, defined in the world coordinate
		system.

		The scene observed by the Camera should be roughly centered on this position,
		and included in a sceneRadius() sphere. This approximate description of the
		scene permits a zNear() and zFar() clipping planes definition, and allows
		convenient positioning methods such as showEntireScene().

		Default value is (0,0,0) (world origin). Use setSceneCenter() to change it.
		See also setSceneBoundingBox().

		Note that Viewer::sceneCenter() (resp. Viewer::setSceneCenter()) simply
		calls this method (resp. setSceneCenter()) on its associated
		Viewer::camera(). */
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
		/*! Returns the ManipulatedFrame attached to the Camera.

		This ManipulatedFrame defines its position() and orientation() and can
		translate mouse events into Camera displacement. Set using setFrame(). */
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
		void onFrameModified();

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

		friend class Viewer;
	};

}

#endif // EASY3D_CAMERA_H
