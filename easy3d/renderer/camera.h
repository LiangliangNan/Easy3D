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

#ifndef EASY3D_RENDERER_CAMERA_H
#define EASY3D_RENDERER_CAMERA_H


#include <easy3d/core/types.h>
#include "easy3d/util/signal.h"


namespace easy3d {


    class Frame;
    class ManipulatedCameraFrame;
    class KeyFrameInterpolator;

   /**
     * \brief A perspective or orthographic camera.
     * \class Camera easy3d/renderer/camera.h
     * \details A Camera defines some intrinsic parameters (field_of_view(), position(), view_direction(), up_vector()...)
     * and useful positioning tools that ease its placement (show_entire_scene(), fit_sphere(), look_at()...). It exports
     * its associated OpenGL projection and modelview matrices and can interactively be modified using the mouse.
     *
     * <h3>Mouse manipulation</h3>
     *
     * The position() and orientation() of the Camera are defined by a ManipulatedFrame (retrieved using frame()).
     * These methods are just convenient wrappers to the equivalent Frame methods. This also means that the Camera
     * frame() can be attached to a Frame::referenceFrame() which enables complex Camera setups.
     *
     * If a pivot_point() has been set, the Camera observes a scene and rotates around its pivot_point().
     *
     * <h3>Other functionalities</h3>
     *
     * The type() of the Camera can be Camera::ORTHOGRAPHIC or Camera::PERSPECTIVE (see Type()). field_of_view() is
     * meaningless with Camera::ORTHOGRAPHIC.
     *
     * The near and far planes of the Camera are fitted to the scene and determined from scene radius, scene center,
     * and z_clipping_coefficient() by the z_near() and z_far() methods. Reasonable values on the scene extends hence have
     * to be provided to the Camera to correctly display the scene. High level positioning methods also use this
     * information (show_entire_scene(), center_scene()...).
     *
     * A Camera owns a KeyFrameInterpolator (keyframe_interpolator()) that can be used to save Camera
     * positions and paths: call add_keyframe() on it to record a path and start_interpolation() to make
     * the Camera follow it. The Camera's own interpolate_to*() helpers use the same interpolator.
     *
     * Use camera_coordinates_of() and world_coordinates_of() to convert to and from the Camera frame() coordinate system.
     * projected_coordinates_of() and unprojected_coordinates_of() will convert from screen to 3D coordinates.
     * convert_click_to_line() is very useful for analytical object selection.
     *
     * A Camera can also be used outside a viewer or even without OpenGL for its coordinate system conversion
     * capabilities. Note however that some of them explicitly rely on the presence of a Z-buffer.
     *
     * <h3>Coordinate Systems</h3>
     *
     * Easy3D uses three coordinate systems. Understanding these is essential when working with camera operations:
     *
     * 1. <b>Logical Window Coordinates</b> (points, DPI-independent):
     *    - screen_width() and screen_height() return the logical window size
     *    - These match Viewer::width() and Viewer::height()
     *    - Used for camera operations, projection calculations, and coordinate conversions
     *
     * 2. <b>Framebuffer Coordinates</b> (physical pixels, DPI-dependent):
     *    - Related to logical size by: framebuffer_width = width * dpi_scaling()
     *    - Used for GPU operations, texture allocation, and viewport setup
     *    - Access via Viewer::framebuffer_width() and Viewer::framebuffer_height()
     *
     * 3. <b>Mouse/Event Coordinates</b>:
     *    - Always in logical window coordinates with top-left origin
     *    - Passed to camera methods like projected_coordinates_of()
     *
     * <b>When to use Camera vs Viewer methods:</b>
     * - Use Camera::screen_width()/screen_height() for camera operations, projection matrices, and coordinate conversions
     * - Use Viewer::width()/height() when working with UI, window events, or when you need the same logical size
     * - Use Viewer::framebuffer_width()/height() only for GPU operations that require physical pixel dimensions
     * - Camera screen dimensions are logical on both the OpenGL and Vulkan backends. For a single-view
     *   Viewer they equal Viewer::width()/height(); a MultiViewer's camera instead describes one cell,
     *   so its screen dimensions are the cell's logical size rather than the whole window.
     *
     * When the Camera is used through a Viewer (the usual case) the viewer owns it, keeps its screen
     * size in sync on resize and drives its frame from the mouse, so you only need to configure it,
     * typically in the viewer constructor:
     *      \code
     *          camera()->set_type(Camera::PERSPECTIVE);
     *          camera()->set_up_vector(vec3(0, 0, 1));        // Z pointing up
     *          camera()->set_view_direction(vec3(-1, 0, 0));  // X pointing out
     *          camera()->show_entire_scene();                 // position and orient the camera
     *      \endcode
     * A standalone Camera (e.g. an OffScreen render or a camera you animate yourself) additionally needs:
     *  - <code> set_screen_width_and_height() </code> at both
     *      - creation or initialization (i.e., before the viewer appears) of the application;
     *      - change of the window size.
     *  - <code> frame()->action_start() </code> on mouse down and
     *         <code> frame()->action_end() </code> on mouse up.
     *  - <code> frame()->action_rotate() </code> on mouse move for rotation and
     *         <code> frame()->action_translate() </code> on mouse move for translation and
     *         <code> frame()->action_zoom() </code> on mouse wheel for zoom
     *  - re-drawing whenever the pose changes: the viewer already listens to the frame_modified
     *        signal; elsewhere use <code> easy3d::connect(&camera->frame_modified, ...) </code>.
     *
     * To make the entire scene visible, call <code> camera_->set_scene_bounding_box() </code> and
     * <code> camera_->show_entire_scene() </code> ;
     *
     * To retrieve the model view projection matrix, call <code> camera_->model_view_projection_matrix() </code>
     */
    class Camera {
    public:
        /**
         * \brief Default constructor.
         */
        Camera();
        /**
         * \brief Destructor.
         */
        virtual ~Camera();

