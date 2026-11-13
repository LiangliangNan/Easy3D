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

#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/transform.h>

#include <algorithm>

#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/key_frame_interpolator.h>


// \cond
namespace easy3d {


	/*! Default constructor.

	 scene_center() is (0,0,0), scene_radius() is 1.0 and type() is PERSPECTIVE with a
	 \c M_PI/4 field_of_view(); the Camera looks along the +Y axis (set_view_direction())
	 with Z up, then show_entire_scene() places it so that the unit scene is visible.
	 screen_width()/screen_height() start at 600x400 and are replaced by the viewer's
	 logical size as soon as the Camera is driven by a Viewer, and the stereo parameters
	 of the original libQGLViewer Camera are not part of Easy3D. */
	Camera::Camera()
		: frame_(nullptr)
		, field_of_view_(static_cast<float>(M_PI / 4.0))
		, model_view_matrix_is_up_to_date_(false)
		, projection_matrix_is_up_to_date_(false)
	{
        interpolation_kfi_ = new KeyFrameInterpolator;
        set_frame(new ManipulatedCameraFrame());

		// Requires field_of_view(): the scene bounds below derive z_near()/z_far()
		// and the orthographic scale from it.
		set_scene_radius(1.0f);

		// Initial value (only scaled after this)
		ortho_coef_ = std::tan(field_of_view() / 2.0f);

		// Also defines the pivot_point(), which changes ortho_coef_. Requires a
		// frame().
		set_scene_center(vec3(0.0f, 0.0f, 0.0f));

		// Requires field_of_view() when called with ORTHOGRAPHIC. Attention to
		// projection_matrix_ below.
		set_type(PERSPECTIVE);
		set_view_direction(vec3(0.0, 1.0, 0.0));

		// #CONNECTION# keep these in sync with the defaults documented by
		// z_near_coefficient() and z_clipping_coefficient().
		set_z_near_coefficient(0.001f);
		set_z_clipping_coefficient(std::sqrt(3.0f));

		// Dummy values
		set_screen_width_and_height(600, 400);

        model_view_matrix_.load_identity();
        projection_matrix_.load_zero(); // compute_projection_matrix() is lazy and assumes 0.0 everywhere

		// position and orient the camera
		show_entire_scene();
	}

	/*! Virtual destructor.

	 The frame() and keyframe_interpolator() are deleted: the Camera created both in
	 its constructors and owns them (the interpolator is never shared, and set_frame()
	 rebinds it to the new frame). Note that the frame set_frame() \e replaces is not
	 deleted, so delete the previous frame yourself when it is not shared. */
	Camera::~Camera() {
		delete frame_;
        delete interpolation_kfi_;
	}

	/*! Copy constructor. Performs a deep copy using operator=(). */
	Camera::Camera(const Camera &camera)
		: frame_(nullptr)
	{
        interpolation_kfi_ = new KeyFrameInterpolator;

        // Requires the interpolation_kfi_
		set_frame(new ManipulatedCameraFrame(*camera.frame()));

        model_view_matrix_.load_identity();
        projection_matrix_.load_zero(); // compute_projection_matrix() is lazy and assumes 0.0 everywhere

		(*this) = camera;
	}

	/*! Equal operator.

	 All the parameters of \p camera are copied. The frame() pointer is not
	 modified, but its Frame::position() and Frame::orientation() are set to those
	 of \p camera.

	 \attention The Camera screen_width() and screen_height() are set to those of \p
	 camera. If your Camera is associated with a Viewer, you should update these
	 value after the call to this method: \code
	 *(camera()) = other_camera;
	 camera()->set_screen_width_and_height(width(), height());
	 \endcode
	 The same applies to scene_center() and scene_radius(), if needed. */
	Camera &Camera::operator=(const Camera &camera) {
		set_screen_width_and_height(camera.screen_width(), camera.screen_height());
		set_field_of_view(camera.field_of_view());
		set_scene_radius(camera.scene_radius());
		set_scene_center(camera.scene_center());
		set_z_near_coefficient(camera.z_near_coefficient());
		set_z_clipping_coefficient(camera.z_clipping_coefficient());
		set_type(camera.type());

		ortho_coef_ = camera.ortho_coef_;
		projection_matrix_is_up_to_date_ = false;

		// frame_ and interpolation_kfi_ pointers are not shared.
		frame_->set_reference_frame(nullptr);
		frame_->set_position(camera.position());
		frame_->set_orientation(camera.orientation());

        interpolation_kfi_->stop_interpolation();

		compute_projection_matrix();
		compute_model_view_matrix();

		return *this;
	}

	/*! Sets Camera screen_width() and screen_height() (expressed in pixels).

	You should not call this method when the Camera is associated with a Viewer,
	since the latter automatically updates these values when it is resized (hence
	overwriting your values).

	Non-positive dimension are silently replaced by a 1 pixel value to ensure
	frustum coherence.

	If your Camera is used without a Viewer (offscreen rendering, shadow maps),
	use set_aspect_ratio() instead to define the projection matrix. */
	void Camera::set_screen_width_and_height(int width, int height) {
		// Prevent negative and zero dimensions that would cause divisions by zero.
		screen_width_ = width > 0 ? width : 1;
		screen_height_ = height > 0 ? height : 1;
		projection_matrix_is_up_to_date_ = false;
	}

	/*! Returns the near clipping plane distance used by the Camera projection
	 matrix.

	 The clipping planes' positions depend on the scene_radius() and scene_center()
	 rather than being fixed small-enough and large-enough values. A good scene
	 dimension approximation will hence result in an optimal precision of the
	 z-buffer.

	 The near clipping plane is positioned at a distance equal to
	 z_clipping_coefficient() * scene_radius() in front of the scene_center(): \code
	 z_near = distance_to_scene_center() - z_clipping_coefficient()*scene_radius();
	 \endcode

	 In order to prevent negative or too small z_near() values (which would degrade
	 the z precision), z_near_coefficient() is used when the Camera is inside the
	 scene_radius() sphere: \code const double z_min = z_near_coefficient() *
	 z_clipping_coefficient() * scene_radius(); if (z_near < z_min) z_near = z_min;
	 // With an ORTHOGRAPHIC type, the value is simply clamped to 0.0
	 \endcode

	 See also the z_far(), z_clipping_coefficient() and z_near_coefficient()
	 documentations.

	 If you need a completely different z_near computation, overload the z_near() and
	 z_far() (and, for an orthographic Camera, get_ortho_width_height()) methods in a
	 new class that publicly inherits from Camera:
	 \code
	     class MyCamera : public Camera
         {
           virtual float z_near() const { return 0.001f; };
           virtual float z_far() const { return 100.0f; };
         }
	 \endcode

	 Since a Viewer creates and owns its own Camera, such a subclass is used with a
	 Camera you construct and drive yourself (see the Tutorial_205_RealCamera example,
	 which configures a Camera from real intrinsic and extrinsic parameters).

	 \attention The value is always positive although the clipping plane is
	 positioned at a negative z value in the Camera coordinate system. This follows
	 the \c gluPerspective standard. */
	float Camera::z_near() const {
#if 0 // standard camera
        return 0.001f;
#else
		const float z_near_scene = z_clipping_coefficient() * scene_radius();
		float z = distance_to_scene_center() - z_near_scene;

		// Prevents negative or null z_near values.
		const float z_min = z_near_coefficient() * z_near_scene;
		if (z < z_min)
			switch (type()) {
			case PERSPECTIVE:
				z = z_min;
				break;
			case ORTHOGRAPHIC:
				z = 0.0;
				break;
			}
		return z;
#endif
	}

