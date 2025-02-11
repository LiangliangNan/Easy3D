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

#ifndef EASY3D_RENDERER_MANIPULATED_CAMERA_FRAME_H
#define EASY3D_RENDERER_MANIPULATED_CAMERA_FRAME_H

#include <easy3d/renderer/manipulated_frame.h>


namespace easy3d {

    /**
     * \brief A manipulated frame with camera-specific mouse bindings.
     * \class ManipulatedCameraFrame easy3d/renderer/manipulated_camera_frame.h
     *
     * \details A ManipulatedCameraFrame is a specialization of a ManipulatedFrame, designed
     *      to be set as the Camera::frame(). Mouse motions are interpreted in a way that
     *      mimics the movement of the camera in the scene. For example, when the mouse moves
     *      to the right, the ManipulatedFrame would translate to the right, but the
     *      ManipulatedCameraFrame translates to the \e left, making the \e scene appear to
     *      move to the right.
     *
     *      A ManipulatedCameraFrame rotates around its pivotPoint(), which corresponds to
     *      the associated Camera::pivotPoint().
     */
    class ManipulatedCameraFrame : public ManipulatedFrame {
    public:
        /// Default constructor.
        ManipulatedCameraFrame();

        /*! Virtual destructor. Empty. */
        ~ManipulatedCameraFrame() override = default;

        /**
         * \brief Copy constructor.
         * \param mcf The ManipulatedCameraFrame to copy from.
         */
        ManipulatedCameraFrame(const ManipulatedCameraFrame &mcf);

        /**
         * \brief Copy assignment operator.
         * \param mcf The ManipulatedCameraFrame to assign from.
         * \return A reference to this ManipulatedCameraFrame.
         */
        ManipulatedCameraFrame &operator=(const ManipulatedCameraFrame &mcf);

        /*! @name Pivot point */
        //@{
        /**
         * \brief Returns the point the ManipulatedCameraFrame pivots around, which is the
         *      camera's pivot point.
         * \details The pivot point is defined in the world coordinate system. Default value is (0,0,0).
         *      When the ManipulatedCameraFrame is associated with a Camera, Camera::pivotPoint() also returns this
         *      value. This point can interactively be changed by Camera::setPivotPoint().
         * \return The pivot point.
         * \sa setPivotPoint()
         */
        vec3 pivotPoint() const { return pivotPoint_; }

        /**
         * \brief Sets the pivotPoint(), defined in the world coordinate system.
         * \param point The new pivot point.
         */
        void setPivotPoint(const vec3 &point) { pivotPoint_ = point; }
        //@}

        /*! @name Camera manipulation */
        //@{
    public:
        /**
         * \brief Returns whether the QGLViewer::ZOOM action zooms on the pivot point.
         * \details When set to \c false (default), a zoom action will move the camera along its
         *      Camera::viewDirection(), i.e., back and forth along a direction perpendicular to the projection screen.
         *      setZoomsOnPivotPoint() to \c true will move the camera along an axis defined by the Camera::pivotPoint()
         *      and its current position instead. As a result, the projected position of the pivot point on screen will
         *      stay the same during a zoom.
         * \return \c true if zooming on the pivot point is enabled, \c false otherwise.
         */
        bool zoomsOnPivotPoint() const { return zoomsOnPivotPoint_; }

        /**
         * \brief Sets the value of zoomsOnPivotPoint().
         * \param enabled Whether zooming on the pivot point is enabled.
         */
        void setZoomsOnPivotPoint(bool enabled) { zoomsOnPivotPoint_ = enabled; }
        //@}

        /*! @name Frame manipulation */
        //@{
        /**
         * \brief Rotates the frame based on mouse movement.
         * \param mouse_x The current x position of the mouse.
         * \param mouse_y The current y position of the mouse.
         * \param mouse_dx The change in x position of the mouse.
         * \param mouse_dy The change in y position of the mouse.
         * \param camera The camera associated with the frame.
         * \param axis The axis of rotation.
         */
        void action_rotate(int mouse_x, int mouse_y, int mouse_dx, int mouse_dy, Camera* camera, ScreenAxis axis) override;
        /**
         * \brief Translates the frame based on mouse movement.
         * \param mouse_x The current x position of the mouse.
         * \param mouse_y The current y position of the mouse.
         * \param mouse_dx The change in x position of the mouse.
         * \param mouse_dy The change in y position of the mouse.
         * \param camera The camera associated with the frame.
         * \param axis The axis of translation.
         */
        void action_translate(int mouse_x, int mouse_y, int mouse_dx, int mouse_dy, Camera* camera, ScreenAxis axis) override;
        /**
         * \brief Zooms the frame based on mouse wheel movement.
         * \param dy_wheel The change in the mouse wheel position.
         * \param camera The camera associated with the frame.
         */
        void action_zoom(int dy_wheel, Camera* camera) override;
        /**
         * \brief Turns the frame around the camera's Y axis.
         * \param angle_radian The angle of rotation in radians.
         * \param camera The camera associated with the frame.
         */
        virtual void action_turn(float angle_radian, Camera* camera);
        //@}

        //---------------------------------------------------------------
        // more movement of the camera
        //---------------------------------------------------------------

//		if (key == GLFW_KEY_UP && modifiers == 0) {	// move camera forward
//			float step = 0.02f * camera_->sceneRadius();
//			camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, 0.0, -step)));
//		}
//		else if (key == GLFW_KEY_DOWN && modifiers == 0) {// move camera backward
//			float step = 0.02f * camera_->sceneRadius();
//			camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, 0.0, step)));
//		}
//        else if (key == GLFW_KEY_LEFT && modifiers == GLFW_MOD_CONTROL) {	// move camera left
//            float step = 0.02f * camera_->sceneRadius();
//            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(-step, 0.0, 0.0)));
//        }
//        else if (key == GLFW_KEY_RIGHT && modifiers == GLFW_MOD_CONTROL) {	// move camera right
//            float step = 0.02f * camera_->sceneRadius();
//            camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(step, 0.0, 0.0)));
//        }
//		else if (key == GLFW_KEY_UP && modifiers == GLFW_MOD_CONTROL) {	// move camera up
//			float step = 0.02f * camera_->sceneRadius();
//			camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, step, 0.0)));
//		}
//		else if (key == GLFW_KEY_DOWN && modifiers == GLFW_MOD_CONTROL) {	// move camera down
//			float step = 0.02f * camera_->sceneRadius();
//			camera_->frame()->translate(camera_->frame()->inverseTransformOf(vec3(0.0, -step, 0.0)));
//		}
//
//---------------------------------------------------------------------------
//
//        // zoom to pixel
//        const double coef = 0.1;
//        // Small hack: attach a temporary frame to take advantage of lookAt without modifying frame
//        static ManipulatedCameraFrame* tempFrame = new ManipulatedCameraFrame;
//        tempFrame->setPosition(coef*camera()->frame()->position() + (1.0-coef)*p);
//        tempFrame->setOrientation(camera()->frame()->orientation());
//        camera()->setFrame(tempFrame);
//        camera()->lookAt(p);
//
//        // zoom to fit screen
//        static ManipulatedCameraFrame* tempFrame = new ManipulatedCameraFrame;
//        tempFrame->setPosition(camera()->frame()->position());
//        tempFrame->setOrientation(camera()->frame()->orientation());
//        camera()->setFrame(tempFrame);
//        camera()->showEntireScene();

    private:
        bool zoomsOnPivotPoint_;
        vec3 pivotPoint_;

    private:
        friend class Camera;
        friend class Viewer;
    };


}

#endif // EASY3D_RENDERER_MANIPULATED_CAMERA_FRAME_H