        /**
         * \brief Copy constructor.
         * \param camera The camera to copy.
         */
        Camera(const Camera &camera);
        
        /**
         * \brief Assignment operator.
         * \param camera The camera to assign.
         * \return A reference to this camera.
         */
        Camera &operator=(const Camera &camera);

        /**
         * \brief Enumerates the two possible types of Camera.
         * \details See type() and set_type(). This type mainly defines a different projection matrix
         *      (see compute_projection_matrix()). Many other methods (convert_click_to_line(),
         *      projected_coordinates_of(), world_pixel_ratio()...) are affected by this Type.
         */
        enum Type {
            PERSPECTIVE,    ///< Perspective camera.
            ORTHOGRAPHIC    ///< Orthographic camera.
        };

        /*! @name Position and orientation */
        //@{
    public:
        /**
         * \brief Get the position of the camera.
         * \return The position of the camera.
         */
        vec3 position() const;
        /**
         * \brief Get the up vector of the camera.
         * \return The up vector of the camera.
         */
        vec3 up_vector() const;
        /**
         * \brief Get the view direction of the camera.
         * \return The view direction of the camera.
         */
        vec3 view_direction() const;
        /**
         * \brief Get the right vector of the camera.
         * \return The right vector of the camera.
         */
        vec3 right_vector() const;
        /**
          * \brief Get the orientation of the camera.
          * \return The orientation of the camera.
          */
        quat orientation() const;

        // -----------------------------------------------------------------

        /**
         * \brief Sets the Camera's position() and orientation() from an OpenGL ModelView matrix.
         * \details This enables a Camera initialisation from another OpenGL application. After this method has been
         *      called, model_view_matrix() returns a matrix equivalent to \p mv (the frame is rebuilt from it, so the
         *      cached matrices are recomputed on the next query).
         *      Only the orientation() and position() of the Camera are modified: \p mv is expected to be a rigid
         *      modelview matrix.
         * \param mv The model view matrix.
         */
        void set_from_model_view_matrix(const mat4 &mv) const;
        /**
         * \brief Defines the position(), orientation() and field_of_view() of the camera from calibrated camera
         *      intrinsic and extrinsic parameters. This function assumes the camera parameters were obtained by
         *      standard camera calibration, in which image coordinates are denoted in pixels, with the origin point
         *      (0, 0) corresponding to the top-left corner of the image. The X axis starts at the left edge of an
         *      image and goes towards the right edge. The Y axis starts at the top of the image towards image bottom.
         *      All image pixels have non-negative coordinates.
         * \param fx The focal length in the x direction.
         * \param fy The focal length in the y direction.
         * \param skew The skew coefficient.
         * \param cx The principal point in the x direction.
         * \param cy The principal point in the y direction.
         * \param R The rotation matrix. It denotes the coordinate system transformation from 3D world coordinates
         *      to 3D camera coordinates.
         * \param t The translation vector. It is the position of the origin of the world coordinate system expressed
         *      in the camera coordinate system. \note t is often mistakenly considered the position of the camera.
         *      The position of the camera expressed in world coordinates is C = -inverse(R) * t = -transpose(R) * t.
         * \param convert True to convert from vision convention to OpenGL convention (i.e., invert Y and Z axes).
         *      This is because the camera coordinates of computer vision goes X right, Y down, Z forward, while the
         *      camera coordinates of OpenGL goes X right, Y up, Z inward.
         *  \sa set_from_calibration.
         */
        void set_from_calibration(float fx, float fy, float skew, float cx, float cy,
                                  const mat3 &R, const vec3 &t, bool convert = true);
        