	/*! Returns the far clipping plane distance used by the Camera projection
	matrix.

	The far clipping plane is positioned at a distance equal to
	z_clipping_coefficient() * scene_radius() behind the scene_center(): \code z_far =
	distance_to_scene_center() + z_clipping_coefficient()*scene_radius(); \endcode

	See the z_near() documentation for details. */
	float Camera::z_far() const {
#if 0 // standard camera
        return 1000.0f;
#else
		return distance_to_scene_center() + z_clipping_coefficient() * scene_radius();
#endif
	}

	/*! Sets the vertical field_of_view() of the Camera (in radians).

	Only the vertical value is stored: horizontal_field_of_view() is derived from it
	with aspect_ratio(), and set_horizontal_field_of_view() does the opposite. The
	projection matrix is recomputed on the next query, and set_fov_to_fit_scene()
	uses the same setter to make the whole scene fit vertically. */
	void Camera::set_field_of_view(float fov) {
		field_of_view_ = fov;
		projection_matrix_is_up_to_date_ = false;
	}

	/*! Defines the Camera type().

	Changing the camera Type alters the viewport and the objects' sizes can be
	changed. This method guarantees that the two frustum match in a plane normal to
	view_direction(), passing through the pivot_point().

	Prefix the type with \c Camera if needed, as in:
	\code
	    camera()->set_type(ORTHOGRAPHIC);
	\endcode */
	void Camera::set_type(Type type) {
		// make ORTHOGRAPHIC frustum fit PERSPECTIVE (at least in plane normal to
		// view_direction(), passing through RAP). Done only when CHANGING type since
		// ortho_coef_ may have been changed with a set_pivot_point() in the meantime.
		if ((type == ORTHOGRAPHIC) && (type_ == PERSPECTIVE))
			ortho_coef_ = std::tan(field_of_view() / 2.0f);
		type_ = type;
		projection_matrix_is_up_to_date_ = false;
	}

	/*! Sets the Camera frame().

	If you want to move the Camera, use set_position() and set_orientation() or one of
	the Camera positioning methods (look_at(), fit_sphere(), show_entire_scene()...)
	instead.

	If you want to save the Camera position(), there's no need to call this method
	either. Use keyframe_interpolator() instead, e.g. add_keyframe() to record the
	pose and start_interpolation() to replay it.

	This method is actually mainly useful if you derive the ManipulatedFrame
	class and want to use an instance of your new class to move the Camera.

	A \c NULL \p mcf pointer will silently be ignored. The calling method is
	responsible for deleting the previous frame() pointer if needed in order to
	prevent memory leaks. */
	void Camera::set_frame(ManipulatedCameraFrame *const mcf) {
		if (!mcf)
			return;

		frame_ = mcf;
        interpolation_kfi_->set_frame(frame());

        easy3d::connect(&frame_->modified,  this,  &Camera::modified);
        // frame_->modified.connect(this, &Camera::modified); // this should also work
        
        modified();
	}

	/*! Returns the distance from the Camera center to scene_center(), projected
	  along the Camera Z axis. Used by z_near() and z_far() to optimize the Z range.
	*/
	float Camera::distance_to_scene_center() const {
		return std::fabs((frame()->coordinates_of(scene_center())).z);
	}

	/*! Returns the \p half_width and \p half_height of the Camera orthographic
	 frustum.

	 These values are only valid and used when the Camera is of type()
	 ORTHOGRAPHIC. They are expressed in world units and are used by
	 compute_projection_matrix() to define the projection matrix as an \c glOrtho(
	 -half_width, half_width, -half_height, half_height, z_near(), z_far() ) would do.

	 These values are proportional to the Camera (z projected) distance to the
	 pivot_point(). When zooming on the object, the Camera is translated forward \e
	 and its frustum is narrowed, making the object appear bigger on screen, as
	 intuitively expected.

	 This method is virtual: overload it (as well as z_near() and z_far()) to change
	 this behavior if desired. */
	void Camera::get_ortho_width_height(float &half_width, float &half_height) const {
		const float dist = ortho_coef_ * std::fabs(camera_coordinates_of(pivot_point()).z);
		//#CONNECTION# fit_screen_region
		half_width = dist * ((aspect_ratio() < 1.0f) ? 1.0f : aspect_ratio());
		half_height = dist * ((aspect_ratio() < 1.0f) ? 1.0f / aspect_ratio() : 1.0f);
	}

	/*! Computes the projection matrix associated with the Camera.

	 If type() is PERSPECTIVE, defines a \c GL_PROJECTION matrix similar to
	 what would \c gluPerspective() do using the field_of_view(), window
	 aspect_ratio(), z_near() and z_far() parameters.

	 If type() is ORTHOGRAPHIC, the projection matrix is as what \c
	 glOrtho() would do. Frustum's width and height are set using
	 get_ortho_width_height().

	 Both types use z_near() and z_far() to place clipping planes. These values are
	 determined from scene_radius() and scene_center() so that they best fit the scene
	 size.

	 Use projection_matrix() to retrieve this matrix. That accessor (and
	 model_view_projection_matrix()) calls this method itself, so you only need to
	 call it explicitly to force an immediate recomputation. Overriding the virtual
	 z_near(), z_far() and get_ortho_width_height() is usually enough to get an exotic
	 frustum; override this method, or install a matrix with set_projection_matrix(),
	 if you want to define the matrix as a whole.

	 \note A Camera used on its own, without a Viewer (offscreen rendering, shadow
	 maps, (un)projected_coordinates_of() computations), needs no explicit call: the
	 accessors recompute on demand. Only set_projection_matrix() bypasses this
	 mechanism, and it is cleared by the next parameter change. */
	void Camera::compute_projection_matrix()
	{
		if (projection_matrix_is_up_to_date_)
			return;

		const float ZNear = z_near();
		const float ZFar = z_far();

		switch (type())
		{
		case PERSPECTIVE:
		{
			// #CONNECTION# all non-null coefficients were set to 0.0 in constructor.
			const float f = 1.0f / std::tan(field_of_view() / 2.0f);
			projection_matrix_[0] = f / aspect_ratio();
			projection_matrix_[5] = f;
			projection_matrix_[10] = (ZNear + ZFar) / (ZNear - ZFar);
			projection_matrix_[11] = -1.0f;
			projection_matrix_[14] = 2.0f * ZNear * ZFar / (ZNear - ZFar);
			projection_matrix_[15] = 0.0f;
			// same as gluPerspective( 180.0*field_of_view()/M_PI, aspect_ratio(), z_near(), z_far() );
			break;
		}
		case ORTHOGRAPHIC:
		{
			float w, h;
			get_ortho_width_height(w, h);
			projection_matrix_[0] = 1.0f / w;
			projection_matrix_[5] = 1.0f / h;
			projection_matrix_[10] = -2.0f / (ZFar - ZNear);
			projection_matrix_[11] = 0.0f;
			projection_matrix_[14] = -(ZFar + ZNear) / (ZFar - ZNear);
			projection_matrix_[15] = 1.0f;
			// same as glOrtho( -w, w, -h, h, z_near(), z_far() );
			break;
		}
		}

		projection_matrix_is_up_to_date_ = true;
	}

