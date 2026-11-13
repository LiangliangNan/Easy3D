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


#ifndef EASY3D_RENDERER_BACKENDS_VULKAN_CLIPPING_PLANE_H
#define EASY3D_RENDERER_BACKENDS_VULKAN_CLIPPING_PLANE_H

#include <easy3d/renderer/render_device.h>
namespace easy3d {
    class Camera;
    class ClippingPlane;
    namespace vulkan {
        // Counterpart of ClippingPlane's OpenGL draw/setup methods. Shared CPU state
        // remains in ClippingPlane; the host submits these read-only scene annotations.
        class ClippingPlaneRenderer {
        public:
            static std::vector<vec4> planes(const ClippingPlane &plane);
            std::vector<graphics::DrawCommand> draw(const ClippingPlane &plane, const Camera &camera);

        private:
            float radius_ = 0;
            std::shared_ptr<const graphics::Geometry> face_;
            graphics::DrawCommand wire_;
            mat4 wire_mvp_;
            int width_ = 0, height_ = 0;
        };
    } // namespace vulkan
} // namespace easy3d
#endif
