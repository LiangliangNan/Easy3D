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


#include <easy3d/viewer/camera.h>

#include <algorithm>

#include <easy3d/viewer/manipulated_camera_frame.h>
#include <easy3d/viewer/viewer.h>


namespace easy3d {


	/*! Default constructor.

	 sceneCenter() is set to (0,0,0) and sceneRadius() is set to 1.0. type() is
	 Camera::PERSPECTIVE, with a \c M_PI/4 fieldOfView().

	 See IODistance(), physicalDistanceToScreen(), physicalScreenWidth() and
	 focusDistance() documentations for default stereo parameter values. */
	Camera::Camera()
		: frame_(nullptr)
		, fieldOfView_(static_cast<float>(M_PI / 4.0))
		, modelViewMatrixIsUpToDate_(false)
		, projectionMatrixIsUpToDate_(false)
	{
		setFrame(new ManipulatedCameraFrame());

		// Requires fieldOfView() to define focusDistance()
		setSceneRadius(1.0f);

		// Initial value (only scaled after this)
		orthoCoef_ = tan(fieldOfView() / 2.0f);

		// Also defines the pivotPoint(), which changes orthoCoef_. Requires a
		// frame().
		setSceneCenter(vec3(0.0f, 0.0f, 0.0f));

		// Requires fieldOfView() when called with ORTHOGRAPHIC. Attention to
		// projectionMatrix_ below.
		setType(PERSPECTIVE);
		setViewDirection(vec3(0.0, 1.0, 0.0));

		// #CONNECTION# initFromDOMElement default values
		setZNearCoefficient(0.005f);
		setZClippingCoefficient(sqrt(3.0f));

		// Dummy values
		setScreenWidthAndHeight(600, 400);

        modelViewMatrix_.load_identity();
        projectionMatrix_.load_zero(); // computeProjectionMatrix() is lazy and assumes 0.0 everywhere

		// position and orient the camera
		showEntireScene();
	}

	/*! Virtual destructor.

	 The frame() is deleted, but the different keyFrameInterpolator() are \e not
	 deleted (in case they are shared). */
	Camera::~Camera() {
		delete frame_;
	}

	/*! Copy constructor. Performs a deep copy using operator=(). */
	Camera::Camera(const Camera &camera)
		: frame_(nullptr)
	{
		// Requires the interpolationKfi_
		setFrame(new ManipulatedCameraFrame(*camera.frame()));

        modelViewMatrix_.load_identity();
        projectionMatrix_.load_zero(); // computeProjectionMatrix() is lazy and assumes 0.0 everywhere

		(*this) = camera;
	}

	/*! Equal operator.

	 All the parameters of \p camera are copied. The frame() pointer is not
	 modified, but its Frame::position() and Frame::orientation() are set to those
	 of \p camera.

	 \attention The Camera screenWidth() and screenHeight() are set to those of \p
	 camera. If your Camera is associated with a Viewer, you should update these
	 value after the call to this method: \code
	 *(camera()) = otherCamera;
	 camera()->setScreenWidthAndHeight(width(), height());
	 \endcode
	 The same applies to sceneCenter() and sceneRadius(), if needed. */
	Camera &Camera::operator=(const Camera &camera) {
		setScreenWidthAndHeight(camera.screenWidth(), camera.screenHeight());
		setFieldOfView(camera.fieldOfView());
		setSceneRadius(camera.sceneRadius());
		setSceneCenter(camera.sceneCenter());
		setZNearCoefficient(camera.zNearCoefficient());
		setZClippingCoefficient(camera.zClippingCoefficient());
		setType(camera.type());

		orthoCoef_ = camera.orthoCoef_;
		projectionMatrixIsUpToDate_ = false;

		// frame_ and interpolationKfi_ pointers are not shared.
		frame_->setReferenceFrame(nullptr);
		frame_->setPosition(camera.position());
		frame_->setOrientation(camera.orientation());

		computeProjectionMatrix();
		computeModelViewMatrix();

		return *this;
	}

	/*! Sets Camera screenWidth() and screenHeight() (expressed in pixels).

	You should not call this method when the Camera is associated with a Viewer,
	since the latter automatically updates these values when it is resized (hence
	overwritting your values).

	Non-positive dimension are silently replaced by a 1 pixel value to ensure
	frustrum coherence.

	If your Camera is used without a Viewer (offscreen rendering, shadow maps),
	use setAspectRatio() instead to define the projection matrix. */
	void Camera::setScreenWidthAndHeight(int width, int height) {
		// Prevent negative and zero dimensions that would cause divisions by zero.
		screenWidth_ = width > 0 ? width : 1;
		screenHeight_ = height > 0 ? height : 1;
		projectionMatrixIsUpToDate_ = false;
	}

	/*! Returns the near clipping plane distance used by the Camera projection
	 matrix.

	 The clipping planes' positions depend on the sceneRadius() and sceneCenter()
	 rather than being fixed small-enough and large-enough values. A good scene
	 dimension approximation will hence result in an optimal precision of the
	 z-buffer.

	 The near clipping plane is positioned at a distance equal to
	 zClippingCoefficient() * sceneRadius() in front of the sceneCenter(): \code
	 zNear = distanceToSceneCenter() - zClippingCoefficient()*sceneRadius();
	 \endcode

	 In order to prevent negative or too small zNear() values (which would degrade
	 the z precision), zNearCoefficient() is used when the Camera is inside the
	 sceneRadius() sphere: \code const double zMin = zNearCoefficient() *
	 zClippingCoefficient() * sceneRadius(); if (zNear < zMin) zNear = zMin;
	 // With an ORTHOGRAPHIC type, the value is simply clamped to 0.0
	 \endcode

	 See also the zFar(), zClippingCoefficient() and zNearCoefficient()
	 documentations.

	 If you need a completely different zNear computation, overload the zNear() and
	 zFar() methods in a new class that publicly inherits from Camera and use
	 Viewer::setCamera(): \code class myCamera :: public qglviewer::Camera
	 {
	   virtual double Camera::zNear() const { return 0.001; };
	   virtual double Camera::zFar() const { return 100.0; };
	 }
	 \endcode

	 See the <a href="../examples/standardCamera.html">standardCamera example</a>
	 for an application.

	 \attention The value is always positive although the clipping plane is
	 positioned at a negative z value in the Camera coordinate system. This follows
	 the \c gluPerspective standard. */
	float Camera::zNear() const {
		const float zNearScene = zClippingCoefficient() * sceneRadius();
		float z = distanceToSceneCenter() - zNearScene;

		// Prevents negative or null zNear values.
		const float zMin = zNearCoefficient() * zNearScene;
		if (z < zMin)
			switch (type()) {
			case Camera::PERSPECTIVE:
				z = zMin;
				break;
			case Camera::ORTHOGRAPHIC:
				z = 0.0;
				break;
			}
		return z;
	}

	/*! Returns the far clipping plane distance used by the Camera projection
	matrix.

	The far clipping plane is positioned at a distance equal to
	zClippingCoefficient() * sceneRadius() behind the sceneCenter(): \code zFar =
	distanceToSceneCenter() + zClippingCoefficient()*sceneRadius(); \endcode

	See the zNear() documentation for details. */
	float Camera::zFar() const {
		return distanceToSceneCenter() + zClippingCoefficient() * sceneRadius();
	}

	/*! Sets the vertical fieldOfView() of the Camera (in radians).

	Note that focusDistance() is set to sceneRadius() / tan(fieldOfView()/2) by this
	method. */
	void Camera::setFieldOfView(float fov) {
		fieldOfView_ = fov;
		projectionMatrixIsUpToDate_ = false;
	}

	/*! Defines the Camera type().

	Changing the camera Type alters the viewport and the objects' sizes can be
	changed. This method garantees that the two frustum match in a plane normal to
	viewDirection(), passing through the pivotPoint().

	Prefix the type with \c Camera if needed, as in:
	\code
	camera()->setType(Camera::ORTHOGRAPHIC);
	// or even qglviewer::Camera::ORTHOGRAPHIC if you do not use namespace
	\endcode */
	void Camera::setType(Type type) {
		// make ORTHOGRAPHIC frustum fit PERSPECTIVE (at least in plane normal to
		// viewDirection(), passing through RAP). Done only when CHANGING type since
		// orthoCoef_ may have been changed with a setPivotPoint() in the meantime.
		if ((type == Camera::ORTHOGRAPHIC) && (type_ == Camera::PERSPECTIVE))
			orthoCoef_ = tan(fieldOfView() / 2.0f);
		type_ = type;
		projectionMatrixIsUpToDate_ = false;
	}