        /**
         * \brief Defines the position(), orientation() and field_of_view() of the camera from calibrated camera
         *      intrinsic and extrinsic parameters. This function assumes the camera parameters were obtained by
         *      standard camera calibration, in which image coordinates are denoted in pixels, with the origin point
         *      (0, 0) corresponding to the top-left corner of the image. The X axis starts at the left edge of an
         *      image and goes towards the right edge. The Y axis starts at the top of the image towards image bottom.
         *      All image pixels have non-negative coordinates.
         * \param proj The projection matrix, which can be computed as P = K * M * [R|t], where R is a 3x3 matrix
         *      representing the camera rotation, and t is a 3D vector describing the camera translation (rotation
         *      first then translation).
         * \attention The additional M is a 3 by 3 matrix that converts the coordinate system from the vision
         *      convention to the OpenGL convention (i.e., inverting Y and Z axes), i.e.,
         *          M(1, 1) = -1;   // invert the Y axis
         *          M(2, 2) = -1;   // invert the Z axis
         *      This is because the camera coordinates in computer vision goes X right, Y down, Z forward, while the
         *      camera coordinates in OpenGL goes X right, Y up, Z inward.
         *  \sa set_from_calibration.
         */
        void set_from_calibration(const mat34 &proj);

        // -----------------------------------------------------------------

        /**
         * \brief Temporally change the projection matrix.
         * \param proj The projection matrix.
         * \attention This function directly changes the projection matrix. It actually doesn't change any of the camera
         *      parameters (i.e., position, orientation, fov, z_far, z_near, etc.). This is only useful if you want to
         *      temporally use a different projection matrix for rendering (e.g., grabbing a snapshot from the
         *      framebuffer).
         */
        void set_projection_matrix(const mat4 &proj);

        /**
         * \brief Temporally change the modelview matrix.
         * \param mv The modelview matrix.
         * \attention This function directly changes the modelview matrix. It actually doesn't change any of the camera
         *      parameters (i.e., position, orientation, fov, z_far, z_near, etc.). This is only useful if you want to
         *      temporally use a different modelview matrix for rendering (e.g., grabbing a snapshot from the
         *      framebuffer).
         */
        void set_model_view_matrix(const mat4 &mv);

        // -----------------------------------------------------------------

    public:
        /**
         * \brief Set the position of the camera.
         * \param pos The position of the camera.
         */
        void set_position(const vec3 &pos) const;
        /**
         * \brief Set the orientation of the camera.
         * \param q The orientation of the camera.
         */
        void set_orientation(const quat &q) const;
        /**
         * \brief Set the orientation of the camera.
         * \param theta The theta angle.
         * \param phi The phi angle.
         */
        void set_orientation(float theta, float phi) const;
        /**
         * \brief Set the up vector of the camera.
         * \param up The up vector.
         * \param no_move True to not move the camera.
         */
        void set_up_vector(const vec3 &up, bool no_move = true) const;
        /**
         * \brief Set the view direction of the camera.
         * \param direction The view direction.
         */
        void set_view_direction(const vec3 &direction) const;
        //@}