	/*! Computes the model_view matrix associated with the Camera's position() and
	 orientation().

	 This matrix converts from the world coordinates system to the Camera
	 coordinates system, so that coordinates can then be projected on screen using
	 the projection matrix (see compute_projection_matrix()).

	 Use model_view_matrix() to retrieve this matrix.

	 \note The matrix is recomputed on demand by model_view_matrix() and
	 model_view_projection_matrix(), so a Camera used without a Viewer (offscreen
	 rendering, coordinate conversions) needs no explicit call. Only
	 set_model_view_matrix() bypasses this mechanism: it installs the matrix as-is and
	 the next pose change discards it. */
	void Camera::compute_model_view_matrix()
	{
		if (model_view_matrix_is_up_to_date_)
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

		model_view_matrix_[0] = 1.0f - q11 - q22;
		model_view_matrix_[1] = q01 - q23;
		model_view_matrix_[2] = q02 + q13;
		model_view_matrix_[3] = 0.0f;

		model_view_matrix_[4] = q01 + q23;
		model_view_matrix_[5] = 1.0f - q22 - q00;
		model_view_matrix_[6] = q12 - q03;
		model_view_matrix_[7] = 0.0f;

		model_view_matrix_[8] = q02 - q13;
		model_view_matrix_[9] = q12 + q03;
		model_view_matrix_[10] = 1.0f - q11 - q00;
		model_view_matrix_[11] = 0.0;

		const vec3 t = q.inverse_rotate(position());

		model_view_matrix_[12] = -t.x;
		model_view_matrix_[13] = -t.y;
		model_view_matrix_[14] = -t.z;
		model_view_matrix_[15] = 1.0;

		model_view_matrix_is_up_to_date_ = true;
	}

	/*! Returns the Camera projection matrix.

	 Based on compute_projection_matrix() to make sure the Camera projection matrix is
	 up to date.

	 This matrix only reflects the Camera's internal parameters. It is the matrix the
	 backends bind for drawing (see the per-drawable matrices computed by
	 Drawable::draw and the picking code), so it may differ from a projection matrix
	 that a rendering pass installed itself, e.g. the one used while
	 set_projection_matrix() was in effect or the one of a shadow-map pass.

	 The matrix is OpenGL-style: right-handed camera space looking along \c -Z, with
	 clip coordinates mapped to the [-1, 1] x [-1, 1] x [-1, 1] cube. Only the Camera
	 is described here; the window origin and the pixel size of the render target
	 belong to the Viewer, which converts with world_to_window()/window_to_world()
	 (see also projected_coordinates_of()/unprojected_coordinates_of() here and
	 transform::project()/unproject() below). Prefer those over multiplying by this
	 matrix by hand: they behave the same on both backends.

	 The matrix is stored in \e column-major order (see mat4 for details).

	 See also model_view_matrix() and set_projection_matrix(). */
	const mat4& Camera::projection_matrix() const
	{
		// May not be needed, but easier like this.
		const_cast<Camera*>(this)->compute_projection_matrix();
		return projection_matrix_;
	}

	/*! Returns the Camera model view matrix.

	 First calls compute_model_view_matrix() to define the Camera model view matrix.

	 The returned matrix is the pure Camera pose: it converts from the world to the
	 Camera coordinate system, before any per-drawable manipulated matrix is applied
	 (Drawable::manipulated_matrix() is what extra local transform a drawable adds).
	 Because the Camera holds no OpenGL state, this is exactly the matrix the backends
	 use, and it does not change when a render pass pushes and pops its own transforms.

	 The matrix is stored in \e column-major order (see mat4 for details).

	 See also projection_matrix(), model_view_projection_matrix() and
	 set_from_model_view_matrix(). */
	const mat4& Camera::model_view_matrix() const
	{
		// May not be needed, but easier like this.
		const_cast<Camera*>(this)->compute_model_view_matrix();
		return model_view_matrix_;
	}


	/*! Returns the product of the projection and the model view matrices.

	  Calls projection_matrix() and model_view_matrix() (which recompute both caches
	  on demand) and returns their product, i.e. the matrix that maps world
	  coordinates to clip coordinates. This is the matrix used by
	  transform::project()/unproject() and by the backends when they bind a drawable. */
	mat4 Camera::model_view_projection_matrix() const {
		return projection_matrix() * model_view_matrix();
	}

	// -----------------------------------------------------------------
	// Change the matrices directly. This is useful if you want to temporally
	// change the view, e.g., grabbing a large snapshot from the framebuffer...
	void Camera::set_projection_matrix(const mat4& proj) {
		projection_matrix_ = proj;
	}


	void Camera::set_model_view_matrix(const mat4& mv) {
		model_view_matrix_ = mv;
	}

	/*! Sets the scene_radius() value. Invalid (non positive or near zero) values are
	ignored, with an error message.

	\attention The radius also sizes the clipping planes (see z_near() and z_far())
	and the eye distance computed by show_entire_scene(), so the cached projection
	matrix is invalidated. No other Camera state is derived from it. */
	void Camera::set_scene_radius(float radius) {
		if (radius <= epsilon<float>()) {
			LOG(ERROR) << "Scene radius must be positive (value is: " << radius << ")";
			return;
		}

		scene_radius_ = radius;
		projection_matrix_is_up_to_date_ = false;
	}

	/*! Similar to set_scene_radius() and set_scene_center(), but the scene limits are
	  defined by a (world axis aligned) bounding box. */
	void Camera::set_scene_bounding_box(const vec3 &min, const vec3 &max) {
		set_scene_center((min + max) / 2.0f);
		set_scene_radius(0.5f * (max - min).norm());
	}

	/*! Sets the scene_center().

	 \attention This method also sets the pivot_point() to scene_center(). */
	void Camera::set_scene_center(const vec3 &center) {
		scene_center_ = center;
		set_pivot_point(scene_center());
		projection_matrix_is_up_to_date_ = false;
	}


	/*! Changes the pivot_point() to \p point (defined in the world coordinate
	 * system). */
	void Camera::set_pivot_point(const vec3 &point) {
		const float prev_dist = std::fabs(camera_coordinates_of(pivot_point()).z);

		// If frame's RAP is set directly, projection_matrix_is_up_to_date_ should also be
		// set to 'false' to ensure proper re-computation of the ORTHO projection matrix.
		frame()->set_pivot_point(point);

		// ortho_coef_ is used to compensate for changes of the pivot_point, so that the
		// image does not change when the pivot_point is changed in ORTHOGRAPHIC mode.
		const float new_dist = std::fabs(camera_coordinates_of(pivot_point()).z);
		// Prevents division by zero when rap is set to camera position
		if ((prev_dist > 1E-9) && (new_dist > 1E-9))
			ortho_coef_ *= prev_dist / new_dist;
		projection_matrix_is_up_to_date_ = false;
	}