	/*! Sets the Camera frame().

	If you want to move the Camera, use setPosition() and setOrientation() or one of
	the Camera positioning methods (lookAt(), fitSphere(), showEntireScene()...)
	instead.

	If you want to save the Camera position(), there's no need to call this method
	either. Use addKeyFrameToPath() and playPath() instead.

	This method is actually mainly useful if you derive the ManipulatedFrame
	class and want to use an instance of your new class to move the Camera.

	A \c NULL \p mcf pointer will silently be ignored. The calling method is
	responsible for deleting the previous frame() pointer if needed in order to
	prevent memory leaks. */
	void Camera::setFrame(ManipulatedCameraFrame *const mcf) {
		if (!mcf)
			return;

		frame_ = mcf;
		frame_->addObserver(this);

		onFrameModified();
	}

	/*! Returns the distance from the Camera center to sceneCenter(), projected
	  along the Camera Z axis. Used by zNear() and zFar() to optimize the Z range.
	*/
	float Camera::distanceToSceneCenter() const {
		return std::fabs((frame()->coordinatesOf(sceneCenter())).z);
	}

	/*! Returns the \p halfWidth and \p halfHeight of the Camera orthographic
	 frustum.

	 These values are only valid and used when the Camera is of type()
	 Camera::ORTHOGRAPHIC. They are expressed in OpenGL units and are used by
	 loadProjectionMatrix() to define the projection matrix using: \code glOrtho(
	 -halfWidth, halfWidth, -halfHeight, halfHeight, zNear(), zFar() ) \endcode

	 These values are proportional to the Camera (z projected) distance to the
	 pivotPoint(). When zooming on the object, the Camera is translated forward \e
	 and its frustum is narrowed, making the object appear bigger on screen, as
	 intuitively expected.

	 Overload this method to change this behavior if desired, as is done in the
	 <a href="../examples/standardCamera.html">standardCamera example</a>. */
	void Camera::getOrthoWidthHeight(float &halfWidth, float &halfHeight) const {
		const float dist = orthoCoef_ * std::fabs(cameraCoordinatesOf(pivotPoint()).z);
		//#CONNECTION# fitScreenRegion
		halfWidth = dist * ((aspectRatio() < 1.0f) ? 1.0f : aspectRatio());
		halfHeight = dist * ((aspectRatio() < 1.0f) ? 1.0f / aspectRatio() : 1.0f);
	}

	/*! Computes the projection matrix associated with the Camera.

	 If type() is Camera::PERSPECTIVE, defines a \c GL_PROJECTION matrix similar to
	 what would \c gluPerspective() do using the fieldOfView(), window
	 aspectRatio(), zNear() and zFar() parameters.

	 If type() is Camera::ORTHOGRAPHIC, the projection matrix is as what \c
	 glOrtho() would do. Frustum's width and height are set using
	 getOrthoWidthHeight().

	 Both types use zNear() and zFar() to place clipping planes. These values are
	 determined from sceneRadius() and sceneCenter() so that they best fit the scene
	 size.

	 Use getProjectionMatrix() to retrieve this matrix. Overload
	 loadProjectionMatrix() if you want your Camera to use an exotic projection
	 matrix.

	 \note You must call this method if your Camera is not associated with a
	 Viewer and is used for off screen computations (using
	 (un)projectedCoordinatesOf() for instance). loadProjectionMatrix() does it
	 otherwise. */
	void Camera::computeProjectionMatrix()
	{
		if (projectionMatrixIsUpToDate_)
			return;

		const float ZNear = zNear();
		const float ZFar = zFar();

		switch (type())
		{
		case Camera::PERSPECTIVE:
		{
			// #CONNECTION# all non null coefficients were set to 0.0 in constructor.
			const float f = 1.0f / tan(fieldOfView() / 2.0f);
			projectionMatrix_[0] = f / aspectRatio();
			projectionMatrix_[5] = f;
			projectionMatrix_[10] = (ZNear + ZFar) / (ZNear - ZFar);
			projectionMatrix_[11] = -1.0f;
			projectionMatrix_[14] = 2.0f * ZNear * ZFar / (ZNear - ZFar);
			projectionMatrix_[15] = 0.0f;
			// same as gluPerspective( 180.0*fieldOfView()/M_PI, aspectRatio(), zNear(), zFar() );
			break;
		}
		case Camera::ORTHOGRAPHIC:
		{
			float w, h;
			getOrthoWidthHeight(w, h);
			projectionMatrix_[0] = 1.0f / w;
			projectionMatrix_[5] = 1.0f / h;
			projectionMatrix_[10] = -2.0f / (ZFar - ZNear);
			projectionMatrix_[11] = 0.0f;
			projectionMatrix_[14] = -(ZFar + ZNear) / (ZFar - ZNear);
			projectionMatrix_[15] = 1.0f;
			// same as glOrtho( -w, w, -h, h, zNear(), zFar() );
			break;
		}
		}

		projectionMatrixIsUpToDate_ = true;
	}

	/*! Computes the modelView matrix associated with the Camera's position() and
	 orientation().

	 This matrix converts from the world coordinates system to the Camera
	 coordinates system, so that coordinates can then be projected on screen using
	 the projection matrix (see computeProjectionMatrix()).

	 Use getModelViewMatrix() to retrieve this matrix.

	 \note You must call this method if your Camera is not associated with a
	 Viewer and is used for offscreen computations (using
	 (un)projectedCoordinatesOf() for instance). loadModelViewMatrix() does it
	 otherwise. */
	void Camera::computeModelViewMatrix()
	{
		if (modelViewMatrixIsUpToDate_)
			return;

		const quat q = orientation();

		const float q00 = 2.0f * q[0] * q[0];
		const float q11 = 2.0f * q[1] * q[1];
		const float q22 = 2.0f * q[2] * q[2];

		const float q01 = 2.0f * q[0] * q[1];
		const float q02 = 2.0f * q[0] * q[2];
		const float q03 = 2.0f * q[0] * q[3];

		const float q12 = 2.0f * q[1] * q[2];
		const float q13 = 2.0f * q[1] * q[3];

		const float q23 = 2.0f * q[2] * q[3];

		modelViewMatrix_[0] = 1.0f - q11 - q22;
		modelViewMatrix_[1] = q01 - q23;
		modelViewMatrix_[2] = q02 + q13;
		modelViewMatrix_[3] = 0.0f;

		modelViewMatrix_[4] = q01 + q23;
		modelViewMatrix_[5] = 1.0f - q22 - q00;
		modelViewMatrix_[6] = q12 - q03;
		modelViewMatrix_[7] = 0.0f;

		modelViewMatrix_[8] = q02 - q13;
		modelViewMatrix_[9] = q12 + q03;
		modelViewMatrix_[10] = 1.0f - q11 - q00;
		modelViewMatrix_[11] = 0.0;

		const vec3 t = q.inverse_rotate(position());

		modelViewMatrix_[12] = -t.x;
		modelViewMatrix_[13] = -t.y;
		modelViewMatrix_[14] = -t.z;
		modelViewMatrix_[15] = 1.0;

		modelViewMatrixIsUpToDate_ = true;
	}

	/*! Fills \p m with the Camera projection matrix values.

	 Based on computeProjectionMatrix() to make sure the Camera projection matrix is
	 up to date.

	 This matrix only reflects the Camera's internal parameters and it may differ
	 from the \c GL_PROJECTION matrix retrieved using \c
	 glGetDoublev(GL_PROJECTION_MATRIX, m). It actually represents the state of the
	 \c GL_PROJECTION after Viewer::preDraw(), at the beginning of
	 Viewer::draw(). If you modified the \c GL_PROJECTION matrix (for instance
	 using Viewer::startScreenCoordinatesSystem()), the two results differ.

	 The result is an OpenGL 4x4 matrix, which is given in \e column-major order
	 (see \c glMultMatrix man page for details).

	 See also getModelViewMatrix() and setFromProjectionMatrix(). */
	const mat4& Camera::projectionMatrix() const
	{
		// May not be needed, but easier like this.
		const_cast<Camera*>(this)->computeProjectionMatrix();
		return projectionMatrix_;
	}

	/*! Fills \p m with the Camera modelView matrix values.

	 First calls computeModelViewMatrix() to define the Camera modelView matrix.

	 Note that this matrix may \e not be the one you would get from a \c
	 glGetDoublev(GL_MODELVIEW_MATRIX, m). It actually represents the state of the
	 \c GL_MODELVIEW after Viewer::preDraw(), at the \e beginning of
	 Viewer::draw(). It converts from the world to the Camera coordinate system.
	 As soon as you modify the \c GL_MODELVIEW in your Viewer::draw() method
	 (using glTranslate, glRotate... or similar methods), the two matrices differ.

	 The result is an OpenGL 4x4 matrix, which is given in \e column-major order
	 (see \c glMultMatrix man page for details).

	 See also getProjectionMatrix() and setFromModelViewMatrix(). */
	const mat4& Camera::modelViewMatrix() const
	{
		// May not be needed, but easier like this.
		const_cast<Camera*>(this)->computeModelViewMatrix();
		return modelViewMatrix_;
	}


