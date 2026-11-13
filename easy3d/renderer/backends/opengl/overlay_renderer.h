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


#ifndef EASY3D_RENDERER_BACKENDS_OPENGL_OVERLAY_RENDERER_H
#define EASY3D_RENDERER_BACKENDS_OPENGL_OVERLAY_RENDERER_H

#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/framebuffer_object.h>
#include <functional>
namespace easy3d {
    namespace opengl {
        // Per-context realization of immutable triangle annotations. Viewer owns GL state isolation.
        class OverlayRenderer {
        public:
            ~OverlayRenderer();
            void set(std::vector<graphics::DrawCommand> commands, bool foreground);
            void draw(bool foreground,
                      int source_width,
                      int source_height,
                      const ivec4 &tile,
                      const std::function<void(int, int)> &draw_depth);

        private:
            struct Draw {
                graphics::DrawCommand command;
                unsigned vao = 0, vertices = 0, indices = 0, texture = 0;
            };
            void clear();
            std::vector<Draw> draws_;
            std::unique_ptr<ShaderProgram> shader_;
            std::unique_ptr<FramebufferObject> depth_, hud_;
            bool foreground_ = false;
        };
    } // namespace opengl
} // namespace easy3d
#endif