	/*! Returns the world length that projects to one logical screen pixel at \p position.

	 A line of \c n * world_pixel_ratio() world units, located at \p position in the
	 world coordinates system, will be projected with a length of \c n logical
	 pixels on screen (DPI-independent; screen_height() is logical on both backends).

	 Use this method to scale objects so that they have a constant logical pixel size
	 on screen. The following code will draw a 20 logical pixel line, starting at
	 scene_center() and always directed along the screen vertical direction: \code
	 glBegin(GL_LINES);
	 glVertex3fv(scene_center());
	 glVertex3fv(scene_center() + 20 * world_pixel_ratio(scene_center()) *
	 camera()->up_vector()); glEnd(); \endcode */
	float Camera::world_pixel_ratio(const vec3 &position) const {
		switch (type()) {
		case PERSPECTIVE:
			return 2.0f * std::fabs((frame()->coordinates_of(position)).z) *
				std::tan(field_of_view() / 2.0f) / static_cast<float>(screen_height());
		case ORTHOGRAPHIC: {
			float w(0), h(0);
			get_ortho_width_height(w, h);
			return 2.0f * h / static_cast<float>(screen_height());
		}
		}
		// Bad compilers complain
		return 1.0f;
	}

	/*! Changes the Camera field_of_view() so that the entire scene (defined by
	 scene_center() and scene_radius()) is visible from the
	 Camera position().

	 The position() and orientation() of the Camera are not modified and you first
	 have to orientate the Camera in order to actually see the scene (see look_at(),
	 show_entire_scene() or fit_sphere()).

	 This method is especially useful for \e shadow \e maps computation. Use the
	 Camera positioning tools (set_position(), look_at()) to position a Camera at the
	 light position. Then use this method to define the field_of_view() so that the
	 shadow map resolution is optimally used: \code
	 // The light camera needs size hints in order to optimize its field_of_view
	 light_camera->set_scene_radius(scene_radius());
	 light_camera->set_scene_center(scene_center());

	 // Place the light camera.
	 light_camera->set_position(light_frame->position());
	 light_camera->look_at(scene_center());
	 light_camera->set_fov_to_fit_scene();
	 \endcode

	 The snippet above is the typical use: a light Camera that shares the scene
	 bounds of the main Camera, so that its frustum covers exactly the geometry to be
	 shadowed.

	 \attention The field_of_view() is clamped to M_PI/2.0. This happens when the
	 Camera is at a distance lower than std::sqrt(2.0) * scene_radius() from the
	 scene_center(). It optimizes the shadow map resolution, although it may miss
	 some parts of the scene. */
	void Camera::set_fov_to_fit_scene() {
		if (distance_to_scene_center() > std::sqrt(2.0f) * scene_radius())
			set_field_of_view(2.0f * std::asin(scene_radius() / distance_to_scene_center()));
		else
			set_field_of_view(static_cast<float>(M_PI) / 2.0f);
	}

    /*! Makes the Camera smoothly move towards and look at the 3D point \p p.

     The end pose places the eye nine tenths of the way from the current
     position() towards \p p and oriented by look_at(), i.e. with \p p projected in
     the center of the window. The eye already being close to \p p keeps the
     rotation needed to face it small, which makes the move read as a zoom rather
     than as a turn. The interpolation takes half a second and replaces any
     interpolation in progress.

     See also interpolate_to_fit_scene() and look_at(). */
    void Camera::interpolate_to_look_at(const vec3 &p) {
        const float coef = 0.1f;
        const vec3& pos = coef*frame()->position() + (1.0f-coef)*p;
        const quat& ori = frame()->orientation();

        ManipulatedCameraFrame temp_frame;

#if 1 // Move the POI to screen center
        // Small hack: attach a temporary frame to take advantage of look_at without
        // modifying frame
        ManipulatedCameraFrame *const original_frame = frame();
        temp_frame.set_position_and_orientation(pos, ori);
        set_frame(&temp_frame);
        look_at(p);
        set_frame(original_frame);
#else
        temp_frame.set_position_and_orientation(pos, ori);
#endif
        interpolate_to(&temp_frame, 0.5);
    }

    /*! Interpolates the Camera along a half second KeyFrameInterpolator path so that
     the entire scene fits the screen at the end.

     The scene is defined by its scene_center() and its scene_radius(). See
     show_entire_scene(), which computes the end pose; like it, this method only
     translates the Camera, so the orientation() is preserved.

     See also interpolate_to_look_at(). */
    void Camera::interpolate_to_fit_scene() {
        // Small hack: attach a temporary frame to take advantage of look_at without
        // modifying frame
        ManipulatedCameraFrame temp_frame;
        ManipulatedCameraFrame *const original_frame = frame();
        temp_frame.set_position(frame()->position());
        temp_frame.set_orientation(frame()->orientation());
        set_frame(&temp_frame);
        show_entire_scene();
        set_frame(original_frame);

        interpolate_to(&temp_frame, 0.5);
    }

    /*! Smoothly interpolates the Camera on a KeyFrameInterpolator path so that it
      goes to \p fr.

      \p fr is expressed in world coordinates. \p duration is the time (in seconds)
      the move takes. The current Camera pose is recorded as the first keyframe and
      \p fr as the keyframe at \p duration, so the path always has two keyframes:
      keyframe_interpolator()->delete_path() is called first and any interpolation
      in progress is stopped.

      See also interpolate_to_look_at() and interpolate_to_fit_scene(). */
    void Camera::interpolate_to(const Frame *fr, float duration) {
        if (interpolation_kfi_->is_interpolation_started())
            interpolation_kfi_->stop_interpolation();

        interpolation_kfi_->delete_path();
        interpolation_kfi_->add_keyframe(*frame());
        interpolation_kfi_->add_keyframe(*fr, duration);
        interpolation_kfi_->start_interpolation();
    }


	/*! Moves the Camera so that the entire scene is visible.

	 Simply calls fit_sphere() on a sphere defined by scene_center() and
	 scene_radius().

	 You will typically use this method in Viewer::init() after you defined a new
	 scene_radius(). */
	void Camera::show_entire_scene() const { fit_sphere(scene_center(), scene_radius()); }

	/*! Moves the Camera so that its scene_center() is projected on the center of the
	 window. The orientation() and field_of_view() are unchanged.

	 Simply projects the current position on a line passing through scene_center().
	 See also show_entire_scene().*/
	void Camera::center_scene() const {
		frame()->project_on_line(scene_center(), view_direction());
	}

	/*! Sets the Camera orientation(), so that it looks at point \p target (defined
	 in the world coordinate system).

	 The Camera position() is not modified. Simply set_view_direction().

	 See also set_up_vector(), set_orientation(), show_entire_scene(), fit_sphere() and
	 fit_bounding_box(). */
	void Camera::look_at(const vec3 &target) const {
		set_view_direction(target - position());
	}