	/*! Fills \p m with the product of the ModelView and Projection matrices.

	  Calls getModelViewMatrix() and getProjectionMatrix() and then fills \p m with
	  the product of these two matrices. */
	mat4 Camera::modelViewProjectionMatrix() const {
		return projectionMatrix() * modelViewMatrix();
	}

	// -----------------------------------------------------------------
	// Change the matrices directly. This is useful if you want to temporally
	// change the view, e.g., grabbing a large snapshot from the framebuffer...
	void Camera::set_projection_matrix(const mat4& proj) {
		projectionMatrix_ = proj;
	}


	void Camera::set_modelview_matrix(const mat4& mv) {
		modelViewMatrix_ = mv;
	}

	/*! Sets the sceneRadius() value. Negative values are ignored.

	\attention This methods also sets focusDistance() to sceneRadius() /
	tan(fieldOfView()/2) and flySpeed() to 1% of sceneRadius(). */
	void Camera::setSceneRadius(float radius) {
		if (radius <= 0.0f) {
			std::cerr << "Scene radius must be positive (value is: " << radius << ")" << std::endl;
			return;
		}

		sceneRadius_ = radius;
		projectionMatrixIsUpToDate_ = false;
	}

	/*! Similar to setSceneRadius() and setSceneCenter(), but the scene limits are
	  defined by a (world axis aligned) bounding box. */
	void Camera::setSceneBoundingBox(const vec3 &min, const vec3 &max) {
		setSceneCenter((min + max) / 2.0f);
		setSceneRadius(0.5f * (max - min).norm());
	}

	/*! Sets the sceneCenter().

	 \attention This method also sets the pivotPoint() to sceneCenter(). */
	void Camera::setSceneCenter(const vec3 &center) {
		sceneCenter_ = center;
		setPivotPoint(sceneCenter());
		projectionMatrixIsUpToDate_ = false;
	}


	/*! Changes the pivotPoint() to \p point (defined in the world coordinate
	 * system). */
	void Camera::setPivotPoint(const vec3 &point) {
		const float prevDist = std::fabs(cameraCoordinatesOf(pivotPoint()).z);

		// If frame's RAP is set directly, projectionMatrixIsUpToDate_ should also be
		// set to false to ensure proper recomputation of the ORTHO projection matrix.
		frame()->setPivotPoint(point);

		// orthoCoef_ is used to compensate for changes of the pivotPoint, so that the
		// image does not change when the pivotPoint is changed in ORTHOGRAPHIC mode.
		const float newDist = std::fabs(cameraCoordinatesOf(pivotPoint()).z);
		// Prevents division by zero when rap is set to camera position
		if ((prevDist > 1E-9) && (newDist > 1E-9))
			orthoCoef_ *= prevDist / newDist;
		projectionMatrixIsUpToDate_ = false;
	}

	/*! Returns the ratio between pixel and OpenGL units at \p position.

	 A line of \c n * pixelGLRatio() OpenGL units, located at \p position in the
	 world coordinates system, will be projected with a length of \c n pixels on
	 screen.

	 Use this method to scale objects so that they have a constant pixel size on
	 screen. The following code will draw a 20 pixel line, starting at sceneCenter()
	 and always directed along the screen vertical direction: \code
	 glBegin(GL_LINES);
	 glVertex3fv(sceneCenter());
	 glVertex3fv(sceneCenter() + 20 * pixelGLRatio(sceneCenter()) *
	 camera()->upVector()); glEnd(); \endcode */
	float Camera::pixelGLRatio(const vec3 &position) const {
		switch (type()) {
		case Camera::PERSPECTIVE:
			return 2.0f * std::fabs((frame()->coordinatesOf(position)).z) *
				tan(fieldOfView() / 2.0f) / screenHeight();
		case Camera::ORTHOGRAPHIC: {
			float w, h;
			getOrthoWidthHeight(w, h);
			return 2.0f * h / screenHeight();
		}
		}
		// Bad compilers complain
		return 1.0f;
	}

	/*! Changes the Camera fieldOfView() so that the entire scene (defined by
	 Viewer::sceneCenter() and Viewer::sceneRadius()) is visible from the
	 Camera position().

	 The position() and orientation() of the Camera are not modified and you first
	 have to orientate the Camera in order to actually see the scene (see lookAt(),
	 showEntireScene() or fitSphere()).

	 This method is especially useful for \e shadow \e maps computation. Use the
	 Camera positioning tools (setPosition(), lookAt()) to position a Camera at the
	 light position. Then use this method to define the fieldOfView() so that the
	 shadow map resolution is optimally used: \code
	 // The light camera needs size hints in order to optimize its fieldOfView
	 lightCamera->setSceneRadius(sceneRadius());
	 lightCamera->setSceneCenter(sceneCenter());

	 // Place the light camera.
	 lightCamera->setPosition(lightFrame->position());
	 lightCamera->lookAt(sceneCenter());
	 lightCamera->setFOVToFitScene();
	 \endcode

	 See the (soon available) shadowMap contribution example for a practical
	 implementation.

	 \attention The fieldOfView() is clamped to M_PI/2.0. This happens when the
	 Camera is at a distance lower than sqrt(2.0) * sceneRadius() from the
	 sceneCenter(). It optimizes the shadow map resolution, although it may miss
	 some parts of the scene. */
	void Camera::setFOVToFitScene() {
		if (distanceToSceneCenter() > sqrt(2.0f) * sceneRadius())
			setFieldOfView(2.0f * asin(sceneRadius() / distanceToSceneCenter()));
		else
			setFieldOfView(static_cast<float>(M_PI) / 2.0f);
	}


	/*! Moves the Camera so that the entire scene is visible.

	 Simply calls fitSphere() on a sphere defined by sceneCenter() and
	 sceneRadius().

	 You will typically use this method in Viewer::init() after you defined a new
	 sceneRadius(). */
	void Camera::showEntireScene() { fitSphere(sceneCenter(), sceneRadius()); }

	/*! Moves the Camera so that its sceneCenter() is projected on the center of the
	 window. The orientation() and fieldOfView() are unchanged.

	 Simply projects the current position on a line passing through sceneCenter().
	 See also showEntireScene().*/
	void Camera::centerScene() {
		frame()->projectOnLine(sceneCenter(), viewDirection());
	}

	/*! Sets the Camera orientation(), so that it looks at point \p target (defined
	 in the world coordinate system).

	 The Camera position() is not modified. Simply setViewDirection().

	 See also setUpVector(), setOrientation(), showEntireScene(), fitSphere() and
	 fitBoundingBox(). */
	void Camera::lookAt(const vec3 &target) {
		setViewDirection(target - position());
	}

	/*! Moves the Camera so that the sphere defined by (\p center, \p radius) is
	 visible and fits in the frustum.

	 The Camera is simply translated to center the sphere in the screen and make it
	 fit the frustum. Its orientation() and its fieldOfView() are unchanged.

	 You should therefore orientate the Camera before you call this method. See
	 lookAt(), setOrientation() and setUpVector(). */
	void Camera::fitSphere(const vec3 &center, float radius) {
		float distance = 0.0f;
		switch (type()) {
		case Camera::PERSPECTIVE: {
			const float yview = radius / sin(fieldOfView() / 2.0f);
			const float xview = radius / sin(horizontalFieldOfView() / 2.0f);
			distance = std::max(xview, yview);
			break;
		}
		case Camera::ORTHOGRAPHIC: {
			distance = dot((center - pivotPoint()), viewDirection()) + (radius / orthoCoef_);
			break;
		}
		}
		vec3 newPos(center - distance * viewDirection());
		frame()->setPositionWithConstraint(newPos);
	}

	/*! Moves the Camera so that the (world axis aligned) bounding box (\p min, \p
	  max) is entirely visible, using fitSphere(). */
	void Camera::fitBoundingBox(const vec3 &min, const vec3 &max) {
		float diameter = std::max(std::fabs(max[1] - min[1]), std::fabs(max[0] - min[0]));
		diameter = std::max(std::fabs(max[2] - min[2]), diameter);
		fitSphere(0.5f * (min + max), 0.5f * diameter);
	}