        /*! @name Positioning tools */
    public:
        /**
         * \brief Make the camera look at a target.
         * \details Only the orientation changes: this is set_view_direction(target - position()). Use
         *      center_scene() to also bring scene_center() to the middle of the screen.
         * \param target The target to look at.
         */
        void look_at(const vec3 &target) const;
        /**
         * \brief Show the entire scene.
         * \details Simply calls fit_sphere(scene_center(), scene_radius()): the Camera is moved along its
         *      view_direction() and its orientation() is unchanged. Set the scene bounds first with
         *      set_scene_bounding_box(), otherwise the default unit sphere at the world origin is used.
         * \sa center_scene() to bring scene_center() to the middle of the screen.
         */
        void show_entire_scene() const;
        /**
         * \brief Move the camera so that entire sphere fits the screen.
         * \details The Camera is translated along its view_direction() (orientation() unchanged) to the distance
         *      at which the sphere fills the screen for the current field_of_view() (perspective) or
         *      orthographic scale. scene_center()/scene_radius() are left unchanged.
         * \param center The center of the sphere.
         * \param radius The radius of the sphere.
         */
        void fit_sphere(const vec3 &center, float radius) const;
        /**
         * \brief Move the camera so that entire box fits the screen.
         * \details Convenience for fit_sphere() on the sphere that surrounds the axis-aligned box, whose center
         *      is (\p min + \p max) / 2 and whose radius is half the box diagonal. Corners are world coordinates.
         * \param min The minimum corner of the bounding box.
         * \param max The maximum corner of the bounding box.
         */
        void fit_bounding_box(const vec3 &min, const vec3 &max) const;
        /**
         * \brief Moves the Camera so that the rectangular screen region fits the screen.
         * \details The region is given by two opposite corners in continuous logical window coordinates,
         *      with the origin in the upper left corner (see screen_width() and screen_height()).
         *      The Camera is translated (its orientation() is unchanged) so that the region is entirely visible.
         *      Since the screen coordinates only define a \e frustum in 3D, it's the intersection of this frustum with
         *      a plane (orthogonal to the view_direction() and passing through the scene_center()) that is used to define
         *      the 3D rectangle that is eventually fitted.
         * \param xmin The minimum x coordinate.
         * \param ymin The minimum y coordinate.
         * \param xmax The maximum x coordinate.
         * \param ymax The maximum y coordinate.
         */
        void fit_screen_region(int xmin, int ymin, int xmax, int ymax) const;
        
        /**
         * \brief Moves the Camera so that its scene_center() is projected in the center of the screen.
         * \details This method simply projects the current position on a line passing through scene_center(). The
         *      orientation() and field_of_view() are unchanged.
         * \sa show_entire_scene()
         */
        void center_scene() const;

    public:
        /**
         * \brief Get the keyframe interpolator.
         * \details The Camera creates and owns this interpolator (it is released with the Camera and is never
         *      shared). set_frame() rebinds it to the new frame, so it always interpolates the Camera frame().
         * \return The keyframe interpolator.
         */
        KeyFrameInterpolator *keyframe_interpolator() const { return interpolation_kfi_; }

        /**
         * \brief Perform keyframe interpolation to look at a point.
         * \details The Camera is animated for half a second towards a pose that looks at \p point and is close
         *      to it (the eye is moved to 0.1 * position() + 0.9 * point), so that \p point ends up in the middle
         *      of the screen. The target pose is computed on a temporary frame and reached through
         *      interpolate_to(), so the interactive frame() is only driven by the interpolator.
         *      The Viewer calls this for 'Z' + left click on a visible point.
         * \param point The point to look at.
         */
        void interpolate_to_look_at(const vec3 &point);

        /**
         * \brief Perform keyframe interpolation to fit the scene.
         * \details Animates the Camera for half a second towards the show_entire_scene() pose of the current
         *      scene, keeping its orientation(); the target is computed on a temporary frame and reached
         *      through interpolate_to(). The Viewer calls this for 'Z' + right click.
         */
        void interpolate_to_fit_scene();

        /**
         * \brief Perform keyframe interpolation to a frame.
         * \details Replaces the interpolator path with the two keyframes (the current frame() and \p frame, the
         *      latter expressed in world coordinates) and starts the interpolation, which then takes \p duration
         *      seconds to reach \p frame.
         * \param frame The frame to interpolate to.
         * \param duration The duration of the interpolation.
         */
        void interpolate_to(const Frame* frame, float duration);

    public:
        /**
         * \brief Get the type of the camera.
         * \details A Camera::PERSPECTIVE Camera uses a classical projection mainly defined by its field_of_view().
         *      With a Camera::ORTHOGRAPHIC type(), the field_of_view() is meaningless and the width and height of the
         *      camera frustum are inferred from the distance to the pivot_point() using get_ortho_width_height(). Both
         *      types use z_near() and z_far() (to define their clipping planes) and aspect_ratio() (for frustum shape).
         * \return The type of the camera.   
         */
        Type type() const { return type_; }