	/*! Moves the Camera so that the sphere defined by (\p center, \p radius) is
	 visible and fits in the frustum.

	 The Camera is simply translated to center the sphere in the screen and make it
	 fit the frustum. Its orientation() and its field_of_view() are unchanged.

	 You should therefore orientate the Camera before you call this method. See
	 look_at(), set_orientation() and set_up_vector(). */
	void Camera::fit_sphere(const vec3 &center, float radius) const {
		float distance = 0.0f;
		switch (type()) {
		case PERSPECTIVE: {
			const float yview = radius / std::sin(field_of_view() / 2.0f);
			const float xview = radius / std::sin(horizontal_field_of_view() / 2.0f);
			distance = std::max(xview, yview);
			break;
		}
		case ORTHOGRAPHIC: {
			distance = dot((center - pivot_point()), view_direction()) + (radius / ortho_coef_);
			break;
		}
		}
		vec3 new_pos(center - distance * view_direction());
		frame()->set_position_with_constraint(new_pos);
	}

	/*! Moves the Camera so that the (world axis aligned) bounding box (\p min, \p
	  max) is entirely visible, using fit_sphere(). */
	void Camera::fit_bounding_box(const vec3 &min, const vec3 &max) const {
        const float radius = distance(min, max) * 0.5f;
        const vec3& center = 0.5f * (min + max);
        fit_sphere(center, radius);
	}

	/*! Moves the Camera so that the rectangular screen region defined by \p
	  rectangle (pixel units, with origin in the upper left corner) fits the screen.

	  The Camera is translated (its orientation() is unchanged) so that \p rectangle
	  is entirely visible. Since the pixel coordinates only define a \e frustum in
	  3D, it's the intersection of this frustum with a plane (orthogonal to the
	  view_direction() and passing through the scene_center()) that is used to define
	  the 3D rectangle that is eventually fitted. */
	void Camera::fit_screen_region(int xmin, int ymin, int xmax, int ymax) const {
        if (xmin == xmax || ymin == ymax)
            return;

		const vec3& vd = view_direction();
		const float dist_to_plane = distance_to_scene_center();
		int cx = static_cast<int>((xmin + xmax) * 0.5);
		int cy = static_cast<int>((ymin + ymax) * 0.5);

		vec3 orig, dir;
		convert_click_to_line(cx, cy, orig, dir);
		const vec3& new_center = orig + dist_to_plane / dot(dir, vd) * dir;

		convert_click_to_line(xmin, cy, orig, dir);
		const vec3& point_x = orig + dist_to_plane / dot(dir, vd) * dir;

		convert_click_to_line(cx, ymin, orig, dir);
		const vec3& point_y = orig + dist_to_plane / dot(dir, vd) * dir;

		float distance = 0.0f;
		switch (type()) {
		case PERSPECTIVE: {
			const float dist_x =
				(point_x - new_center).norm() / std::sin(horizontal_field_of_view() / 2.0f);
			const float dist_y = (point_y - new_center).norm() / std::sin(field_of_view() / 2.0f);
			distance = std::max(dist_x, dist_y);
			break;
		}
		case ORTHOGRAPHIC: {
			const float dist = dot((new_center - pivot_point()), vd);
			//#CONNECTION# get_ortho_width_height
			const float dist_x = (point_x - new_center).norm() / ortho_coef_ /
				((aspect_ratio() < 1.0f) ? 1.0f : aspect_ratio());
			const float dist_y = (point_y - new_center).norm() / ortho_coef_ /
				((aspect_ratio() < 1.0f) ? 1.0f / aspect_ratio() : 1.0f);
			distance = dist + std::max(dist_x, dist_y);
			break;
		}
		}

		vec3 new_pos(new_center - distance * vd);
		frame()->set_position_with_constraint(new_pos);
	}

	/*! Rotates the Camera so that its up_vector() becomes \p up (defined in the
	 world coordinate system).

	 The Camera is rotated around an axis orthogonal to \p up and to the current
	 up_vector() direction. Use this method in order to define the Camera horizontal
	 plane.

	 When \p no_move is set to \c false, the orientation modification is compensated
	 by a translation, so that the pivot_point() stays projected at the same position
	 on screen. This is especially useful when the Camera is used as an observer of
	 the scene (default mouse binding).

	 When \p no_move is \c true (default), the Camera position() is left unchanged,
	 which is the intuitive behavior in a walk-through mode: the Camera turns in
	 place instead of re-framing the pivot point.

	 The rotation is applied to frame(), so up_vector() (which is derived from the
	 Camera orientation()) now returns \p up.

	 See also set_view_direction(), look_at() and set_orientation(). */
	void Camera::set_up_vector(const vec3 &up, bool no_move) const {
		quat q(vec3(0.0, 1.0, 0.0), frame()->transform_of(up));

		if (!no_move)
			frame()->set_position(pivot_point() -
			(frame()->orientation() * q)
				.rotate(frame()->coordinates_of(pivot_point())));

		frame()->rotate(q);
	}

	/*! Sets the orientation() of the Camera using polar coordinates.

	 \p theta rotates the Camera around its Y axis, and \e then \p phi rotates it
	 around its X axis. The polar coordinates are defined in the world coordinates
	 system: \p theta = \p phi = 0 means that the Camera is directed towards the
	 world Z axis. Both angles are expressed in radians.

	 See also set_up_vector(). The position() of the Camera is unchanged, you may want
	 to call show_entire_scene() after this method to move the Camera.

	 This method can be useful to render a panoramic sequence: sweep \p theta (and
	 optionally \p phi), drawing and grabbing a frame after each step, e.g. with
	 Viewer::capture(). */
	void Camera::set_orientation(float theta, float phi) const {
		vec3 axis(0.0f, 1.0f, 0.0f);
		const quat rot1(axis, theta);
		axis = vec3(-std::cos(theta), 0.0f, std::sin(theta));
		const quat rot2(axis, phi);
		set_orientation(rot1 * rot2);
	}

	/*! Sets the Camera orientation(), defined in the world coordinate system. */
	void Camera::set_orientation(const quat &q) const {
		frame()->set_orientation(q);
	}

	/*! Rotates the Camera so that its view_direction() is \p direction (defined in
	 the world coordinate system).

	 The Camera position() is not modified. The Camera is rotated so that the
	 horizon (defined by its up_vector()) is preserved. See also look_at() and
	 set_up_vector(). */
	void Camera::set_view_direction(const vec3 &direction) const {
		if (direction.length2() < 1E-10)
			return;

		vec3 x_axis = cross(direction, up_vector());
		if (x_axis.length2() < 1E-10)
		{
			// target is aligned with up_vector, this means a rotation around the X axis.
			// the X axis is then unchanged, let's keep it !
			x_axis = frame()->inverse_transform_of(vec3(1.0, 0.0, 0.0));
		}

		quat q;
		q.set_from_rotated_basis(x_axis, cross(x_axis, direction), -direction);
		frame()->set_orientation_with_constraint(q);
	}

	/*! Returns the Camera position (the eye), defined in the world coordinate
	system.

	Use set_position() to set the Camera position. Other convenient methods are
	show_entire_scene() or fit_sphere(). Actually returns \c frame()->position().

	This position corresponds to the projection center of a PERSPECTIVE
	Camera. It is not located in the image plane, which is at a z_near() distance
	ahead. */
	vec3 Camera::position() const { return frame()->position(); }