	/*! Moves the Camera so that the rectangular screen region defined by \p
	  rectangle (pixel units, with origin in the upper left corner) fits the screen.

	  The Camera is translated (its orientation() is unchanged) so that \p rectangle
	  is entirely visible. Since the pixel coordinates only define a \e frustum in
	  3D, it's the intersection of this frustum with a plane (orthogonal to the
	  viewDirection() and passing through the sceneCenter()) that is used to define
	  the 3D rectangle that is eventually fitted. */
	void Camera::fitScreenRegion(int xmin, int ymin, int xmax, int ymax) {
		const vec3& vd = viewDirection();
		const float distToPlane = distanceToSceneCenter();
		int cx = static_cast<int>((xmin + xmax) * 0.5);
		int cy = static_cast<int>((ymin + ymax) * 0.5);

		vec3 orig, dir;
		convertClickToLine(cx, cy, orig, dir);
		const vec3& newCenter = orig + distToPlane / dot(dir, vd) * dir;

		convertClickToLine(xmin, cy, orig, dir);
		const vec3& pointX = orig + distToPlane / dot(dir, vd) * dir;

		convertClickToLine(cx, ymin, orig, dir);
		const vec3& pointY = orig + distToPlane / dot(dir, vd) * dir;

		float distance = 0.0f;
		switch (type()) {
		case Camera::PERSPECTIVE: {
			const float distX =
				(pointX - newCenter).norm() / sin(horizontalFieldOfView() / 2.0f);
			const float distY = (pointY - newCenter).norm() / sin(fieldOfView() / 2.0f);
			distance = std::max(distX, distY);
			break;
		}
		case Camera::ORTHOGRAPHIC: {
			const float dist = dot((newCenter - pivotPoint()), vd);
			//#CONNECTION# getOrthoWidthHeight
			const float distX = (pointX - newCenter).norm() / orthoCoef_ /
				((aspectRatio() < 1.0f) ? 1.0f : aspectRatio());
			const float distY = (pointY - newCenter).norm() / orthoCoef_ /
				((aspectRatio() < 1.0f) ? 1.0f / aspectRatio() : 1.0f);
			distance = dist + std::max(distX, distY);
			break;
		}
		}

		vec3 newPos(newCenter - distance * vd);
		frame()->setPositionWithConstraint(newPos);
	}

	/*! Rotates the Camera so that its upVector() becomes \p up (defined in the
	 world coordinate system).

	 The Camera is rotated around an axis orthogonal to \p up and to the current
	 upVector() direction. Use this method in order to define the Camera horizontal
	 plane.

	 When \p noMove is set to \c false, the orientation modification is compensated
	 by a translation, so that the pivotPoint() stays projected at the same position
	 on screen. This is especially useful when the Camera is used as an observer of
	 the scene (default mouse binding).

	 When \p noMove is \c true (default), the Camera position() is left unchanged,
	 which is an intuitive behavior when the Camera is in a walkthrough fly mode
	 (see the Viewer::MOVE_FORWARD and Viewer::MOVE_BACKWARD
	 Viewer::MouseAction).

	 The frame()'s ManipulatedFrame::sceneUpVector() is set accordingly.

	 See also setViewDirection(), lookAt() and setOrientation(). */
	void Camera::setUpVector(const vec3 &up, bool noMove) {
		quat q(vec3(0.0, 1.0, 0.0), frame()->transformOf(up));

		if (!noMove)
			frame()->setPosition(pivotPoint() -
			(frame()->orientation() * q)
				.rotate(frame()->coordinatesOf(pivotPoint())));

		frame()->rotate(q);
	}

	/*! Sets the orientation() of the Camera using polar coordinates.

	 \p theta rotates the Camera around its Y axis, and \e then \p phi rotates it
	 around its X axis. The polar coordinates are defined in the world coordinates
	 system: \p theta = \p phi = 0 means that the Camera is directed towards the
	 world Z axis. Both angles are expressed in radians.

	 See also setUpVector(). The position() of the Camera is unchanged, you may want
	 to call showEntireScene() after this method to move the Camera.

	 This method can be useful to create Quicktime VR panoramic sequences, see the
	 Viewer::saveSnapshot() documentation for details. */
	void Camera::setOrientation(float theta, float phi) {
		vec3 axis(0.0f, 1.0f, 0.0f);
		const quat rot1(axis, theta);
		axis = vec3(-cos(theta), 0.0f, sin(theta));
		const quat rot2(axis, phi);
		setOrientation(rot1 * rot2);
	}

	/*! Sets the Camera orientation(), defined in the world coordinate system. */
	void Camera::setOrientation(const quat &q) {
		frame()->setOrientation(q);
	}

	/*! Rotates the Camera so that its viewDirection() is \p direction (defined in
	 the world coordinate system).

	 The Camera position() is not modified. The Camera is rotated so that the
	 horizon (defined by its upVector()) is preserved. See also lookAt() and
	 setUpVector(). */
	void Camera::setViewDirection(const vec3 &direction) {
		if (direction.length2() < 1E-10)
			return;

		vec3 xAxis = cross(direction, upVector());
		if (xAxis.length2() < 1E-10)
		{
			// target is aligned with upVector, this means a rotation around X axis
			// X axis is then unchanged, let's keep it !
			xAxis = frame()->inverseTransformOf(vec3(1.0, 0.0, 0.0));
		}

		quat q;
		q.set_from_rotated_basis(xAxis, cross(xAxis, direction), -direction);
		frame()->setOrientationWithConstraint(q);
	}

	/*! Returns the Camera position (the eye), defined in the world coordinate
	system.

	Use setPosition() to set the Camera position. Other convenient methods are
	showEntireScene() or fitSphere(). Actually returns \c frame()->position().

	This position corresponds to the projection center of a Camera::PERSPECTIVE
	Camera. It is not located in the image plane, which is at a zNear() distance
	ahead. */
	vec3 Camera::position() const { return frame()->position(); }

	/*! Returns the normalized up vector of the Camera, defined in the world
	coordinate system.

	Set using setUpVector() or setOrientation(). It is orthogonal to viewDirection()
	and to rightVector().

	It corresponds to the Y axis of the associated frame() (actually returns
	frame()->inverseTransformOf(vec3(0.0, 1.0, 0.0)) ). */
	vec3 Camera::upVector() const {
		return frame()->inverseTransformOf(vec3(0.0, 1.0, 0.0));
	}
	/*! Returns the normalized view direction of the Camera, defined in the world
	coordinate system.

	Change this value using setViewDirection(), lookAt() or setOrientation(). It is
	orthogonal to upVector() and to rightVector().

	This corresponds to the negative Z axis of the frame() (
	frame()->inverseTransformOf(vec3(0.0, 0.0, -1.0)) ). */
	vec3 Camera::viewDirection() const {
		return frame()->inverseTransformOf(vec3(0.0, 0.0, -1.0));
	}

	/*! Returns the normalized right vector of the Camera, defined in the world
	coordinate system.

	This vector lies in the Camera horizontal plane, directed along the X axis
	(orthogonal to upVector() and to viewDirection()). Set using setUpVector(),
	lookAt() or setOrientation().

	Simply returns frame()->inverseTransformOf(vec3(1.0, 0.0, 0.0)). */
	vec3 Camera::rightVector() const {
		return frame()->inverseTransformOf(vec3(1.0, 0.0, 0.0));
	}

	/*! Returns the Camera orientation, defined in the world coordinate system.

	Actually returns \c frame()->orientation(). Use setOrientation(), setUpVector()
	or lookAt() to set the Camera orientation. */
	quat Camera::orientation() const { return frame()->orientation(); }

	/*! Sets the Camera position() (the eye), defined in the world coordinate
	 * system. */
	void Camera::setPosition(const vec3 &pos) { frame()->setPosition(pos); }

	/*! Returns the Camera frame coordinates of a point \p src defined in world
	coordinates.

	worldCoordinatesOf() performs the inverse transformation.

	Note that the point coordinates are simply converted in a different coordinate
	system. They are not projected on screen. Use projectedCoordinatesOf() for that.
	*/
	vec3 Camera::cameraCoordinatesOf(const vec3 &src) const {
		return frame()->coordinatesOf(src);
	}

	/*! Returns the world coordinates of the point whose position \p src is defined
	in the Camera coordinate system.

	cameraCoordinatesOf() performs the inverse transformation. */
	vec3 Camera::worldCoordinatesOf(const vec3 &src) const {
		return frame()->inverseCoordinatesOf(src);
	}

	/*! The point the Camera pivots around with the Viewer::ROTATE mouse binding.
	Defined in world coordinate system.

	Default value is the sceneCenter().

	\attention setSceneCenter() changes this value. */
	vec3 Camera::pivotPoint() const { return frame()->pivotPoint(); }

	/*! Sets the Camera's position() and orientation() from an OpenGL ModelView
	matrix.

	This enables a Camera initialisation from an other OpenGL application. \p
	modelView is a 16 double vector representing a valid OpenGL ModelView matrix,
	such as one can get using: \code double mvm[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, mvm);
	myCamera->setFromModelViewMatrix(mvm);
	\endcode

	After this method has been called, getModelViewMatrix() returns a matrix
	equivalent to \p modelView.

	Only the orientation() and position() of the Camera are modified.

	\note If you defined your matrix as \c double \c mvm[4][4], pass \c
	&(mvm[0][0]) as a parameter. */
	void Camera::setFromModelViewMatrix(const mat4& mv)
	{
		// Get upper left (rotation) matrix
		mat3 rot(mv);

		// Transform upperLeft into the associated float
		quat q;
		q.set_from_rotation_matrix(rot);

		setOrientation(q);
		setPosition(-q.rotate(vec3(mv(0, 3), mv(1, 3), mv(2, 3))));
	}


