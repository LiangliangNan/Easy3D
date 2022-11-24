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

#ifndef EASY3D_RENDERER_MANIPULATED_CAMERA_FRAME_H
#define EASY3D_RENDERER_MANIPULATED_CAMERA_FRAME_H

#include <easy3d/renderer/manipulated_frame.h>


namespace easy3d {


    /*! \brief A manipulated frame with camera specific mouse bindings.
      \class ManipulatedCameraFrame easy3d/renderer/manipulated_camera_frame.h

      \details A ManipulatedCameraFrame is a specialization of a ManipulatedFrame, designed
      to be set as the Camera::frame(). Mouse motions are basically interpreted in a
      negated way: when the mouse goes to the right, the ManipulatedFrame
      translation goes to the right, while the ManipulatedCameraFrame has to go to
      the \e left, so that the \e scene seems to move to the right.

      A ManipulatedCameraFrame rotates around its pivotPoint(), which corresponds to
      the associated Camera::pivotPoint().

      A ManipulatedCameraFrame can also "fly" in the scene. It basically moves
      forward, and turns according to the mouse motion. See flySpeed(),
      sceneUpVector() and the QGLViewer::MOVE_FORWARD and QGLViewer::MOVE_BACKWARD
      QGLViewer::MouseAction.

      See the <a href="../mouse.html">mouse page</a> for a description of the
      possible actions that can be performed using the mouse and their bindings.
      */
    class ManipulatedCameraFrame : public ManipulatedFrame {
    public:
        ManipulatedCameraFrame();

        /*! Virtual destructor. Empty. */
        ~ManipulatedCameraFrame() override = default;

        ManipulatedCameraFrame(const ManipulatedCameraFrame &mcf);

        ManipulatedCameraFrame &operator=(const ManipulatedCameraFrame &mcf);

        /*! @name Pivot point */
        //@{
    public:
        /*! Returns the point the ManipulatedCameraFrame pivot point, around which the
        camera rotates.

        It is defined in the world coordinate system. Default value is (0,0,0).

        When the ManipulatedCameraFrame is associated to a Camera,
        Camera::pivotPoint() also returns this value. This point can interactively be
        changed using the mouse (see Camera::setPivotPointFromPixel() and
        QGLViewer::RAP_FROM_PIXEL and QGLViewer::RAP_IS_CENTER in the <a
        href="../mouse.html">mouse page</a>). */
        vec3 pivotPoint() const { return pivotPoint_; }

        /*! Sets the pivotPoint(), defined in the world coordinate system. */
        void setPivotPoint(const vec3 &point) { pivotPoint_ = point; }
        //@}

        /*! @name Camera manipulation */
        //@{
    public:
        /*! Returns whether or not the QGLViewer::ZOOM action zooms on the pivot
          point.

          When set to \c false (default), a zoom action will move the camera along its
          Camera::viewDirection(), i.e. back and forth along a direction perpendicular
          to the projection screen.

          setZoomsOnPivotPoint() to \c true will move the camera along an axis defined
          by the Camera::pivotPoint() and its current position instead. As a result,
          the projected position of the pivot point on screen will stay the same
          during a zoom. */
        bool zoomsOnPivotPoint() const { return zoomsOnPivotPoint_; }

        /*! Sets the value of zoomsOnPivotPoint(). Default value is false. */
        void setZoomsOnPivotPoint(bool enabled) { zoomsOnPivotPoint_ = enabled; }
        //@}

        /*! @name Frame manipulation */
        //@{
    public:
        void action_rotate(int mouse_x, int mouse_y, int mouse_dx, int mouse_dy, Camera* camera, ScreenAxis axis) override;
        void action_translate(int mouse_x, int mouse_y, int mouse_dx, int mouse_dy, Camera* camera, ScreenAxis axis) override;
        void action_zoom(int dy_wheel, Camera* camera) override;

        virtual void action_turn(float angle_radian, Camera* camera);        // The rotation around camera Y
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