        /**
         * \brief Returns the vertical field of view of the camera (in radians).
         * \details Value is set using set_field_of_view(). Default value is pi/4 radians. This value is meaningless if
         *      the Camera type() is Camera::ORTHOGRAPHIC.
         *      The field of view corresponds the one used in \c gluPerspective (see manual). It sets the Y (vertical)
         *      aperture of the Camera. The X (horizontal) angle is inferred from the window aspect ratio (see
         *      aspect_ratio() and horizontal_field_of_view()).
         *      Use set_fov_to_fit_scene() to adapt the field_of_view() to a given scene.
         * \return Vertical field of view in radians.
         */
        float field_of_view() const { return field_of_view_; }

        /**
         * \brief Returns the horizontal field of view of the camera (in radians).
         * \details Value is set using set_horizontal_field_of_view() or set_field_of_view(). These values are linked by:
         *   \code
         *      horizontal_field_of_view() = 2.0 * std::atan( std::tan(field_of_view()/2.0) * aspect_ratio() ).
         *   \endcode
         * \return The horizontal field of view in radians.
         */
        float horizontal_field_of_view() const {
            return 2.0f * std::atan(std::tan(field_of_view() / 2.0f) * aspect_ratio());
        }

        /**
         * \brief Returns the Camera aspect ratio defined by screen_width() / screen_height().
         * \details When the Camera is attached to a Viewer, these values and hence the aspect_ratio() are automatically
         *      fitted to the viewer's window aspect ratio using set_screen_width_and_height().
         * \return The aspect ratio.
         */
        float aspect_ratio() const {
            return static_cast<float>(screen_width_) / static_cast<float>(screen_height_);
        }

        /**
         * \brief Returns the width of the Camera screen in logical window coordinates (points).
         * \details This value represents the logical window size (DPI-independent), not physical pixels.
         *      It is kept in sync with the Viewer's logical width via set_screen_width_and_height().
         *      Use this for camera operations, projection calculations, and coordinate conversions.
         *      For physical pixel dimensions, use Viewer::framebuffer_width() instead.
         * \return The logical width in points (equivalent to Viewer::width()).
         * \see screen_height(), set_screen_width_and_height(), Viewer::width(), Viewer::framebuffer_width()
         */
        int screen_width() const { return screen_width_; }

        /**
         * \brief Returns the height of the Camera screen in logical window coordinates (points).
         * \details This value represents the logical window size (DPI-independent), not physical pixels.
         *      It is kept in sync with the Viewer's logical height via set_screen_width_and_height().
         *      Use this for camera operations, projection calculations, and coordinate conversions.
         *      For physical pixel dimensions, use Viewer::framebuffer_height() instead.
         * \return The logical height in points (equivalent to Viewer::height()).
         * \see screen_width(), set_screen_width_and_height(), Viewer::height(), Viewer::framebuffer_height()
         */
        int screen_height() const { return screen_height_; }

        /**
         * \brief Returns the world length that projects to one logical pixel at a position.
         * \param position The world-space position at which to evaluate the ratio.
         * \return World units per logical screen pixel (DPI-independent), derived
         *      from screen_height(). A segment of length n * world_pixel_ratio() at
         *      \p position projects to n logical pixels, so impostor radii computed
         *      as diameter * world_pixel_ratio() * 0.5 keep a constant logical size on both
         *      backends regardless of the framebuffer's physical pixel density.
         * \details The ratio depends on the camera type: for PERSPECTIVE it grows with the
         *      view-space depth of \p position, while for ORTHOGRAPHIC it is constant.
         */
        float world_pixel_ratio(const vec3 &position) const;

        /**
         * \brief Returns the coefficient which is used to set z_near() when the Camera is inside the sphere defined by
         *      scene_center() and z_clipping_coefficient() * scene_radius().
         * \details In that case, the z_near() value is set to z_near_coefficient() * z_clipping_coefficient() * scene_radius().
         *      See the z_near() documentation for details.
         *      Default value is 0.001, which is appropriate for most applications. In case you need a high dynamic
         *      ZBuffer precision, you can increase this value (~0.1).
         *      A lower value will prevent clipping of very close objects at the expense of the worst Z precision.
         *      Only meaningful when Camera type is Camera::PERSPECTIVE.
         * \return The z_near coefficient.
         */
        float z_near_coefficient() const { return z_near_coef_; }

