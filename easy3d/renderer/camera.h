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
     * \details A Camera defines some intrinsic parameters (fieldOfView(), position(), viewDirection(), upVector()...)
     * and useful positioning tools that ease its placement (showEntireScene(), fitSphere(), lookAt()...). It exports
     * its associated OpenGL projection and modelview matrices and can interactively be modified using the mouse.
     *
     * <h3>Mouse manipulation</h3>
     *
     * The position() and orientation() of the Camera are defined by a ManipulatedFrame (retrieved using frame()).
     * These methods are just convenient wrappers to the equivalent Frame methods. This also means that the Camera
     * frame() can be attached to a Frame::referenceFrame() which enables complex Camera setups.
     *
     * If a pivotPoint() has been set, the Camera observes a scene and rotates around its pivotPoint().
     *
     * <h3>Other functionalities</h3>
     *
     * The type() of the Camera can be Camera::ORTHOGRAPHIC or Camera::PERSPECTIVE (see Type()). fieldOfView() is
     * meaningless with Camera::ORTHOGRAPHIC.
     *
     * The near and far planes of the Camera are fitted to the scene and determined from scene radius, scene center,
     * and zClippingCoefficient() by the zNear() and zFar() methods. Reasonable values on the scene extends hence have
     * to be provided to the Camera to correctly display the scene. High level positioning methods also use this
     * information (showEntireScene(), centerScene()...).
     *
     * A Camera holds KeyFrameInterpolator that can be used to save Camera positions and paths. You can interactively
     * addKeyFrameToPath() to a given path. Use playPath() to make the Camera follow the path.
     *
     * Use cameraCoordinatesOf() and worldCoordinatesOf() to convert to and from the Camera frame() coordinate system.
     * projectedCoordinatesOf() and unprojectedCoordinatesOf() will convert from screen to 3D coordinates.
     * convertClickToLine() is very useful for analytical object selection.
     *
     * A Camera can also be used outside a viewer or even without OpenGL for its coordinate system conversion
     * capabilities. Note however that some of them explicitly rely on the presence of a Z-buffer.
     *
     * To use the camera, you need to do the following:
     *  - Create and (if necessary) setup camera in the constructor of your viewer.
     *      \code
     *          camera_ = new Camera;
     *          camera_->setType(Camera::PERSPECTIVE);
     *          camera_->setUpVector(vec3(0, 0, 1));        // Z pointing up
     *          camera_->setViewDirection(vec3(-1, 0, 0));  // X pointing out
     *          camera_->showEntireScene();                 // position and orient the camera
     *          camera_->connect(this, &Viewer::update);    // connect the viewer's update function
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
         * \details See type() and setType(). This type mainly defines different Camera projection matrix (see
         *      loadProjectionMatrix()). Many other methods (convertClickToLine(), projectedCoordinatesOf(),
         *      pixelGLRatio()...) are affected by this Type.
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
        vec3 upVector() const;
        /**
         * \brief Get the view direction of the camera.
         * \return The view direction of the camera.
         */
        vec3 viewDirection() const;
        /**
         * \brief Get the right vector of the camera.
         * \return The right vector of the camera.
         */
        vec3 rightVector() const;
        /**
          * \brief Get the orientation of the camera.
          * \return The orientation of the camera.
          */
        quat orientation() const;

        // -----------------------------------------------------------------

        /**
         * \brief Sets the Camera's position() and orientation() from an OpenGL ModelView matrix.
         * \details This enables a Camera initialisation from another OpenGL application. After this method has been
         *      called, getModelViewMatrix() returns a matrix equivalent to \p mv.
         *      Only the orientation() and position() of the Camera are modified.
         * \param mv The model view matrix.
         */
        void set_from_model_view_matrix(const mat4 &mv) const;
        /**
         * \brief Defines the position(), orientation() and fieldOfView() of the camera from calibrated camera
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
         * \brief Defines the position(), orientation() and fieldOfView() of the camera from calibrated camera
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
         *      parameters (i.e., position, orientation, fov, zFar, zNear, etc.). This is only useful if you want to
         *      temporally use a different projection matrix for rendering (e.g., grabbing a snapshot from the
         *      framebuffer).
         */
        void set_projection_matrix(const mat4 &proj);

        /**
         * \brief Temporally change the modelview matrix.
         * \param mv The modelview matrix.
         * \attention This function directly changes the modelview matrix. It actually doesn't change any of the camera
         *      parameters (i.e., position, orientation, fov, zFar, zNear, etc.). This is only useful if you want to
         *      temporally use a different modelview matrix for rendering (e.g., grabbing a snapshot from the
         *      framebuffer).
         */
        void set_modelview_matrix(const mat4 &mv);

        // -----------------------------------------------------------------

    public:
        /**
         * \brief Set the position of the camera.
         * \param pos The position of the camera.
         */
        void setPosition(const vec3 &pos) const;
        /**
         * \brief Set the orientation of the camera.
         * \param q The orientation of the camera.
         */
        void setOrientation(const quat &q) const;
        /**
         * \brief Set the orientation of the camera.
         * \param theta The theta angle.
         * \param phi The phi angle.
         */
        void setOrientation(float theta, float phi) const;
        /**
         * \brief Set the up vector of the camera.
         * \param up The up vector.
         * \param noMove True to not move the camera.
         */
        void setUpVector(const vec3 &up, bool noMove = true) const;
        /**
         * \brief Set the view direction of the camera.
         * \param direction The view direction.
         */
        void setViewDirection(const vec3 &direction) const;
        //@}

        /*! @name Positioning tools */
    public:
        /**
         * \brief Make the camera look at a target.
         * \param target The target to look at.
         */
        void lookAt(const vec3 &target) const;
        /**
         * \brief Show the entire scene.
         */
        void showEntireScene() const;
        /**
         * \brief Move the camera so that entire sphere fits the screen.
         * \param center The center of the sphere.
         * \param radius The radius of the sphere.
         */
        void fitSphere(const vec3 &center, float radius) const;
        /**
         * \brief Move the camera so that entire box fits the screen.
         * \param min The minimum corner of the bounding box.
         * \param max The maximum corner of the bounding box.
         */
        void fitBoundingBox(const vec3 &min, const vec3 &max) const;
        /**
         * \brief Moves the Camera so that the rectangular screen region fits the screen.
         * \details The rectangular screen region is defined in pixel units, with origin in the upper left corner.
         *      The Camera is translated (its orientation() is unchanged) so that \p rectangle is entirely visible.
         *      Since the pixel coordinates only define a \e frustum in 3D, it's the intersection of this frustum with
         *      a plane (orthogonal to the viewDirection() and passing through the sceneCenter()) that is used to define
         *      the 3D rectangle that is eventually fitted.
         * \param xmin The minimum x coordinate.
         * \param ymin The minimum y coordinate.
         * \param xmax The maximum x coordinate.
         * \param ymax The maximum y coordinate.
         */
        void fitScreenRegion(int xmin, int ymin, int xmax, int ymax) const;
        
        /**
         * \brief Moves the Camera so that its sceneCenter() is projected in the center of the screen.
         * \details This method simply projects the current position on a line passing through sceneCenter(). The
         *      orientation() and fieldOfView() are unchanged.
         * \sa showEntireScene()
         */
        void centerScene() const;

    public:
        /**
         * \brief Get the keyframe interpolator.
         * \return The keyframe interpolator.
         */
        KeyFrameInterpolator *keyframe_interpolator() const { return interpolationKfi_; }

        /**
         * \brief Perform keyframe interpolation to look at a point.
         * \param point The point to look at.
         */
        void interpolateToLookAt(const vec3 &point);

        /**
         * \brief Perform keyframe interpolation to fit the scene.
         */
        void interpolateToFitScene();

        /**
         * \brief Perform keyframe interpolation to a frame.
         * \param frame The frame to interpolate to.
         * \param duration The duration of the interpolation.
         */
        void interpolateTo(const Frame* frame, float duration);

    public:
        /**
         * \brief Get the type of the camera.
         * \details A Camera::PERSPECTIVE Camera uses a classical projection mainly defined by its fieldOfView().
         *      With a Camera::ORTHOGRAPHIC type(), the fieldOfView() is meaningless and the width and height of the
         *      camera frustum are inferred from the distance to the pivotPoint() using getOrthoWidthHeight(). Both
         *      types use zNear() and zFar() (to define their clipping planes) and aspectRatio() (for frustum shape).
         * \return The type of the camera.   
         */
        Type type() const { return type_; }

        /**
         * \brief Returns the vertical field of view of the camera (in radians).
         * \details Value is set using setFieldOfView(). Default value is pi/4 radians. This value is meaningless if
         *      the Camera type() is Camera::ORTHOGRAPHIC.
         *      The field of view corresponds the one used in \c gluPerspective (see manual). It sets the Y (vertical)
         *      aperture of the Camera. The X (horizontal) angle is inferred from the window aspect ratio (see
         *      aspectRatio() and horizontalFieldOfView()).
         *      Use setFOVToFitScene() to adapt the fieldOfView() to a given scene.
         */
        float fieldOfView() const { return fieldOfView_; }

        /**
         * \brief Returns the horizontal field of view of the camera (in radians).
         * \details Value is set using setHorizontalFieldOfView() or setFieldOfView(). These values are linked by:
         *   \code
         *      horizontalFieldOfView() = 2.0 * std::atan( std::tan(fieldOfView()/2.0) * aspectRatio() ).
         *   \endcode
         * \return The horizontal field of view in radians.
         */
        float horizontalFieldOfView() const {
            return 2.0f * std::atan(std::tan(fieldOfView() / 2.0f) * aspectRatio());
        }

        /**
         * \brief Returns the Camera aspect ratio defined by screenWidth() / screenHeight().
         * \details When the Camera is attached to a Viewer, these values and hence the aspectRatio() are automatically
         *      fitted to the viewer's window aspect ratio using setScreenWidthAndHeight().
         * \return The aspect ratio.
         */
        float aspectRatio() const {
            return static_cast<float>(screenWidth_) / static_cast<float>(screenHeight_);
        }

        /**
         * \brief Returns the width (in pixels) of the Camera screen.
         * \details Set using setScreenWidthAndHeight(). This value is automatically fitted to the Viewer's window
         *      dimensions when the Camera is attached to a viewer.
         * \return The width in pixels.
         */
        int screenWidth() const { return screenWidth_; }

        /**
         * \brief Returns the height (in pixels) of the Camera screen.
         * \details Set using setScreenWidthAndHeight(). This value is automatically fitted to the viewer's window
         *      dimensions when the Camera is attached to a Viewer.
         * \return The height in pixels.
         */
        int screenHeight() const { return screenHeight_; }

        /**
         * \brief Get the pixel GL ratio at a position.
         * \param position The position.
         * \return The pixel GL ratio.
         */
        float pixelGLRatio(const vec3 &position) const;

        /**
         * \brief Returns the coefficient which is used to set zNear() when the Camera is inside the sphere defined by
         *      sceneCenter() and zClippingCoefficient() * sceneRadius().
         * \details In that case, the zNear() value is set to zNearCoefficient() * zClippingCoefficient() * sceneRadius().
         *      See the zNear() documentation for details.
         *      Default value is 0.001, which is appropriate for most applications. In case you need a high dynamic
         *      ZBuffer precision, you can increase this value (~0.1).
         *      A lower value will prevent clipping of very close objects at the expense of the worst Z precision.
         *      Only meaningful when Camera type is Camera::PERSPECTIVE.
         * \return The zNear coefficient.
         */
        float zNearCoefficient() const { return zNearCoef_; }

        /**
         * \brief Returns the coefficient used to position the near and far clipping planes.
         * \details The near (resp. far) clipping plane is positioned at a distance equal to
         *      zClippingCoefficient() * sceneRadius() in front of (resp. behind) the sceneCenter(). This guarantees an
         *      optimal use of the z-buffer range and minimizes aliasing. See the zNear() and zFar() documentations.
         *      Default value is square root of 3.0 (so that a cube of size sceneRadius() is not clipped).
         *      However, since the sceneRadius() is used for other purposes (see showEntireScene(), flySpeed(), ...)
         *      and you may want to change this value to define more precisely the location of the clipping planes.
         *      For a total control on clipping planes' positions, another option is to overload the zNear() and zFar()
         *      methods.
         * \attention When you visualize the Camera paths, set a larger value (suggested value is 5.0) so that the
         *      Camera paths are not clipped. Don't forget to restore the previous zClippingCoefficient() value back
         *      when you leave this mode.
         * \return The zClipping coefficient.
         * \sa zNearCoefficient().
         */
        float zClippingCoefficient() const { return zClippingCoef_; }

        /**
         * \brief Get the near clipping plane distance.
         * \return The near clipping plane distance.
         */
        virtual float zNear() const;
        /**
         * \brief Get the far clipping plane distance.
         * \return The far clipping plane distance.
         */
        virtual float zFar() const;
        /**
         * \brief Get the orthographic width and height.
         * \param halfWidth The half width.
         * \param halfHeight The half height.
         */
        virtual void getOrthoWidthHeight(float &halfWidth, float &halfHeight) const;
        /**
         * \brief Get the frustum planes coefficients.
         * \param coef The coefficients. The six 4-component vectors of \p coef respectively correspond to the left,
         *      right, near, far, top and bottom Camera frustum planes. Each vector holds a plane equation of the form:
         *      \code a*x + b*y + c*z + d = 0 \endcode where \c a,\c b, \c c， and \c d are the 4 components of each
         *      vector, in that order.
         */
        void getFrustumPlanesCoefficients(float coef[6][4]) const;

        /**
         * \brief Get the frustum planes coefficients (alternative version).
         * \param coef The coefficients. The six 4-component vectors of \p coef respectively correspond to the left,
         *      right, near, far, top and bottom Camera frustum planes. Each vector holds a plane equation of the form:
         *      \code a*x + b*y + c*z + d = 0 \endcode where \c a,\c b, \c c， and \c d are the 4 components of each
         *      vector, in that order.
         */
        void getFrustumPlanesCoefficients2(float coef[6][4]) const;

    public:
        /**
         * \brief Set the type of the camera.
         * \param type The type of the camera.
         */
        void setType(Type type);
        /**
         * \brief Set the field of view of the camera.
         * \param fov The field of view in radians.
         */
        void setFieldOfView(float fov);

        /**
         * \brief Sets the horizontalFieldOfView() of the Camera (in radians).
         * \details horizontalFieldOfView() and fieldOfView() are linked by the aspectRatio().
         *      This method actually calls setFieldOfView(( 2.0 * std::atan (std::tan(hfov / 2.0) / aspectRatio()) ))
         *      so that a call to horizontalFieldOfView() returns the expected value.
         * \param hfov The horizontal field of view in radians.
         */
        void setHorizontalFieldOfView(float hfov) {
            setFieldOfView(2.0f * std::atan(std::tan(hfov / 2.0f) / aspectRatio()));
        }
        /**
         * \brief Set the field of view to fit the scene.
         */
        void setFOVToFitScene();

        /**
         * \brief Defines the Camera aspectRatio().
         * \details This value is actually inferred from the screenWidth() / screenHeight() ratio.
         *      You should use setScreenWidthAndHeight() instead.
         *      This method might however be convenient when the Camera is not associated with a Viewer. It actually
         *      sets the screenHeight() to 100 and the screenWidth() accordingly. See also setFOVToFitScene().
         * \param aspect The aspect ratio.  
         * \note If you absolutely need an aspectRatio() that does not correspond to your viewer's window dimensions,
         *      overload loadProjectionMatrix() or multiply the created OpenGL projection matrix by a scaled diagonal
         *      matrix in your viewer's draw() method.
         */
        void setAspectRatio(float aspect) {
            setScreenWidthAndHeight(int(100.0f * aspect), 100);
        }
        /**
         * \brief Set the screen width and height.
         * \param width The width in pixels.
         * \param height The height in pixels.
         */
        void setScreenWidthAndHeight(int width, int height);

        /**
         * \brief Set the zNear coefficient.
         * \param coef The zNear coefficient.
         */
        void setZNearCoefficient(float coef) {
            zNearCoef_ = coef;
            projectionMatrixIsUpToDate_ = false;
        }

        /**
         * \brief Set the zClipping coefficient.
         * \param coef The zClipping coefficient.
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
         *      its zNear() and zFar() values. See the sceneCenter() documentation.
         * \return The radius of the scene.
         * \sa setSceneBoundingBox().
         */
        float sceneRadius() const { return sceneRadius_; }

        /**
         * \brief Returns the position of the scene center, defined in the world coordinate system.
         * \details The scene observed by the Camera should be roughly centered on this position, and included in a
         *      sceneRadius() sphere. This approximate description of the scene permits a zNear() and zFar() clipping
         *      planes definition, and allows convenient positioning methods such as showEntireScene().
         *      Default value is (0,0,0) (world origin). Use setSceneCenter() to change it.
         * \return The center of the scene.
         * \sa setSceneBoundingBox().
         */
        vec3 sceneCenter() const { return sceneCenter_; }
        /**
         * \brief Get the distance to the scene center.
         * \return The distance to the scene center.
         */
        float distanceToSceneCenter() const;

    public:
        /**
         * \brief Set the radius of the scene.
         * \param radius The radius of the scene.
         */
        void setSceneRadius(float radius);
        /**
         * \brief Set the center of the scene.
         * \param center The center of the scene.
         */
        void setSceneCenter(const vec3 &center);

        /**
         * \brief Set the bounding box of the scene.
         * \details This method sets the scene center and radius from the bounding box.
         * \param min The minimum corner of the bounding box.
         * \param max The maximum corner of the bounding box.
         */
        void setSceneBoundingBox(const vec3 &min, const vec3 &max);
        //@}

    public:
        /**
         * \brief Set the pivot point of the camera. After this method is called, the Camera rotates around this point.
         * \param point The pivot point.
         */
        void setPivotPoint(const vec3 &point);
        /**
         * \brief Get the pivot point of the camera.
         * \return The pivot point.
         */
        vec3 pivotPoint() const;

    public:
        /**
         * \brief Returns the ManipulatedFrame attached to the Camera.
         * \details This ManipulatedFrame defines its position() and orientation() and can translate mouse events into
         *      Camera displacement. Set using setFrame().
         * \return The manipulated frame.
         */
        ManipulatedCameraFrame *frame() const { return frame_; }
        /**
         * \brief Set the manipulated frame of the camera.
         * \param mcf The manipulated frame.
         */
        void setFrame(ManipulatedCameraFrame *const mcf);
        //@}

    public:
        /**
         * \brief Compute the projection matrix.
         */
        void computeProjectionMatrix();
        /**
         * \brief Compute the model view matrix.
         */
        void computeModelViewMatrix();
        /**
         * \brief Get the projection matrix.
         * \return The projection matrix.
         */
        const mat4 &projectionMatrix() const;
        /**
         * \brief Get the model view matrix.
         * \return The model view matrix.
         */
        const mat4 &modelViewMatrix() const;
        /**
         * \brief Get the model view projection matrix.
         * \return The model view projection matrix.
         */
        mat4 modelViewProjectionMatrix() const;

        /*! @name World to Camera coordinate systems conversions */
        //@{
    public:
        /**
         * \brief Convert world coordinates to camera coordinates.
         * \param src The source coordinates in world space.
         * \return The coordinates in camera space.
         */
        vec3 cameraCoordinatesOf(const vec3 &src) const;
        /**
         * \brief Convert camera coordinates to world coordinates.
         * \param src The source coordinates in camera space.
         * \return The coordinates in world space.
         */
        vec3 worldCoordinatesOf(const vec3 &src) const;
        //@}

        /*! @name 2D screen to 3D world coordinate systems conversions */
        //@{
    public:
        /**
         * \brief Returns the screen projected coordinates of a 3D point defined in the frame coordinate system.
         *      When \p frame in \c NULL (default), \p src is expressed in the world coordinate system.
         * \param src The source coordinates in world space.
         * \param frame The frame to use for the conversion.
         * \return The coordinates in screen space.
         */
        vec3 projectedCoordinatesOf(const vec3 &src, const Frame *frame = nullptr) const;
        /**
         * \brief Returns the world unprojected coordinates of a point defined in the screen coordinate system.
         *      The result is expressed in the \p frame coordinate system. When \p frame is \c NULL (default), the
         *      result is expressed in the world coordinate system.
         * \param src The source coordinates in screen space.
         * \param frame The frame to use for the conversion.
         * \return The unprojected coordinates.
         */
        vec3 unprojectedCoordinatesOf(const vec3 &src, const Frame *frame = nullptr) const;

        /**
         * \brief Returns the coefficients of a 3D half-line passing through the Camera eye and pixel (x, y).
         * \details The origin of the half line (eye position) is stored in \p orig, while \p dir contains the properly
         *      oriented and normalized direction of the half line. \p x and \p y are expressed as the origin defined
         *      in the upper left corner. Use screenHeight() - y - 1 to convert to OpenGL units.
         * \param x The x coordinate of the click.
         * \param y The y coordinate of the click.
         * \param orig The origin of the line.
         * \param dir The direction of the line.
         */
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
        vec3 sceneCenter_;
        float sceneRadius_; // OpenGL units
        float zNearCoef_;
        float zClippingCoef_;
        float orthoCoef_;
        Type type_;                 // PERSPECTIVE or ORTHOGRAPHIC
        mat4 modelViewMatrix_;        // Buffered model view matrix.
        bool modelViewMatrixIsUpToDate_;
        mat4 projectionMatrix_;        // Buffered projection matrix.
        bool projectionMatrixIsUpToDate_;

        // Key frame interpolation
        KeyFrameInterpolator *interpolationKfi_;

    public:
        /// A signal indicating the frame has been modified.
        Signal<> frame_modified;
    };

}

#endif // EASY3D_RENDERER_CAMERA_H