    void Camera::set_from_calibration(float fx, float fy, float skew, float cx, float cy,
                                      const vec3 &rot, const vec3 &t)
    {
        /**-------------------------------------------------------------------
         * It took me quite a while to figure out this.
         * The OpengGL projection and modelview matrices can be computed as follows.
         * Particle Filtering with Rendered Models: A Two Pass Approach to Multi-object 3D Tracking with the GPU)
         * http://cvrr.ucsd.edu/publications/2008/MurphyChutorian_Trivedi_CVGPU08.pdf
         * More detailed explanation can be found here:
         * http://ksimek.github.io/2013/06/03/calibrated_cameras_in_opengl/
         *-------------------------------------------------------------------*/
        //const mat4 proj(
        //                2.0 * fx / w,  -2.0 * skew / w,  1.0 - 2.0 * cx/w,          0.0,
        //                0.0,            2.0 * fy / h,   -1.0 + 2.0 * (cy + 1.0)/h,  0.0,
        //                0.0,            0.0,             (n + f) / (n - f),         2.0 * n * f / (n - f),
        //                0.0,            0.0,            -1.0,                       0.0
        //                );
        //
        //const mat4 mv = mat4::translation(cam.tx, cam.ty, cam.tz) * mat4::rotation(vec3(cam.rx, cam.ry, cam.rz));

        /**-------------------------------------------------------------------
         * For pin-hole cameras, this following function has higher accuracy than
         * this function (I actually doubt the implmentation of this function).
         *-------------------------------------------------------------------*/
        //const vec3 rot_vec(-cam.rx, -cam.ry, -cam.rz);
        //const float angle = rot_vec.length();
        //const quat q(rot_vec / angle, angle);
        //c->setOrientation(q);
        //const vec3 pos(cam.tx, cam.ty, cam.tz);
        //c->setPosition(-q.rotate(pos));
        //const float proj_5th = 2.0 * fy / cam.h;
        //c->setFieldOfView(2.0 * atan(1.0 / proj_5th));
        
        // I doubt the implmentation of this function.
        const mat3 K(
                    fx, skew, cx,
                    0,  fy,   cy,
                    0,  0,    1);
        const mat4 R = mat4::rotation(rot);
        const mat4 T = mat4::translation(t);

        mat34 M(1.0);
        M(1, 1) = -1;// invert the y axis
        M(2, 2) = -1;// invert the z axis

        const mat34& proj = K * M * T * R;
        set_from_projection_matrix(proj);
    }


	/*! Defines the Camera position(), orientation() and fieldOfView() from a
	 projection matrix.

	 \p matrix has to be given in the format used by vision algorithm. It has 3
	 lines and 4 columns. It transforms a point from the world homogeneous
	 coordinate system (4 coordinates: \c sx, \c sy, \c sz and \c s) into a point in
	 the screen homogeneous coordinate system (3 coordinates: \c sx, \c sy, and \c
	 s, where \c x and \c y are the pixel coordinates on the screen).

	 Its three lines correspond to the homogeneous coordinates of the normals to the
	 planes x=0, y=0 and z=0, defined in the Camera coordinate system.

	 The elements of the matrix are ordered in line major order: you can call \c
	 setFromProjectionMatrix(&(matrix[0][0])) if you defined your matrix as a \c
	 double \c matrix[3][4].

	 \attention Passing the result of getProjectionMatrix() or getModelViewMatrix()
	 to this method is not possible (purposefully incompatible matrix dimensions).
	 \p matrix is more likely to be the product of these two matrices, without the
	 last line.

	 Use setFromModelViewMatrix() to set position() and orientation() from a \c
	 GL_MODELVIEW matrix. fieldOfView() can also be retrieved from a \e perspective
	 \c GL_PROJECTION matrix using 2.0 * atan(1.0/projectionMatrix[5]).

	 This code was written by Sylvain Paris. */
    void Camera::set_from_projection_matrix(const mat34 &proj) {
		// The 3 lines of the matrix are the normals to the planes x=0, y=0, z=0
		// in the camera CS. As we normalize them, we do not need the 4th coordinate.
		vec3 line_0(proj(0, 0), proj(0, 1), proj(0, 2));	line_0 = normalize(line_0);
		vec3 line_1(proj(1, 0), proj(1, 1), proj(1, 2));	line_1 = normalize(line_1);
		vec3 line_2(proj(2, 0), proj(2, 1), proj(2, 2));	line_2 = normalize(line_2);

		// The camera position is at (0,0,0) in the camera CS so it is the
		// intersection of the 3 planes. It can be seen as the kernel
		// of the 3x4 projection matrix. We calculate it through 4 dimensional
		// vectorial product. We go directly into 3D that is to say we directly
		// divide the first 3 coordinates by the 4th one.

		// We derive the 4 dimensional vectorial product formula from the
		// computation of a 4x4 determinant that is developped according to
		// its 4th column. This implies some 3x3 determinants.
        const mat3 m1(
			proj(0, 1), proj(0, 2), proj(0, 3),
			proj(1, 1), proj(1, 2), proj(1, 3),
			proj(2, 1), proj(2, 2), proj(2, 3));
        const mat3 m2(
			proj(0, 0), proj(0, 2), proj(0, 3),
			proj(1, 0), proj(1, 2), proj(1, 3),
			proj(2, 0), proj(2, 2), proj(2, 3));
        const mat3 m3(
			proj(0, 0), proj(0, 1), proj(0, 3),
			proj(1, 0), proj(1, 1), proj(1, 3),
			proj(2, 0), proj(2, 1), proj(2, 3));
        const mat3 m4(
			proj(0, 0), proj(0, 1), proj(0, 2),
			proj(1, 0), proj(1, 1), proj(1, 2),
			proj(2, 0), proj(2, 1), proj(2, 2));

#if 0 // This is a bug!!!
		const vec3 cam_pos(determinant(m1), -determinant(m2), determinant(m3) / (-determinant(m4)));
#else
		float X = determinant(m1);
		float Y = -determinant(m2);
		float Z = determinant(m3);
		float T = -determinant(m4);
		const vec3 cam_pos(X / T, Y / T, Z / T);
#endif

		// We compute the rotation matrix column by column.

		// GL Z axis is front facing.
        const vec3 column_2 = -line_2;

		// X-axis is almost like line_0 but should be orthogonal to the Z axis.
		vec3 column_0 = cross(cross(column_2, line_0), column_2);
		column_0 = normalize(column_0);

		// Y-axis is almost like line_1 but should be orthogonal to the Z axis.
		// Moreover line_1 is downward oriented as the screen CS.
		vec3 column_1 = -cross(cross(column_2, line_1), column_2);
		column_1 = normalize(column_1);

        const mat3 rot(column_0, column_1, column_2);

		// We compute the field of view

		// line_1^column_0 -> vector of intersection line between
		// y_screen=0 and x_camera=0 plane.
		// column_2*(...)  -> cos of the angle between Z vector et y_screen=0 plane
		// * 2 -> field of view = 2 * half angle

		// We need some intermediate values.
        vec3 dummy = cross(line_1, column_0);
		dummy = normalize(dummy);
		const float fov = std::acos(dot(column_2, dummy)) * 2.0f;

		// We set the camera.
		quat q;
		q.set_from_rotation_matrix(rot);
		setOrientation(q);
		setPosition(cam_pos);
		setFieldOfView(fov);
	}


