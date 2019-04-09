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


#include <easy3d/camera.h>

#include <algorithm>

#include <easy3d/manipulated_camera_frame.h>
#include <easy3d/viewer.h>


namespace easy3d {


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
        projectionMatrix_.load_zero(); // computeProjectionMatrix() is lazy and assumes 0.0 almost
        computeProjectionMatrix();

        // position and orient the camera
        showEntireScene();
    }


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
        projectionMatrix_.load_zero(); // computeProjectionMatrix() is

        (*this) = camera;
    }


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


    void Camera::setScreenWidthAndHeight(int width, int height) {
        // Prevent negative and zero dimensions that would cause divisions by zero.
        screenWidth_ = width > 0 ? width : 1;
        screenHeight_ = height > 0 ? height : 1;
        projectionMatrixIsUpToDate_ = false;
    }


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


    float Camera::zFar() const {
        return distanceToSceneCenter() + zClippingCoefficient() * sceneRadius();
    }


    void Camera::setFieldOfView(float fov) {
        fieldOfView_ = fov;
        projectionMatrixIsUpToDate_ = false;
    }


    void Camera::setType(Type type) {
        if ((type == Camera::ORTHOGRAPHIC) && (type_ == Camera::PERSPECTIVE))
            orthoCoef_ = tan(fieldOfView() / 2.0f);
        type_ = type;
        projectionMatrixIsUpToDate_ = false;
    }


    void Camera::setFrame(ManipulatedCameraFrame *const mcf) {
        if (!mcf)
            return;

        frame_ = mcf;
        frame_->addObserver(this);

        onFrameModified();
    }


    float Camera::distanceToSceneCenter() const {
        return std::fabs((frame()->coordinatesOf(sceneCenter())).z);
    }

    void Camera::getOrthoWidthHeight(float &halfWidth, float &halfHeight) const {
        const float dist = orthoCoef_ * std::fabs(cameraCoordinatesOf(sceneCenter()).z);
        halfWidth = dist * ((aspectRatio() < 1.0f) ? 1.0f : aspectRatio());
        halfHeight = dist * ((aspectRatio() < 1.0f) ? 1.0f / aspectRatio() : 1.0f);
    }

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


    const mat4& Camera::projectionMatrix() const
    {
        // May not be needed, but easier like this.
        const_cast<Camera*>(this)->computeProjectionMatrix();
        return projectionMatrix_;
    }


    const mat4& Camera::modelViewMatrix() const
    {
        // May not be needed, but easier like this.
        const_cast<Camera*>(this)->computeModelViewMatrix();
        return modelViewMatrix_;
    }


    mat4 Camera::modelViewProjectionMatrix() const {
        return projectionMatrix() * modelViewMatrix();
    }


    void Camera::setSceneRadius(float radius) {
        if (radius <= 0.0f) {
            std::cerr << "Scene radius must be positive - Ignoring value" << std::endl;
            return;
        }

        sceneRadius_ = radius;
        projectionMatrixIsUpToDate_ = false;
    }


    void Camera::setSceneBoundingBox(const vec3 &min, const vec3 &max) {
        setSceneCenter((min + max) / 2.0f);
        setSceneRadius(0.5f * (max - min).norm());
    }


    void Camera::setSceneCenter(const vec3 &center) {
        sceneCenter_ = center;
        projectionMatrixIsUpToDate_ = false;
    }

    void Camera::setFOVToFitScene() {
        if (distanceToSceneCenter() > sqrt(2.0f) * sceneRadius())
            setFieldOfView(2.0f * asin(sceneRadius() / distanceToSceneCenter()));
        else
            setFieldOfView(static_cast<float>(M_PI) / 2.0f);
    }

    void Camera::showEntireScene() { fitSphere(sceneCenter(), sceneRadius()); }


    void Camera::lookAt(const vec3 &target) {
        setViewDirection(target - position());
    }


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
            distance = dot((center - sceneCenter()), viewDirection()) + (radius / orthoCoef_);
            break;
        }
        }
        vec3 newPos(center - distance * viewDirection());
        frame()->setPositionWithConstraint(newPos);
    }


    void Camera::fitBoundingBox(const vec3 &min, const vec3 &max) {
        float diameter = std::max(std::fabs(max[1] - min[1]), std::fabs(max[0] - min[0]));
        diameter = std::max(std::fabs(max[2] - min[2]), diameter);
        fitSphere(0.5f * (min + max), 0.5f * diameter);
    }


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
            const float dist = dot((newCenter - sceneCenter()), vd);
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


    void Camera::setUpVector(const vec3 &up, bool noMove) {
        quat q(vec3(0.0, 1.0, 0.0), frame()->transformOf(up));

        if (!noMove)
            frame()->setPosition(sceneCenter() -
            (frame()->orientation() * q)
                .rotate(frame()->coordinatesOf(sceneCenter())));

        frame()->rotate(q);
    }


    void Camera::setOrientation(float theta, float phi) {
        vec3 axis(0.0f, 1.0f, 0.0f);
        const quat rot1(axis, theta);
        axis = vec3(-cos(theta), 0.0f, sin(theta));
        const quat rot2(axis, phi);
        setOrientation(rot1 * rot2);
    }


    void Camera::setOrientation(const quat &q) {
        frame()->setOrientation(q);
    }


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


    vec3 Camera::position() const { return frame()->position(); }


    vec3 Camera::upVector() const {
        return frame()->inverseTransformOf(vec3(0.0, 1.0, 0.0));
    }


    vec3 Camera::viewDirection() const {
        return frame()->inverseTransformOf(vec3(0.0, 0.0, -1.0));
    }


    vec3 Camera::rightVector() const {
        return frame()->inverseTransformOf(vec3(1.0, 0.0, 0.0));
    }


    quat Camera::orientation() const { return frame()->orientation(); }


    void Camera::setPosition(const vec3 &pos) { frame()->setPosition(pos); }


    vec3 Camera::cameraCoordinatesOf(const vec3 &src) const {
        return frame()->coordinatesOf(src);
    }


    vec3 Camera::worldCoordinatesOf(const vec3 &src) const {
        return frame()->inverseCoordinatesOf(src);
    }


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
    
    
    void Camera::set_from_projection_matrix(const mat34 &proj) {
        // The 3 lines of the matrix are the normals to the planes x=0, y=0, z=0
        // in the camera CS. As we normalize them, we do not need the 4th coordinate.
        vec3 line_0(proj(0, 0), proj(0, 1), proj(0, 2));    line_0 = normalize(line_0);
        vec3 line_1(proj(1, 0), proj(1, 1), proj(1, 2));    line_1 = normalize(line_1);
        vec3 line_2(proj(2, 0), proj(2, 1), proj(2, 2));    line_2 = normalize(line_2);
        
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
        
        const float X = determinant(m1);
        const float Y = -determinant(m2);
        const float Z = determinant(m3);
        const float T = -determinant(m4);
        const vec3 cam_pos(X / T, Y / T, Z / T);
        
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

    void Camera::onFrameModified() {
        projectionMatrixIsUpToDate_ = false;
        modelViewMatrixIsUpToDate_ = false;
    }

}
