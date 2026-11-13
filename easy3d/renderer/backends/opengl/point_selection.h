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


#ifndef EASY3D_RENDERER_BACKENDS_OPENGL_POINT_SELECTION_H
#define EASY3D_RENDERER_BACKENDS_OPENGL_POINT_SELECTION_H

#include <easy3d/renderer/render_device.h>
#include <easy3d/renderer/shader_program.h>
namespace easy3d {
    class Drawable;
    namespace opengl {
        // OpenGL counterpart of the Vulkan compute point selector. Regions are evaluated
        // in a vertex shader with transform feedback: one invocation owns one 32-bit
        // selection word, so no atomics or compute shaders are required on GL 3.2.
        // Context-owned realizations. GPU snapshots retain the rendering buffer's storage;
        // this owner releases GL objects before context destruction, even for retained snapshots.
        class PointSelector {
        public:
            ~PointSelector();
            // Resources backed by a live drawable (positions alias the rendering VBO).
            // The resource is refreshed automatically when the drawable's buffers change.
            std::shared_ptr<const graphics::PointData> points(const std::shared_ptr<Drawable> &drawable);
            // Resource uploaded from a CPU snapshot, for models that have no rendering
            // buffers. Positions are indexed by source slot; ids mark selectable slots.
            std::shared_ptr<const graphics::PointData> points(std::shared_ptr<const graphics::PointData> data);
            std::vector<uint32_t> select(const graphics::PointSelectionRequest &request);

        private:
            struct Points;
            std::vector<std::shared_ptr<Points>> points_;
            std::unique_ptr<ShaderProgram> shader_;
            unsigned vao_ = 0, feedback_ = 0, input_ = 0, output_ = 0, polygon_ = 0, polygon_texture_ = 0;
        };
    } // namespace opengl
} // namespace easy3d
#endif