	// persp : projectionMatrix_[0]  = f/aspectRatio();
//    bool Camera::setFromProjectionMatrix(const mat4& projectionMatrix)
//    {
//        if ((fabs(projectionMatrix[1]) > 1E-3) || (fabs(projectionMatrix[2]) > 1E-3) ||
//            (fabs(projectionMatrix[3]) > 1E-3) || (fabs(projectionMatrix[4]) > 1E-3) ||
//            (fabs(projectionMatrix[6]) > 1E-3) || (fabs(projectionMatrix[7]) > 1E-3) ||
//            (fabs(projectionMatrix[8]) > 1E-3) || (fabs(projectionMatrix[9]) > 1E-3))
//            std::cerr << "Non null coefficient in projection matrix" << std::endl;
//        else {
//            if ((fabs(projectionMatrix[11] + 1.0) < 1E-5) && (fabs(projectionMatrix[15]) < 1E-5))
//            {
//                if (projectionMatrix[5] < 1E-4) {
//                    std::cerr << "Negative field of view in Camera::setFromProjectionMatrix" << std::endl;
//                    return false;
//                }
//                else
//                    setType(Camera::PERSPECTIVE);
//            }
//            else {
//                if ((fabs(projectionMatrix[11]) < 1E-5) && (fabs(projectionMatrix[15] - 1.0) < 1E-5))
//                    setType(Camera::ORTHOGRAPHIC);
//                else {
//                    std::cerr << "Unable to determine camera type in setFromProjectionMatrix" << std::endl;
//                    return false;
//                }
//            }
//        }
//
//        if (type() == Camera::PERSPECTIVE) {
//            setFieldOfView(2.0 * atan(1.0 / projectionMatrix[5]));
//            const double far = projectionMatrix[14] / (2.0 * (1.0 + projectionMatrix[10]));
//            const double near = (projectionMatrix[10] + 1.0) / (projectionMatrix[10] - 1.0) * far;
//            setSceneRadius((far - near) / 2.0);
//            setSceneCenter(position() + (near + sceneRadius())*viewDirection());
//        }
//        else {
//            const float ZNear = zNear();
//            const float ZFar = zFar();
//            float w, h;
//            getOrthoWidthHeight(w, h);
//            projectionMatrix_[0] = 1.0 / w;
//            projectionMatrix_[5] = 1.0 / h;
//            projectionMatrix_[10] = -2.0 / (ZFar - ZNear);
//            projectionMatrix_[11] = 0.0;
//            projectionMatrix_[14] = -(ZFar + ZNear) / (ZFar - ZNear);
//            projectionMatrix_[15] = 1.0;
//            // same as glOrtho( -w, w, -h, h, zNear(), zFar() );
//            projectionMatrixIsUpToDate_ = true;
//        }
//        return true;
//    }
//

	///////////////////////// Camera to world transform ///////////////////////

	/*! Returns the screen projected coordinates of a point \p src defined in the \p
	 frame coordinate system.

	 When \p frame in \c NULL (default), \p src is expressed in the world coordinate
	 system.

	 The x and y coordinates of the returned vec3 are expressed in pixel, (0,0) being
	 the \e upper left corner of the window. The z coordinate ranges between 0.0
	 (near plane) and 1.0 (excluded, far plane). See the \c gluProject man page for
	 details.

	 unprojectedCoordinatesOf() performs the inverse transformation.

	 See the <a href="../examples/screenCoordSystem.html">screenCoordSystem
	 example</a>.

	 This method only uses the intrinsic Camera parameters (see
	 getModelViewMatrix(), getProjectionMatrix() and getViewport()) and is
	 completely independent of the OpenGL \c GL_MODELVIEW, \c GL_PROJECTION and
	 viewport matrices. You can hence define a virtual Camera and use this method to
	 compute projections out of a classical rendering context.

	 \attention However, if your Camera is not attached to a Viewer (used for
	 offscreen computations for instance), make sure the Camera matrices are updated
	 before calling this method. Call computeModelViewMatrix() and
	 computeProjectionMatrix() to do so.

	 If you call this method several times with no change in the matrices, consider
	 precomputing the projection times modelview matrix to save computation time if
	 required (\c P x \c M in the \c gluProject man page).

	 Here is the code corresponding to what this method does (kindly submitted by
	 Robert W. Kuhn) : \code vec3 project(vec3 point)
	 {
	  int    Viewport[4];
	  double Projection[16], Modelview[16];
	  double matrix[16];

	  // Precomputation begin
	  glGetIntegerv(GL_VIEWPORT         , Viewport);
	  glGetDoublev (GL_MODELVIEW_MATRIX , Modelview);
	  glGetDoublev (GL_PROJECTION_MATRIX, Projection);

	  for (unsigned short m=0; m<4; ++m)
	  {
		for (unsigned short l=0; l<4; ++l)
		{
		  double sum = 0.0;
		  for (unsigned short k=0; k<4; ++k)
			sum += Projection[l+4*k]*Modelview[k+4*m];
		  matrix[l+4*m] = sum;
		}
	  }
	  // Precomputation end

	  double v[4], vs[4];
	  v[0]=point[0]; v[1]=point[1]; v[2]=point[2]; v[3]=1.0;

	  vs[0]=matrix[0 ]*v[0] + matrix[4 ]*v[1] + matrix[8 ]*v[2] + matrix[12 ]*v[3];
	  vs[1]=matrix[1 ]*v[0] + matrix[5 ]*v[1] + matrix[9 ]*v[2] + matrix[13 ]*v[3];
	  vs[2]=matrix[2 ]*v[0] + matrix[6 ]*v[1] + matrix[10]*v[2] + matrix[14 ]*v[3];
	  vs[3]=matrix[3 ]*v[0] + matrix[7 ]*v[1] + matrix[11]*v[2] + matrix[15 ]*v[3];

	  vs[0] /= vs[3];
	  vs[1] /= vs[3];
	  vs[2] /= vs[3];

	  vs[0] = vs[0] * 0.5 + 0.5;
	  vs[1] = vs[1] * 0.5 + 0.5;
	  vs[2] = vs[2] * 0.5 + 0.5;

	  vs[0] = vs[0] * Viewport[2] + Viewport[0];
	  vs[1] = vs[1] * Viewport[3] + Viewport[1];

	  return vec3(vs[0], Viewport[3]-vs[1], vs[2]);
	  }
	 \endcode
	 */
	vec3 Camera::projectedCoordinatesOf(const vec3 &src, const Frame *frame) const {
		vec3 tmp = src;
		if (frame)
			tmp = frame->inverseCoordinatesOf(src);

		const mat4& mvp = modelViewProjectionMatrix();
		const int viewport[] = { 0, 0, screenWidth_, screenHeight_ };
		vec3 vs = mvp * tmp * 0.5f + vec3(0.5f);
		vs.x = vs.x * viewport[2] + viewport[0];
		vs.y = vs.y * viewport[3] + viewport[1];
		return vec3(vs.x, viewport[3] - 1 - vs.y, vs.z);
	}

	/*! Returns the world unprojected coordinates of a point \p src defined in the
	 screen coordinate system.

	 The \p src.x and \p src.y input values are expressed in pixels, (0,0) being the
	 \e upper left corner of the window. \p src.z is a depth value ranging in [0..1[
	 (respectively corresponding to the near and far planes). Note that src.z is \e
	 not a linear interpolation between zNear and zFar. /code src.z = zFar() /
	 (zFar() - zNear()) * (1.0 - zNear() / z); /endcode Where z is the distance from
	 the point you project to the camera, along the viewDirection(). See the \c
	 gluUnProject man page for details.

	 The result is expressed in the \p frame coordinate system. When \p frame is \c
	 NULL (default), the result is expressed in the world coordinates system. The
	 possible \p frame Frame::referenceFrame() are taken into account.

	 projectedCoordinatesOf() performs the inverse transformation.

	 This method only uses the intrinsic Camera parameters (see
	 getModelViewMatrix(), getProjectionMatrix() and getViewport()) and is
	 completely independent of the OpenGL \c GL_MODELVIEW, \c GL_PROJECTION and
	 viewport matrices. You can hence define a virtual Camera and use this method to
	 compute un-projections out of a classical rendering context.

	 \attention However, if your Camera is not attached to a Viewer (used for
	 offscreen computations for instance), make sure the Camera matrices are updated
	 before calling this method (use computeModelViewMatrix(),
	 computeProjectionMatrix()). See also setScreenWidthAndHeight().

	 This method is not computationally optimized. If you call it several times with
	 no change in the matrices, you should buffer the entire inverse projection
	 matrix (modelview, projection and then viewport) to speed-up the queries. See
	 the \c gluUnProject man page for details. */
	vec3 Camera::unprojectedCoordinatesOf(const vec3 &src, const Frame *frame) const {
		const mat4& mvp = modelViewProjectionMatrix();

		const int viewport[] = { 0, 0, screenWidth_, screenHeight_ };
		vec3 vs(src.x, viewport[3] - 1 - src.y, src.z);
		vs.x = (float)(vs.x - viewport[0]) / (float)viewport[2] * 2.0f - 1.0f;
		vs.y = (float)(vs.y - viewport[1]) / (float)viewport[3] * 2.0f - 1.0f;
		vs.z = vs.z * 2.0f - 1.0f;

		vec3 p = inverse(mvp) * vs;
		if (frame)
			p = frame->coordinatesOf(p);
		return p;
	}

	/*! Gives the coefficients of a 3D half-line passing through the Camera eye and
	 pixel (x,y).

	 The origin of the half line (eye position) is stored in \p orig, while \p dir
	 contains the properly oriented and normalized direction of the half line.

	 \p x and \p y are expressed in Qt format (origin in the upper left corner). Use
	 screenHeight() - y to convert to OpenGL units.

	 This method is useful for analytical intersection in a selection method.

	 See the <a href="../examples/select.html">select example</a> for an
	 illustration. */
	void Camera::convertClickToLine(int x, int y, vec3 &orig, vec3 &dir) const {
		switch (type()) {
		case Camera::PERSPECTIVE:
			orig = position();
			dir = vec3(((2.0f * x / screenWidth()) - 1.0f) *
				tan(fieldOfView() / 2.0f) * aspectRatio(),
				((2.0f * (screenHeight() - y) / screenHeight()) - 1.0f) *
				tan(fieldOfView() / 2.0f),
				-1.0f);
			dir = worldCoordinatesOf(dir) - orig;
			dir.normalize();
			break;

		case Camera::ORTHOGRAPHIC: {
			float w, h;
			getOrthoWidthHeight(w, h);
			orig = vec3((2.0f * x / screenWidth() - 1.0f) * w,
				-(2.0f * y / screenHeight() - 1.0f) * h, 0.0f);
			orig = worldCoordinatesOf(orig);
			dir = viewDirection();
			break;
		}
		}
	}