	/*! Returns the normalized up vector of the Camera, defined in the world
	coordinate system.

	Set using set_up_vector() or set_orientation(). It is orthogonal to view_direction()
	and to right_vector().

	It corresponds to the Y axis of the associated frame() (actually returns
	frame()->inverse_transform_of(vec3(0.0, 1.0, 0.0)) ). */
	vec3 Camera::up_vector() const {
		return frame()->inverse_transform_of(vec3(0.0, 1.0, 0.0));
	}
	/*! Returns the normalized view direction of the Camera, defined in the world
	coordinate system.

	Change this value using set_view_direction(), look_at() or set_orientation(). It is
	orthogonal to up_vector() and to right_vector().

	This corresponds to the negative Z axis of the frame() (
	frame()->inverse_transform_of(vec3(0.0, 0.0, -1.0)) ). */
	vec3 Camera::view_direction() const {
		return frame()->inverse_transform_of(vec3(0.0, 0.0, -1.0));
	}

	/*! Returns the normalized right vector of the Camera, defined in the world
	coordinate system.

	This vector lies in the Camera horizontal plane, directed along the X axis
	(orthogonal to up_vector() and to view_direction()). Set using set_up_vector(),
	look_at() or set_orientation().

	Simply returns frame()->inverse_transform_of(vec3(1.0, 0.0, 0.0)). */
	vec3 Camera::right_vector() const {
		return frame()->inverse_transform_of(vec3(1.0, 0.0, 0.0));
	}

	/*! Returns the Camera orientation, defined in the world coordinate system.

	Actually returns \c frame()->orientation(). Use set_orientation(), set_up_vector()
	or look_at() to set the Camera orientation. */
	quat Camera::orientation() const { return frame()->orientation(); }

	/*! Sets the Camera position() (the eye), defined in the world coordinate
	 * system. */
	void Camera::set_position(const vec3 &pos) const { frame()->set_position(pos); }

	/*! Returns the Camera frame coordinates of a point \p src defined in world
	coordinates.

	world_coordinates_of() performs the inverse transformation.

	Note that the point coordinates are simply converted in a different coordinate
	system. They are not projected on screen. Use projected_coordinates_of() for that.
	*/
	vec3 Camera::camera_coordinates_of(const vec3 &src) const {
		return frame()->coordinates_of(src);
	}

	/*! Returns the world coordinates of the point whose position \p src is defined
	in the Camera coordinate system.

	camera_coordinates_of() performs the inverse transformation. */
	vec3 Camera::world_coordinates_of(const vec3 &src) const {
		return frame()->inverse_coordinates_of(src);
	}

	/*! The point the Camera pivots around when the view is rotated with the mouse
	(ManipulatedCameraFrame::action_rotate()). Defined in the world coordinate system.

	Default value is the scene_center().

	\attention set_scene_center() changes this value. */
	vec3 Camera::pivot_point() const { return frame()->pivot_point(); }


	void Camera::set_from_model_view_matrix(const mat4& mv) const
	{
		// The upper-left 3x3 block of a rigid model view matrix is the rotation
		mat3 rot(mv);

		// Express it as the equivalent quaternion
		quat q;
		q.set_from_rotation_matrix(rot);

		set_orientation(q);
		// The last column is the Camera position expressed in the Camera CS, so undo
		// the rotation to get it back in world coordinates.
		set_position(-q.rotate(vec3(mv(0, 3), mv(1, 3), mv(2, 3))));
	}


    void Camera::set_from_calibration(float fx, float fy, float skew, float cx, float cy,
                                      const mat3 &R, const vec3 &t, bool convert)
    {
        // The two methods give the same results, both assuming that image_height equals to (2.0 * cy).
        // This is not accurate and the error seems mainly due to the inaccuracy in cy (because from calibration, cy
        // may not be exactly at the image center).
        // Suggestion: use the image height (usually known in practice) to compute the field of view.
#if 0
        const quat q(inverse(R));  // the inverse rotation represented by a quaternion
        if (convert)
            set_orientation(q);  // this already includes the conversion
        else {
            /// \attention The camera coordinates of computer vision goes X right, Y down, Z forward,
            ///               while the camera coordinates of OpenGL goes X right, Y up, Z inward.
            ///               So we need to invert both Y and Z axes.
            mat3 flip(1.0);
            flip(1, 1) = -1;   // invert the Y axis
            flip(2, 2) = -1;   // invert the Z axis
            set_orientation(quat(flip) * q);
        }

        set_position(-q.rotate(t));    // camera position: -inverse(rot) * t

        // http://ksimek.github.io/2013/06/18/calibrated-cameras-and-gluperspective/
        // https://github.com/opencv/opencv/blob/82f8176b0634c5d744d1a45246244291d895b2d1/modules/calib3d/src/calibration.cpp#L1820
        // We assume image_height = (2.0 * cy). However, cy may not be exactly at the image center.
        const float proj11 = 2.0f * fy / (2.0 * cy); // proj[1][1]
        // Liangliang: if we replace (2.0f * cy) by image_height, the result can be more accurate, i.e.,
        //const float proj11 = 2.0f * fy / image_height; // this will be more accurate
        
        const float fov = 2.0f * std::atan(1.0f / proj11);
        set_field_of_view(fov);

#else

        /**-------------------------------------------------------------------
         * It took me quite a while to figure out this.
         * The OpengGL projection and modelview matrices can be computed as follows.
         * Particle Filtering with Rendered Models: A Two Pass Approach to Multi-object 3D Tracking with the GPU)
         *      http://cvrr.ucsd.edu/publications/2008/MurphyChutorian_Trivedi_CVGPU08.pdf
         * More detailed explanation can be found here:
         *      http://ksimek.github.io/2013/06/03/calibrated_cameras_in_opengl/
         *-------------------------------------------------------------------*/
        //const mat4 proj(
        //                2.0 * fx / w,  -2.0 * skew / w,  1.0 - 2.0 * cx/w,          0.0,
        //                0.0,            2.0 * fy / h,   -1.0 + 2.0 * (cy + 1.0)/h,  0.0,
        //                0.0,            0.0,             (n + f) / (n - f),         2.0 * n * f / (n - f),
        //                0.0,            0.0,            -1.0,                       0.0
        //                );
        //
        mat34 Rt = mat34(1.0f) * mat4::translation(t) * mat4(R); // rotation first, then translation
        if (convert) {
            /// \attention The camera coordinates in computer vision goes X right, Y down, Z forward,
            ///               while the camera coordinates in OpenGL goes X right, Y up, Z inward.
            ///               So we need to invert both Y and Z axes.
#if 1
            mat3 flip(1.0);
            flip(1, 1) = -1;   // invert the Y axis
            flip(2, 2) = -1;   // invert the Z axis
            Rt = flip * Rt;
#else
            /// This can also be achieved by directly change the sign of the 2nd and 3rd rows.
            Rt.set_row(1, -Rt.row(1));
            Rt.set_row(2, -Rt.row(2));
#endif
        }
        const mat3 K(fx, skew, cx,
                     0, fy, cy,
                     0, 0, 1);
        set_from_calibration(K * Rt);
#endif
    }


