// SPDX-License-Identifier: GPL-3.0-or-later
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
#ifndef EASY3D_RENDERER_BACKENDS_VULKAN_KEY_FRAME_INTERPOLATOR_H
#define EASY3D_RENDERER_BACKENDS_VULKAN_KEY_FRAME_INTERPOLATOR_H
#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/frame.h>
namespace easy3d {
    class Camera;
    class KeyFrameInterpolator;
    namespace vulkan {
        // Native rendering counterpart of KeyFrameInterpolator's path/camera drawing.
        // CPU interpolation and playback remain in the shared KeyFrameInterpolator.
        class KeyFrameInterpolatorRenderer {
        public:
            graphics::DrawCommand draw_path(KeyFrameInterpolator &path,
                                            const Camera &camera,
                                            float thickness = 2,
                                            const vec4 &color = vec4(1, 1, .5f, 1));
            graphics::DrawCommand draw_cameras(KeyFrameInterpolator &path,
                                               const Camera &camera,
                                               float camera_width,
                                               const vec4 &color = vec4(.2f, .7f, .3f, 1));

        private:
            std::vector<vec3> path_positions_;
            std::vector<Frame> keyframes_;
            graphics::DrawCommand path_command_, camera_command_;
            mat4 path_mvp_;
            int width_ = 0, height_ = 0;
            float thickness_ = 0, camera_width_ = 0, fov_ = 0, aspect_ = 0;
        };
    } // namespace vulkan
} // namespace easy3d
#endif
