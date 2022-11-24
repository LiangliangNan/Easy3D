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

#ifndef EASY3D_GUI_PICKER_H
#define EASY3D_GUI_PICKER_H

#include <easy3d/core/types.h>
#include <easy3d/renderer/camera.h>


namespace easy3d {

    class FramebufferObject;

    /**
     * \brief Base class for picking mechanism.
     * \class Picker easy3d/gui/picker.h
     * \see ModelPicker, PointCloudPicker, SurfaceMeshPicker
     */
    class Picker {
    public:
        explicit Picker(const Camera *cam);

        virtual ~Picker();

        /// \brief Returns the pointer of the camera.
        const Camera *camera() const { return camera_; }

        /**
         * \brief Construct a picking line.
         * @param x The cursor x-coordinate, relative to the left edge of the content area.
         * @param y The cursor y-coordinate, relative to the top edge of the content area.
         * @attention The screen point is expressed in the screen coordinate system with an origin in the upper left
         *            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with
         *            a Retina display.
         */
        Line3 picking_line(int x, int y) const {
            const vec3 p_near = unproject(x, y, 0);
            const vec3 p_far = unproject(x, y, 1);
            return Line3::from_two_points(p_near, p_far);
        }

        /**
         * \brief The picking direction, pointing inside the screen.
         * @param x The cursor x-coordinate, relative to the left edge of the content area.
         * @param y The cursor y-coordinate, relative to the top edge of the content area.
         * @attention The screen point is expressed in the screen coordinate system with an origin in the upper left
         *            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with
         *            a Retina display.
         */
        vec3 picking_dir(int x, int y) const {
            return picking_line(x, y).direction();
        }

        /**
         * \brief Project a 3D point in the world coordinate system onto the 2D screen coordinate system.
         * @param p A 3D point in the world coordinate system.
         * @return The x and y components of the returned value denote the projected screen point expressed in the
         *         screen coordinate system, with (0, 0) being the upper left corner of the content area. The z
         *         component ranges between 0.0 (near plane) and 1.0 (excluded, far plane).
         * @attention The screen point is expressed in the screen coordinate system with an origin in the upper left
         *            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with
         *            a Retina display.
         */
        vec3 project(const vec3 &p) const {
            return camera()->projectedCoordinatesOf(p);
        }

        /**
         * \brief Compute the world coordinates of a point defined in the screen coordinate system.
         * @param x The cursor x-coordinate, relative to the left edge of the content area.
         * @param y The cursor y-coordinate, relative to the top edge of the content area.
         * @param depth The depth value of the screen point, ranging between 0.0 and 1.0 (excluded).
         * @return The world unprojected coordinates of the screen point.
         * @attention The screen point is expressed in the screen coordinate system with an origin in the upper left
         *            corner. So it doesn't necessarily correspond to a pixel on High DPI devices, e.g. a Mac with
         *            a Retina display.
         */
        vec3 unproject(int x, int y, float depth) const {
            return camera()->unprojectedCoordinatesOf(vec3(static_cast<float>(x), static_cast<float>(y), depth));
        }

        /**
         * \brief Convert a point expressed in the screen coordinate system (with an origin in the upper left corner)
         * into the OpenGL coordinate system (with an origin in the lower left corner). The high DPI scaling is also
         * taken into consideration, so the result always corresponds to its image pixel.
         * @param x The x-coordinate, relative to the left edge of the content area.
         * @param y The y-coordinate, relative to the top edge of the content area.
         * @param gl_x Returns the x component of the point in the OpenGL coordinate system.
         * @param gl_y Returns the y component of the point in the OpenGL coordinate system.
         * @param width The width of the OpenGL viewport (may not be identical to the width of the screen in pixels).
         * @param height The width of the OpenGL viewport (may not be identical to the height of the screen in pixels).
         */
        void screen_to_opengl(int x, int y, int &gl_x, int &gl_y, int width, int height) const;

    protected:

        // prepare a frame buffer for the offscreen rendering
        void setup_framebuffer(int width, int height);

    protected:
        const Camera *camera_;

        bool use_gpu_if_supported_; // use GPU if supported

        // All the picking tasks can share the same framebuffer
        static FramebufferObject *fbo_;
    };

}

#endif  // EASY3D_GUI_PICKER_H