	/*! Defines the Camera position(), orientation() and field_of_view() from a
	 calibration/projection matrix.

	 \p proj has to be given in the format used by vision algorithms. It has 3
	 lines and 4 columns. It transforms a point from the world homogeneous
	 coordinate system (4 coordinates: \c sx, \c sy, \c sz and \c s) into a point in
	 the screen homogeneous coordinate system (3 coordinates: \c sx, \c sy, and \c
	 s, where \c x and \c y are the pixel coordinates on the screen).

	 Its three lines correspond to the homogeneous coordinates of the normals to the
	 planes x=0, y=0 and z=0, defined in the Camera coordinate system.

	 \attention Passing the result of projection_matrix() or model_view_matrix() to
	 this method is not possible (purposefully incompatible matrix dimensions).
	 \p proj is more likely to be the product of these two matrices, without the last
	 line, i.e. the usual \c K * \c [R | t] decomposition of a calibrated camera.

	 Use set_from_model_view_matrix() to set position() and orientation() from a
	 model view matrix. field_of_view() can also be retrieved from a \e perspective
	 projection matrix using 2.0 * std::atan(1.0/projection_matrix[5]) -- which is
	 what the \c fy / \c cy entries of \p proj amount to here, see the notes below.

	 This code was written by Sylvain Paris. Modified and bug fixed by Liangliang Nan.
	 */
    void Camera::set_from_calibration(const mat34 &proj) {
		// The 3 lines of the matrix are the normals to the planes x=0, y=0, z=0
		// in the camera CS. As we normalize them, we do not need the 4th coordinate.
        vec3 line_0 = vec3(proj.row(0));	line_0.normalize();
        vec3 line_1 = vec3(proj.row(1));	line_1.normalize();
        vec3 line_2 = vec3(proj.row(2));	line_2.normalize();

		// The camera position is at (0,0,0) in the camera CS, so it is the
		// intersection of the 3 planes. It can be seen as the kernel
		// of the 3x4 projection matrix. We calculate it through 4 dimensional
		// vector product. We go directly into 3D that is to say we directly
		// divide the first 3 coordinates by the 4th one.

		// We derive the 4 dimensional vector product formula from the
		// computation of a 4x4 determinant that is developed according to
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
		column_0.normalize();

		// Y-axis is almost like line_1 but should be orthogonal to the Z axis.
		// Moreover, line_1 is downward oriented as the screen CS.
		vec3 column_1 = -cross(cross(column_2, line_1), column_2);
		column_1.normalize();

        const mat3 rot(column_0, column_1, column_2);

		// We compute the field of view.
        // The fov computation in this function assumes that image_height = (2.0 * cy). So it is equivalent to
        //      const float proj11 = 2.0f * fy / (2.0f * cy);
        //      const float fov = 2.0f * std::atan(1.0f / proj11);
        // However in practice, cy may not be exactly at the image center. To get an accurate fov, the actual image
        // height should be used.
        // TODO: add image size as argument to this function

		// cross(line_1, column_0) -> vector of intersection line between y_screen=0 and x_camera=0 plane.
		// column_2 * (...)  -> cos of the angle between Z vector and y_screen=0 plane
		// * 2 -> field of view = 2 * half angle
        vec3 intersection = cross(line_1, column_0); // vector of intersection line between y_screen=0 and x_camera=0 plane.
        intersection.normalize();
		const float fov = std::acos(dot(column_2, intersection)) * 2.0f;

		// We set the camera.
		quat q;
		q.set_from_rotation_matrix(rot);
		set_orientation(q);
		set_position(cam_pos);
		set_field_of_view(fov);
	}


	///////////////////////// Camera to world transform ///////////////////////

	/*! Returns the screen projected coordinates of a 3D point \p src defined in the \p
	 frame coordinate system (world coordinates when \p frame is \c NULL).

	 The result is in continuous logical window coordinates with the origin at the
	 upper-left corner; z is the normalized depth in [0,1] (near to far). The mapping
	 is shared with the viewers through transform::project(), so it stays consistent
	 with world_to_window() and picking on both backends.

	 Only the intrinsic Camera matrices and screen size are used; no graphics-context
	 state is read, so an offscreen Camera only needs its screen_width()/screen_height()
	 and its pose: the matrices are recomputed on demand.

	 unprojected_coordinates_of() performs the inverse transformation. */
    vec3 Camera::projected_coordinates_of(const vec3 &src, const Frame *frame) const {
        const int viewport[] = {0, 0, screen_width_, screen_height_};
        return transform::project(frame ? frame->inverse_coordinates_of(src) : src,
                                  model_view_projection_matrix(), viewport, false);
    }

	/*! Returns the world unprojected coordinates of a point \p src defined in the
	 screen coordinate system: continuous logical x/y with the origin at the
	 upper-left corner and a normalized depth z in [0,1] (near to far, non-linear in
	 distance for perspective cameras). No pixel-center offset is added; pass pixel
	 centers (x+0.5, y+0.5) when unprojecting a sampled framebuffer pixel.

	 The result is expressed in the \p frame coordinate system (world coordinates
	 when \p frame is \c NULL). The mapping is shared with the viewers through
	 transform::unproject(). For repeated queries with unchanged matrices, reuse a
	 precomputed inverse via transform::unproject_inverse().

	 projected_coordinates_of() performs the inverse transformation. */
    vec3 Camera::unprojected_coordinates_of(const vec3 &src, const Frame *frame) const {
        const int viewport[] = {0, 0, screen_width_, screen_height_};
        const vec3 point = transform::unproject(src, model_view_projection_matrix(), viewport, false);
        return frame ? frame->coordinates_of(point) : point;
    }


    void Camera::convert_click_to_line(int x, int y, vec3 &orig, vec3 &dir) const {
        const int viewport[] = {0, 0, screen_width_, screen_height_};
        const mat4 inv = transform::checked_inverse(model_view_projection_matrix());
        const vec3 near_point = transform::unproject_inverse(vec3(float(x), float(y), 0), inv, viewport, false);
        const vec3 inside = transform::unproject_inverse(vec3(float(x), float(y), .5f), inv, viewport, false);
        dir = normalize(inside - near_point);
        if (type() == PERSPECTIVE)
            orig = position();
        else
            orig = near_point + dir * (dot(position() - near_point, view_direction()) / dot(dir, view_direction()));
    }


