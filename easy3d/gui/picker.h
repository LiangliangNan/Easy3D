/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#ifndef EASY3D_GUI_PICKER_H
#define EASY3D_GUI_PICKER_H

#include <easy3d/core/types.h>
#include <easy3d/viewer/camera.h>


namespace easy3d {

    class FramebufferObject;

    class Picker {
    public:
        Picker(Camera *cam);

        ~Picker();

        Camera *camera() const { return camera_; }

        //------------------------------------------------------

        Line3 picker_line(int x, int y) const {
            const vec3 p_near = unproject(vec2(float(x), float(y)), 0);
            const vec3 p_far = unproject(vec2(float(x), float(y)), 1);
            return Line3::from_two_points(p_near, p_far);
        }


        // pointing into the screen
        vec3 picker_dir(int x, int y) const {
            return picker_line(x, y).direction();
        }


        // 3D space to screen
        vec2 project(const vec3 &p) const {
            const vec3 q = camera()->projectedCoordinatesOf(p);
            return vec2(q.x, q.y);
        }


        // screen to 3D space
        vec3 unproject(const vec2 &p, float depth) const {
            return camera()->unprojectedCoordinatesOf(vec3(p.x, p.y, depth));
        }


    protected:
        Camera *camera_;

        bool use_gpu_; // use GPU if supported

        // Only one fbo is required for all the picking tasks
        static FramebufferObject *fbo_;
    };

}

#endif  // EASY3D_GUI_PICKER_H