        /**
         * \brief Returns the coefficient used to position the near and far clipping planes.
         * \details The near (resp. far) clipping plane is positioned at a distance equal to
         *      z_clipping_coefficient() * scene_radius() in front of (resp. behind) the scene_center(). This guarantees an
         *      optimal use of the z-buffer range and minimizes aliasing. See the z_near() and z_far() documentations.
         *      Default value is square root of 3.0 (so that a cube of size scene_radius() is not clipped).
         *      However, since the scene_radius() is used for other purposes (see show_entire_scene())
         *      you may want to change this value to define more precisely the location of the clipping planes.
         *      For a total control on clipping planes' positions, another option is to overload the z_near() and z_far()
         *      methods.
         * \attention When you visualize the Camera paths, set a larger value (suggested value is 5.0) so that the
         *      Camera paths are not clipped. Don't forget to restore the previous z_clipping_coefficient() value back
         *      when you leave this mode.
         * \return The z-clipping coefficient.
         * \sa z_near_coefficient().
         */
        float z_clipping_coefficient() const { return z_clipping_coef_; }

        /**
         * \brief Get the near clipping plane distance.
         * \return The near clipping plane distance.
         */
        virtual float z_near() const;
        /**
         * \brief Get the far clipping plane distance.
         * \return The far clipping plane distance.
         */
        virtual float z_far() const;
        /**
         * \brief Get the orthographic width and height.
         * \param half_width The half width.
         * \param half_height The half height.
         */
        virtual void get_ortho_width_height(float &half_width, float &half_height) const;
        /**
         * \brief Get the frustum planes coefficients.
         * \param coef The coefficients. The six 4-component vectors of \p coef respectively correspond to the left,
         *      right, near, far, top and bottom Camera frustum planes. The first three components are a unit normal
         *      pointing \e out of the frustum and the fourth is the plane offset, i.e. \c p is on the plane when
         *      \code a*p.x + b*p.y + c*p.z = d \endcode and inside when that holds with a \e smaller left-hand side
         *      for all six planes. Note that this \c d is the negation of the \c d of the homogeneous form
         *      \c a*x+b*y+c*z+d = 0 (which get_frustum_planes_coefficients2() uses).
         */
        void get_frustum_planes_coefficients(float coef[6][4]) const;

        /**
         * \brief Get the frustum planes coefficients (alternative version).
         * \param coef The coefficients. The six 4-component vectors of \p coef respectively correspond to the left,
         *      right, near, far, top and bottom Camera frustum planes. Each vector holds a plane equation of the form:
         *      \code a*x + b*y + c*z + d = 0 \endcode where \c a,\c b, \c c, and \c d are the 4 components of each
         *      vector, in that order. The unit normals point \e in, so a point inside the frustum makes the left-hand
         *      side positive; this is the form \c glClipPlane() expects. Same planes as
         *      get_frustum_planes_coefficients(), which instead uses outward normals and the natural equation.
         */
        void get_frustum_planes_coefficients2(float coef[6][4]) const;

    public:
        /**
         * \brief Set the type of the camera.
         * \param type The type of the camera.
         */
        void set_type(Type type);
        /**
         * \brief Set the field of view of the camera.
         * \param fov The field of view in radians.
         */
        void set_field_of_view(float fov);

        /**
         * \brief Sets the horizontal_field_of_view() of the Camera (in radians).
         * \details horizontal_field_of_view() and field_of_view() are linked by the aspect_ratio().
         *      This method actually calls set_field_of_view(( 2.0 * std::atan (std::tan(hfov / 2.0) / aspect_ratio()) ))
         *      so that a call to horizontal_field_of_view() returns the expected value.
         * \param hfov The horizontal field of view in radians.
         */
        void set_horizontal_field_of_view(float hfov) {
            set_field_of_view(2.0f * std::atan(std::tan(hfov / 2.0f) / aspect_ratio()));
        }
        /**
         * \brief Set the field of view to fit the scene.
         */
        void set_fov_to_fit_scene();