	/*! Returns the 6 plane equations of the Camera frustum.

	 \p coef is filled with six 4-component vectors, in order: left, right, near,
	 far, top and bottom. The first three components are a unit normal pointing
	 \e out of the frustum and the fourth is the offset of the plane, so \c p lies on
	 the plane when \code a*p.x + b*p.y + c*p.z = d \endcode, and the frustum
	 interior is where the left-hand side is \e smaller than \c d for all six planes:
	 \code
	 bool inside = true;
	 for (int i = 0; i < 6 && inside; ++i)
	 inside = (coef[i][0]*p.x + coef[i][1]*p.y + coef[i][2]*p.z - coef[i][3]) <= 0.0f;
	 \endcode
	 This is the test to use for frustum culling (a bounding sphere of radius \c r
	 around \c p is outside as soon as the left-hand side exceeds \c r for one plane).

	 \attention The 4th component is \e not the \c d of the homogeneous equation
	 \c a*x+b*y+c*z+d = 0 (that form, with inward normals, is what
	 get_frustum_planes_coefficients2() returns). To install one of these planes as a
	 \c glClipPlane() clipping plane, which keeps the half-space where the homogeneous
	 equation is positive, negate the normal: \c {-coef[i][0], -coef[i][1],
	 -coef[i][2], coef[i][3]}.

	 Planes are defined in the world coordinate system and are computed from the
	 Camera parameters directly (see get_frustum_planes_coefficients2() for the
	 version derived from the model view projection matrix). */
	void Camera::get_frustum_planes_coefficients(float coef[6][4]) const
	{
		// Computed once and for all
		const vec3 pos = position();
		const vec3 view_dir = view_direction();
		const vec3 up = up_vector();
		const vec3 right = right_vector();
		const float pos_view_dir = dot(pos, view_dir);

		static vec3 normal[6];
		static float dist[6];

		switch (type())
		{
		case PERSPECTIVE:
		{
			const float hhfov = horizontal_field_of_view() / 2.0f;
			const float chhfov = std::cos(hhfov);
			const float shhfov = std::sin(hhfov);
			normal[0] = -shhfov * view_dir;
			normal[1] = normal[0] + chhfov * right;
			normal[0] = normal[0] - chhfov * right;

			normal[2] = -view_dir;
			normal[3] = view_dir;

			const float hfov = field_of_view() / 2.0f;
			const float chfov = std::cos(hfov);
			const float shfov = std::sin(hfov);
			normal[4] = -shfov * view_dir;
			normal[5] = normal[4] - chfov * up;
			normal[4] = normal[4] + chfov * up;

			for (int i = 0; i < 2; ++i)
				dist[i] = dot(pos, normal[i]);
			for (int j = 4; j < 6; ++j)
				dist[j] = dot(pos, normal[j]);

			// Natural equations are:
			// dist[0,1,4,5] = pos * normal[0,1,4,5];
			// dist[2] = (pos + z_near() * view_dir) * normal[2];
			// dist[3] = (pos + z_far()  * view_dir) * normal[3];

			// 2 times less computation using expanded/merged equations. Dir vectors are normalized.
			const float pos_right_cos_hh = chhfov * dot(pos, right);
			dist[0] = -shhfov * pos_view_dir;
			dist[1] = dist[0] + pos_right_cos_hh;
			dist[0] = dist[0] - pos_right_cos_hh;
			const float pos_up_cos_h = chfov * dot(pos, up);
			dist[4] = -shfov * pos_view_dir;
			dist[5] = dist[4] - pos_up_cos_h;
			dist[4] = dist[4] + pos_up_cos_h;

			break;
		}
		case ORTHOGRAPHIC:
			normal[0] = -right;
			normal[1] = right;
			normal[4] = up;
			normal[5] = -up;

			float hw, hh;
			get_ortho_width_height(hw, hh);
			dist[0] = dot((pos - hw * right), normal[0]);
			dist[1] = dot((pos + hw * right), normal[1]);
			dist[4] = dot((pos + hh * up), normal[4]);
			dist[5] = dot((pos - hh * up), normal[5]);
			break;
		}

		// Front and far planes are identical for both camera types.
		normal[2] = -view_dir;
		normal[3] = view_dir;
		dist[2] = -pos_view_dir - z_near();
		dist[3] = pos_view_dir + z_far();

		for (int i = 0; i < 6; ++i)
		{
			coef[i][0] = normal[i].x;
			coef[i][1] = normal[i].y;
			coef[i][2] = normal[i].z;
			coef[i][3] = dist[i];
		}
	}


	/*! Returns the 6 plane equations of the Camera frustum.
	 This is another implementation of get_frustum_planes_coefficients(...): the
	 planes are extracted from the model view projection matrix (the classic
	 Gribb-Hartmann combination of its rows) instead of being built from the Camera
	 vectors, and they are returned in the homogeneous form \code a*x + b*y + c*z + d
	 = 0 \endcode with \e inward normals, so that a point inside the frustum gives a
	 positive left-hand side for the six planes. The planes themselves are the same
	 (see the \attention note of get_frustum_planes_coefficients() about the
	 components of \p coef).
     See "Fast Extraction of Viewing Frustum Planes from the World-View-Projection Matrix" by Gil Gribb
     and Klaus Hartmann. 2001.*/
	void Camera::get_frustum_planes_coefficients2(float frustum[6][4]) const
	{
        // First, let's assume that we're going to store the frustum values in a variable like this:
        // float frustum[6][4];
        // That's six sets of four numbers (six planes, each with an A, B, C, and D value).
		const mat4& clip = model_view_projection_matrix();

		/* Extract the numbers for the LEFT plane */
		frustum[0][0] = clip[3] + clip[0];
		frustum[0][1] = clip[7] + clip[4];
		frustum[0][2] = clip[11] + clip[8];
		frustum[0][3] = clip[15] + clip[12];

		/* Normalize the result */
		float  t = std::sqrt(frustum[0][0] * frustum[0][0] + frustum[0][1] * frustum[0][1] + frustum[0][2] * frustum[0][2]);
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
		t = std::sqrt(frustum[1][0] * frustum[1][0] + frustum[1][1] * frustum[1][1] + frustum[1][2] * frustum[1][2]);
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
		t = std::sqrt(frustum[2][0] * frustum[2][0] + frustum[2][1] * frustum[2][1] + frustum[2][2] * frustum[2][2]);
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
		t = std::sqrt(frustum[3][0] * frustum[3][0] + frustum[3][1] * frustum[3][1] + frustum[3][2] * frustum[3][2]);
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
		t = std::sqrt(frustum[4][0] * frustum[4][0] + frustum[4][1] * frustum[4][1] + frustum[4][2] * frustum[4][2]);
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
		t = std::sqrt(frustum[5][0] * frustum[5][0] + frustum[5][1] * frustum[5][1] + frustum[5][2] * frustum[5][2]);
		frustum[5][0] /= t;
		frustum[5][1] /= t;
		frustum[5][2] /= t;
		frustum[5][3] /= t;
	}


    void Camera::modified() {
		projection_matrix_is_up_to_date_ = false;
		model_view_matrix_is_up_to_date_ = false;
        frame_modified.send();
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
	// Frustum culling is a VERY useful thing when it comes to 3D.
	// If you want a large world, there is no way you are going to send it down the
	// 3D pipeline every frame and let OpenGL take care of it for you.  That would
	// give you a 0.001 frame rate.  If you hit '+' and bring the sphere count up to
	// 1000, then take off culling, you will see the HUGE difference it makes.  
	// Also, you wouldn't really be rendering 1000 spheres.  You would most likely
	// use the sphere code for larger objects.  Let me explain.  Say you have a bunch
	// of objects, well... all you need to do is give the objects a radius, and then
	// test that radius against the frustum.  If that sphere is in the frustum, then you
	// render that object.  Also, you won't be rendering a high poly sphere, so it won't
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
	// 3) Now that we have the 2 matrices combined, we can abstract the sides of the frustum.
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
	// I want to thank Mark Morley for his tutorial on frustum culling. Most everything I got
	// here comes from his teaching.  If you want more in-depth, visit his tutorial at:
	//
	// http://www.markmorley.com/opengl/frustumculling.html
	// http://www.crownandcutlass.com/features/technicaldetails/frustum.html
	//
}

// \endcond