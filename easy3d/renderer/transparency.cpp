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

#include <easy3d/renderer/transparency.h>

#ifdef EASY3D_HAS_VULKAN

namespace easy3d {

    // Out-of-line so that every module shares one registry: an inline function's local
    // static is duplicated per translation unit's module, which on Windows leaves the
    // viewer registering in one copy and scene hosts looking up another.
    std::map<const Camera*, Transparency::FrameRenderer> &Transparency::renderers() {
        static std::map<const Camera *, FrameRenderer> value;
        return value;
    }

    void Transparency::set_frame_renderer(const Camera *camera, FrameRenderer renderer) {
        if (renderer)
            renderers()[camera] = std::move(renderer);
        else
            renderers().erase(camera);
    }

    void Transparency::render_frame(const Camera *camera, const std::vector<TrianglesDrawable *> &surfaces,
                                    int method, int peels) {
        const auto it = renderers().find(camera);
        if (it == renderers().end())
            throw std::logic_error("transparency requires a Viewer or OffScreen frame");
        it->second(surfaces, method, peels);
    }

} // namespace easy3d

#endif