        /**
         * \brief Defines the Camera aspect_ratio().
         * \details This value is actually inferred from the screen_width() / screen_height() ratio.
         *      You should use set_screen_width_and_height() instead.
         *      This method might however be convenient when the Camera is not associated with a Viewer. It actually
         *      sets the screen_height() to 100 and the screen_width() accordingly. See also set_fov_to_fit_scene().
         * \param aspect The aspect ratio.  
         * \note If you absolutely need an aspect_ratio() that does not correspond to your viewer's window dimensions,
         *      override compute_projection_matrix(), or build a scaled diagonal matrix and install it with
         *      set_projection_matrix() in your viewer's draw() method.
         */
        void set_aspect_ratio(float aspect) {
            set_screen_width_and_height(int(100.0f * aspect), 100);
        }
        /**
         * \brief Set the screen width and height in logical window coordinates (points).
         * \details This method should be called when the viewer window is resized. The parameters represent
         *      the logical window size (DPI-independent), not physical pixels. The Camera's screen dimensions
         *      are kept in sync with the Viewer's logical size (Viewer::width() and Viewer::height()).
         * \param width The logical width in points (same as Viewer::width()).
         * \param height The logical height in points (same as Viewer::height()).
         * \see screen_width(), screen_height(), Viewer::width(), Viewer::height()
         */
        void set_screen_width_and_height(int width, int height);

        /**
         * \brief Set the z_near coefficient.
         * \param coef The z-near coefficient.
         */
        void set_z_near_coefficient(float coef) {
            z_near_coef_ = coef;
            projection_matrix_is_up_to_date_ = false;
        }

        /**
         * \brief Set the z-clipping coefficient.
         * \param coef The z-clipping coefficient.
         */
        void set_z_clipping_coefficient(float coef) {
            z_clipping_coef_ = coef;
            projection_matrix_is_up_to_date_ = false;
        }
        //@}

        /*! @name Scene radius and center */
        //@{
    public:
        /**
         * \brief Returns the radius of the scene observed by the Camera.
         * \details You need to provide such an approximation of the scene dimensions so that the Camera can adapt
         *      its z_near() and z_far() values. See the scene_center() documentation.
         * \return The radius of the scene.
         * \sa set_scene_bounding_box().
         */
        float scene_radius() const { return scene_radius_; }

        /**
         * \brief Returns the position of the scene center, defined in the world coordinate system.
         * \details The scene observed by the Camera should be roughly centered on this position, and included in a
         *      scene_radius() sphere. This approximate description of the scene permits a z_near() and z_far() clipping
         *      planes definition, and allows convenient positioning methods such as show_entire_scene().
         *      Default value is (0,0,0) (world origin). Use set_scene_center() to change it.
         * \return The center of the scene.
         * \sa set_scene_bounding_box().
         */
        vec3 scene_center() const { return scene_center_; }
        /**
         * \brief Get the distance to the scene center.
         * \return The distance to the scene center.
         */
        float distance_to_scene_center() const;

    public:
        /**
         * \brief Set the radius of the scene.
         * \param radius The radius of the scene.
         */
        void set_scene_radius(float radius);
        /**
         * \brief Set the center of the scene.
         * \param center The center of the scene.
         */
        void set_scene_center(const vec3 &center);

        /**
         * \brief Set the bounding box of the scene.
         * \details This method sets the scene center and radius from the bounding box.
         * \param min The minimum corner of the bounding box.
         * \param max The maximum corner of the bounding box.
         */
        void set_scene_bounding_box(const vec3 &min, const vec3 &max);
        //@}

    public:
        /**
         * \brief Set the pivot point of the camera. After this method is called, the Camera rotates around this point.
         * \param point The pivot point.
         */
        void set_pivot_point(const vec3 &point);
        /**
         * \brief Get the pivot point of the camera.
         * \return The pivot point.
         */
        vec3 pivot_point() const;

    public:
        /**
         * \brief Returns the ManipulatedCameraFrame attached to the Camera.
         * \details This frame defines the Camera position() and orientation() and translates mouse events into
         *      Camera displacement (the Viewer drives it for you). Its own Frame::referenceFrame() may be another
         *      frame, which enables nested camera setups. Set using set_frame().
         * \return The manipulated frame.
         */
        ManipulatedCameraFrame *frame() const { return frame_; }
        /**
         * \brief Set the manipulated frame of the camera.
         * \details Use this only when you derive ManipulatedCameraFrame and want your subclass to move the Camera:
         *      position()/orientation() and the positioning tools then follow the new frame. The Camera takes
         *      ownership of \p mcf from now on but does not delete the frame it replaces, so delete the previous
         *      frame() yourself if it is not shared. A \c NULL \p mcf is silently ignored.
         * \param mcf The manipulated frame.
         */
        void set_frame(ManipulatedCameraFrame *const mcf);
        //@}