	/*! Returns the 6 plane equations of the Camera frustum.

	The six 4-component vectors of \p coef respectively correspond to the left,
	right, near, far, top and bottom Camera frustum planes. Each vector holds a
	plane equation of the form: \code a*x + b*y + c*z + d = 0 \endcode where \c a,
	\c b, \c c and \c d are the 4 components of each vector, in that order.

	See the <a href="../examples/frustumCulling.html">frustumCulling example</a> for
	an application.

	This format is compatible with the \c glClipPlane() function. One camera frustum
	plane can hence be applied in an other viewer to visualize the culling results:
	\code
	 // Retrieve plane equations
	 double coef[6][4];
	 mainViewer->camera()->getFrustumPlanesCoefficients(coef);

	 // These two additional clipping planes (which must have been enabled)
	 // will reproduce the mainViewer's near and far clipping.
	 glClipPlane(GL_CLIP_PLANE0, coef[2]);
	 glClipPlane(GL_CLIP_PLANE1, coef[3]);
	\endcode */
	void Camera::getFrustumPlanesCoefficients(float coef[6][4]) const
	{
		// Computed once and for all
		const vec3 pos = position();
		const vec3 viewDir = viewDirection();
		const vec3 up = upVector();
		const vec3 right = rightVector();
		const float posViewDir = dot(pos, viewDir);

		static vec3 normal[6];
		static float dist[6];

		switch (type())
		{
		case Camera::PERSPECTIVE:
		{
			const float hhfov = horizontalFieldOfView() / 2.0f;
			const float chhfov = cos(hhfov);
			const float shhfov = sin(hhfov);
			normal[0] = -shhfov * viewDir;
			normal[1] = normal[0] + chhfov * right;
			normal[0] = normal[0] - chhfov * right;

			normal[2] = -viewDir;
			normal[3] = viewDir;

			const float hfov = fieldOfView() / 2.0f;
			const float chfov = cos(hfov);
			const float shfov = sin(hfov);
			normal[4] = -shfov * viewDir;
			normal[5] = normal[4] - chfov * up;
			normal[4] = normal[4] + chfov * up;

			for (int i = 0; i < 2; ++i)
				dist[i] = dot(pos, normal[i]);
			for (int j = 4; j < 6; ++j)
				dist[j] = dot(pos, normal[j]);

			// Natural equations are:
			// dist[0,1,4,5] = pos * normal[0,1,4,5];
			// dist[2] = (pos + zNear() * viewDir) * normal[2];
			// dist[3] = (pos + zFar()  * viewDir) * normal[3];

			// 2 times less computations using expanded/merged equations. Dir vectors are normalized.
			const float posRightCosHH = chhfov * dot(pos, right);
			dist[0] = -shhfov * posViewDir;
			dist[1] = dist[0] + posRightCosHH;
			dist[0] = dist[0] - posRightCosHH;
			const float posUpCosH = chfov * dot(pos, up);
			dist[4] = -shfov * posViewDir;
			dist[5] = dist[4] - posUpCosH;
			dist[4] = dist[4] + posUpCosH;

			break;
		}
		case Camera::ORTHOGRAPHIC:
			normal[0] = -right;
			normal[1] = right;
			normal[4] = up;
			normal[5] = -up;

			float hw, hh;
			getOrthoWidthHeight(hw, hh);
			dist[0] = dot((pos - hw * right), normal[0]);
			dist[1] = dot((pos + hw * right), normal[1]);
			dist[4] = dot((pos + hh * up), normal[4]);
			dist[5] = dot((pos - hh * up), normal[5]);
			break;
		}

		// Front and far planes are identical for both camera types.
		normal[2] = -viewDir;
		normal[3] = viewDir;
		dist[2] = -posViewDir - zNear();
		dist[3] = posViewDir + zFar();

		for (int i = 0; i < 6; ++i)
		{
			coef[i][0] = normal[i].x;
			coef[i][1] = normal[i].y;
			coef[i][2] = normal[i].z;
			coef[i][3] = dist[i];
		}
	}


	/*First of all, let's assume that we're going to store the frustum values in a variable like this:
	float frustum[6][4];
	That's six sets of four numbers (six planes, each with an A, B, C, and D value).
	*/
	void Camera::getFrustumPlanesCoefficients2(float frustum[6][4]) const
	{
		const mat4& clip = modelViewProjectionMatrix();

		/* Extract the numbers for the LEFT plane */
		frustum[0][0] = clip[3] + clip[0];
		frustum[0][1] = clip[7] + clip[4];
		frustum[0][2] = clip[11] + clip[8];
		frustum[0][3] = clip[15] + clip[12];

		/* Normalize the result */
		float  t = sqrt(frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] + frustum[0][2] * frustum[0][2]);
		frustum[0][0] /= t;
		frustum[0][1] /= t;
		frustum[0][2] /= t;
		frustum[0][3] /= t;

		/* Extract the numbers for the RIGHT plane */
		frustum[1][0] = clip[3] - clip[0];
		frustum[1][1] = clip[7] - clip[4];
		frustum[1][2] = clip[11] - clip[8];
		frustum[1][3] = clip[15] - clip[12];

