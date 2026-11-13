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


#ifndef EASY3D_RENDERER_BACKENDS_VULKAN_DRAWABLE_H
#define EASY3D_RENDERER_BACKENDS_VULKAN_DRAWABLE_H

#include <easy3d/renderer/render_device.h>
#include <memory>
#include <vector>

namespace easy3d {
    class Camera;
    class Drawable;
    namespace vulkan {

        // Internal rendering implementation for the shared Drawable classes. Owns CPU
        // command/texture caches, not scene objects, cameras, devices or GPU resources.
        // One instance per host; immutable command snapshots may outlive the cache.
        class DrawableRenderer {
        public:
            DrawableRenderer();
            ~DrawableRenderer();
            DrawableRenderer(const DrawableRenderer &) = delete;
            DrawableRenderer &operator=(const DrawableRenderer &) = delete;

            graphics::DrawCommand command(const std::shared_ptr<Drawable> &drawable,
                                          uint32_t id,
                                          const Camera &camera,
                                          const std::vector<vec4> &clipping_planes,
                                          bool model_selected = false) const;
            void prune() const;
            void clear();

        private:
            struct Impl;
            std::unique_ptr<Impl> impl_;
        };
    } // namespace vulkan
} // namespace easy3d
#endif