    public:
        /**
         * \brief Compute the projection matrix.
         */
        void compute_projection_matrix();
        /**
         * \brief Compute the model view matrix.
         */
        void compute_model_view_matrix();
        /**
         * \brief Get the projection matrix.
         * \return The projection matrix.
         */
        const mat4 &projection_matrix() const;
        /**
         * \brief Get the model view matrix.
         * \return The model view matrix.
         */
        const mat4 &model_view_matrix() const;
        /**
         * \brief Get the model view projection matrix.
         * \return The model view projection matrix.
         */
        mat4 model_view_projection_matrix() const;

        /*! @name World to Camera coordinate systems conversions */
        //@{
    public:
        /**
         * \brief Convert world coordinates to camera coordinates.
         * \param src The source coordinates in world space.
         * \return The coordinates in camera space.
         */
        vec3 camera_coordinates_of(const vec3 &src) const;
        /**
         * \brief Convert camera coordinates to world coordinates.
         * \param src The source coordinates in camera space.
         * \return The coordinates in world space.
         */
        vec3 world_coordinates_of(const vec3 &src) const;
        //@}

        /*! @name 2D screen to 3D world coordinate systems conversions */
        //@{
    public:
        /**
         * \brief Returns the screen projected coordinates of a 3D point defined in the frame coordinate system.
         *      When \p frame in \c NULL (default), \p src is expressed in the world coordinate system.
         * \param src The source coordinates in world space.
         * \param frame The frame to use for the conversion.
         * \return Continuous logical coordinates (top-left origin) and normalized depth in [0, 1].
         */
        vec3 projected_coordinates_of(const vec3 &src, const Frame *frame = nullptr) const;
        /**
         * \brief Returns the world unprojected coordinates of a point defined in the screen coordinate system.
         *      The result is expressed in the \p frame coordinate system. When \p frame is \c NULL (default), the
         *      result is expressed in the world coordinate system.
         * \param src Continuous logical x/y (top-left origin) and normalized depth; no pixel-center offset is added.
         * \param frame The frame to use for the conversion.
         * \return The unprojected coordinates.
         */
        vec3 unprojected_coordinates_of(const vec3 &src, const Frame *frame = nullptr) const;

        /**
         * \brief Returns the coefficients of a 3D half-line passing through the Camera eye and pixel (x, y).
         * \details The origin of the half line (eye position) is stored in \p orig, while \p dir contains the properly
         *      oriented and normalized direction of the half line. \p x and \p y are expressed as the origin defined
         *      in the upper left corner, in continuous logical window units.
         * \param x The x coordinate of the click.
         * \param y The y coordinate of the click.
         * \param orig The origin of the line.
         * \param dir The direction of the line.
         */
        void convert_click_to_line(int x, int y, vec3 &orig, vec3 &dir) const;
        //@}

    private:
        /*! Invalidates the cached matrices (so they are recomputed on the next query) and emits
         *  frame_modified. Called by every setter that can change the projection or the pose. */
        void modified();

    private:
        // Pose. The Camera deletes it in its destructor; its 'modified' signal is forwarded to
        // Camera::modified() by set_frame(), which does not delete a replaced frame (the caller must).
        // Drives interactive manipulation and provides position()/orientation() (see frame()).
        ManipulatedCameraFrame *frame_;

        // Camera parameters
        int screen_width_, screen_height_; // logical window (or cell) size, DPI-independent
        float field_of_view_;              // vertical field of view, in radians
        vec3 scene_center_;                // world-space scene center, used to place the clipping planes
        float scene_radius_;               // world-space scene radius, in world units
        float z_near_coef_;                // fraction of scene_radius_ allowed in front of the eye
        float z_clipping_coef_;            // scene_radius_ multiplier placing the near/far planes
        float ortho_coef_;                 // orthographic scale: half-extent per unit view depth,
                                          // tracking field_of_view() and rescaled with the pivot distance
        Type type_;                        // PERSPECTIVE or ORTHOGRAPHIC
        mat4 model_view_matrix_;           // Buffered model view matrix.
        bool model_view_matrix_is_up_to_date_;  // false after a pose change until recomputed
        mat4 projection_matrix_;           // Buffered projection matrix.
        bool projection_matrix_is_up_to_date_;  // false after a parameter change until recomputed

        // Key frame interpolation. Owned by the Camera (created by the constructors, released by the
        // destructor); records and replays camera paths (see keyframe_interpolator()).
        KeyFrameInterpolator *interpolation_kfi_;

    public:
        /// Emitted by modified(): the pose or a parameter changed, so dependents should re-draw.
        Signal<> frame_modified;
    };

}

#endif // EASY3D_RENDERER_CAMERA_H