		/* Normalize the result */
		t = sqrt(frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1] + frustum[1][2] * frustum[1][2]);
		frustum[1][0] /= t;
		frustum[1][1] /= t;
		frustum[1][2] /= t;
		frustum[1][3] /= t;

		/* Extract the NEAR plane */
		frustum[2][0] = clip[3] + clip[2];
		frustum[2][1] = clip[7] + clip[6];
		frustum[2][2] = clip[11] + clip[10];
		frustum[2][3] = clip[15] + clip[14];

		/* Normalize the result */
		t = sqrt(frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1] + frustum[2][2] * frustum[2][2]);
		frustum[2][0] /= t;
		frustum[2][1] /= t;
		frustum[2][2] /= t;
		frustum[2][3] /= t;

		/* Extract the FAR plane */
		frustum[3][0] = clip[3] - clip[2];
		frustum[3][1] = clip[7] - clip[6];
		frustum[3][2] = clip[11] - clip[10];
		frustum[3][3] = clip[15] - clip[14];

		/* Normalize the result */
		t = sqrt(frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1] + frustum[3][2] * frustum[3][2]);
		frustum[3][0] /= t;
		frustum[3][1] /= t;
		frustum[3][2] /= t;
		frustum[3][3] /= t;

		/* Extract the TOP plane */
		frustum[4][0] = clip[3] - clip[1];
		frustum[4][1] = clip[7] - clip[5];
		frustum[4][2] = clip[11] - clip[9];
		frustum[4][3] = clip[15] - clip[13];

		/* Normalize the result */
		t = sqrt(frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1] + frustum[4][2] * frustum[4][2]);
		frustum[4][0] /= t;
		frustum[4][1] /= t;
		frustum[4][2] /= t;
		frustum[4][3] /= t;

		/* Extract the BOTTOM plane */
		frustum[5][0] = clip[3] + clip[1];
		frustum[5][1] = clip[7] + clip[5];
		frustum[5][2] = clip[11] + clip[9];
		frustum[5][3] = clip[15] + clip[13];

		/* Normalize the result */
		t = sqrt(frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1] + frustum[5][2] * frustum[5][2]);
		frustum[5][0] /= t;
		frustum[5][1] /= t;
		frustum[5][2] /= t;
		frustum[5][3] /= t;
	}


	void Camera::onFrameModified() {
		projectionMatrixIsUpToDate_ = false;
		modelViewMatrixIsUpToDate_ = false;
	}




	/*
	// The code below will allow us to make checks within the frustum.  For example,
	// if we want to see if a point, a sphere, or a cube lies inside of the frustum.
	// Because all of our planes point INWARDS (The normals are all pointing inside the frustum)
	// we then can assume that if a point is in FRONT of all of the planes, it's inside.

	///////////////////////////////// POINT IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
	/////
	/////	This determines if a point is inside of the frustum
	/////
	///////////////////////////////// POINT IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

	bool CFrustum::PointInFrustum(float x, float y, float z)
	{
		// If you remember the plane equation (A*x + B*y + C*z + D = 0), then the rest
		// of this code should be quite obvious and easy to figure out yourself.
		// In case don't know the plane equation, it might be a good idea to look
		// at our Plane Collision tutorial at www.GameTutorials.com in OpenGL Tutorials.
		// I will briefly go over it here.  (A,B,C) is the (X,Y,Z) of the normal to the plane.
		// They are the same thing... but just called ABC because you don't want to say:
		// (x*x + y*y + z*z + d = 0).  That would be wrong, so they substitute them.
		// the (x, y, z) in the equation is the point that you are testing.  The D is
		// The distance the plane is from the origin.  The equation ends with "= 0" because
		// that is true when the point (x, y, z) is ON the plane.  When the point is NOT on
		// the plane, it is either a negative number (the point is behind the plane) or a
		// positive number (the point is in front of the plane).  We want to check if the point
		// is in front of the plane, so all we have to do is go through each point and make
		// sure the plane equation goes out to a positive number on each side of the frustum.
		// The result (be it positive or negative) is the distance the point is front the plane.

		// Go through all the sides of the frustum
		for (int i = 0; i < 6; i++)
		{
			// Calculate the plane equation and check if the point is behind a side of the frustum
			if (m_Frustum[i][A] * x + m_Frustum[i][B] * y + m_Frustum[i][C] * z + m_Frustum[i][D] <= 0)
			{
				// The point was behind a side, so it ISN'T in the frustum
				return false;
			}
		}

		// The point was inside of the frustum (In front of ALL the sides of the frustum)
		return true;
	}


	///////////////////////////////// SPHERE IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
	/////
	/////	This determines if a sphere is inside of our frustum by it's center and radius.
	/////
	///////////////////////////////// SPHERE IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

	bool CFrustum::SphereInFrustum(float x, float y, float z, float radius)
	{
		// Now this function is almost identical to the PointInFrustum(), except we
		// now have to deal with a radius around the point.  The point is the center of
		// the radius.  So, the point might be outside of the frustum, but it doesn't
		// mean that the rest of the sphere is.  It could be half and half.  So instead of
		// checking if it's less than 0, we need to add on the radius to that.  Say the
		// equation produced -2, which means the center of the sphere is the distance of
		// 2 behind the plane.  Well, what if the radius was 5?  The sphere is still inside,
		// so we would say, if(-2 < -5) then we are outside.  In that case it's false,
		// so we are inside of the frustum, but a distance of 3.  This is reflected below.

		// Go through all the sides of the frustum
		for (int i = 0; i < 6; i++)
		{
			// If the center of the sphere is farther away from the plane than the radius
			if (m_Frustum[i][A] * x + m_Frustum[i][B] * y + m_Frustum[i][C] * z + m_Frustum[i][D] <= -radius)
			{
				// The distance was greater than the radius so the sphere is outside of the frustum
				return false;
			}
		}

		// The sphere was inside of the frustum!
		return true;
	}


	///////////////////////////////// CUBE IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*
	/////
	/////	This determines if a cube is in or around our frustum by it's center and 1/2 it's length
	/////
	///////////////////////////////// CUBE IN FRUSTUM \\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\\*

	bool CFrustum::CubeInFrustum(float x, float y, float z, float size)
	{
		// This test is a bit more work, but not too much more complicated.
		// Basically, what is going on is, that we are given the center of the cube,
		// and half the length.  Think of it like a radius.  Then we checking each point
		// in the cube and seeing if it is inside the frustum.  If a point is found in front
		// of a side, then we skip to the next side.  If we get to a plane that does NOT have
		// a point in front of it, then it will return false.

		// *Note* - This will sometimes say that a cube is inside the frustum when it isn't.
		// This happens when all the corners of the bounding box are not behind any one plane.
		// This is rare and shouldn't effect the overall rendering speed.

		for (int i = 0; i < 6; i++)
		{
			if (m_Frustum[i][A] * (x - size) + m_Frustum[i][B] * (y - size) + m_Frustum[i][C] * (z - size) + m_Frustum[i][D] > 0)
				continue;
			if (m_Frustum[i][A] * (x + size) + m_Frustum[i][B] * (y - size) + m_Frustum[i][C] * (z - size) + m_Frustum[i][D] > 0)
				continue;
			if (m_Frustum[i][A] * (x - size) + m_Frustum[i][B] * (y + size) + m_Frustum[i][C] * (z - size) + m_Frustum[i][D] > 0)
				continue;
			if (m_Frustum[i][A] * (x + size) + m_Frustum[i][B] * (y + size) + m_Frustum[i][C] * (z - size) + m_Frustum[i][D] > 0)
				continue;
			if (m_Frustum[i][A] * (x - size) + m_Frustum[i][B] * (y - size) + m_Frustum[i][C] * (z + size) + m_Frustum[i][D] > 0)
				continue;
			if (m_Frustum[i][A] * (x + size) + m_Frustum[i][B] * (y - size) + m_Frustum[i][C] * (z + size) + m_Frustum[i][D] > 0)
				continue;
			if (m_Frustum[i][A] * (x - size) + m_Frustum[i][B] * (y + size) + m_Frustum[i][C] * (z + size) + m_Frustum[i][D] > 0)
				continue;
			if (m_Frustum[i][A] * (x + size) + m_Frustum[i][B] * (y + size) + m_Frustum[i][C] * (z + size) + m_Frustum[i][D] > 0)
				continue;

			// If we get here, it isn't in the frustum
			return false;
		}

		return true;
	}

	*/
	/////////////////////////////////////////////////////////////////////////////////
	//
	// * QUICK NOTES * 
	//
	// WOZZERS!  That seemed like an incredible amount to look at, but if you break it
	// down, it's not.  Frustum culling is a VERY useful thing when it comes to 3D.
	// If you want a large world, there is no way you are going to send it down the
	// 3D pipeline every frame and let OpenGL take care of it for you.  That would
	// give you a 0.001 frame rate.  If you hit '+' and bring the sphere count up to
	// 1000, then take off culling, you will see the HUGE difference it makes.  
	// Also, you wouldn't really be rendering 1000 spheres.  You would most likely
	// use the sphere code for larger objects.  Let me explain.  Say you have a bunch
	// of objects, well... all you need to do is give the objects a radius, and then
	// test that radius against the frustum.  If that sphere is in the frustum, then you
	// render that object.  Also, you won't be rendering a high poly sphere so it won't
	// be so slow.  This goes for bounding box's too (Cubes).  If you don't want to
	// do a cube, it is really easy to convert the code for rectangles.  Just pass in
	// a width and height, instead of just a length.  Remember, it's HALF the length of
	// the cube, not the full length.  So it would be half the width and height for a rect.
	// 
	// This is a perfect starter for an octree tutorial.  Wrap you head around the concepts
	// here and then see if you can apply this to making an octree.  Hopefully we will have
	// a tutorial up and running for this subject soon.  Once you have frustum culling,
	// the next step is getting space partitioning.  Either it being a BSP tree of an Octree.
	// 
	// Let's go over a brief overview of the things we learned here:
	//
	// 1) First we need to abstract the frustum from OpenGL.  To do that we need the
	//    projection and modelview matrix.  To get the projection matrix we use:
	//
	//			glGetFloatv( GL_PROJECTION_MATRIX, /* An Array of 16 floats */ );
	//    Then, to get the modelview matrix we use:
	//
	//			glGetFloatv( GL_MODELVIEW_MATRIX, /* An Array of 16 floats */ );
	//    
	//	  These 2 functions gives us an array of 16 floats (The matrix).
	//
	// 2) Next, we need to combine these 2 matrices.  We do that by matrix multiplication.
	//
	// 3) Now that we have the 2 matrixes combined, we can abstract the sides of the frustum.
	//    This will give us the normal and the distance from the plane to the origin (ABC and D).
	//
	// 4) After abstracting a side, we want to normalize the plane data.  (A B C and D).
	//
	// 5) Now we have our frustum, and we can check points against it using the plane equation.
	//    Once again, the plane equation (A*x + B*y + C*z + D = 0) says that if, point (X,Y,Z)
	//    times the normal of the plane (A,B,C), plus the distance of the plane from origin,
	//    will equal 0 if the point (X, Y, Z) lies on that plane.  If it is behind the plane
	//    it will be a negative distance, if it's in front of the plane (the way the normal is facing)
	//    it will be a positive number.
	//
	//
	// If you need more help on the plane equation and why this works, download our
	// Ray Plane Intersection Tutorial at www.GameTutorials.com.
	//
	// That's pretty much it with frustums.  There is a lot more we could talk about, but
	// I don't want to complicate this tutorial more than I already have.
	//
	// I want to thank Mark Morley for his tutorial on frustum culling.  Most of everything I got
	// here comes from his teaching.  If you want more in-depth, visit his tutorial at:
	//
	// http://www.markmorley.com/opengl/frustumculling.html
	// http://www.crownandcutlass.com/features/technicaldetails/frustum.html
	//

}
