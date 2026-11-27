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


#include <easy3d/gui/picker.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/renderer/drawable_renderer.h>
#include <easy3d/renderer/clipping_plane.h>

#include <cmath>
#include <limits>
#include <stdexcept>


namespace easy3d {



    Picker::Picker(const Camera *cam) : camera_(cam) {
        if (!cam) throw std::invalid_argument("picker requires a camera");
    }


    Picker::~Picker() = default;


    graphics::RenderDevice& Picker::render_device() {
        if (!device_) {
            device_ = graphics::create_device();
        }
        if (drawable_renderer_)
            drawable_renderer_->prune();
        return *device_;
    }

    graphics::DrawCommand Picker::draw_command(const std::shared_ptr<Drawable>& drawable, unsigned id) {
        if (!drawable_renderer_)
            drawable_renderer_.reset(new graphics::DrawableRenderer);
        std::vector<vec4> planes;
        const auto *clipping = ClippingPlane::instance();
        if (clipping->is_enabled()) {
            const auto n = clipping->normal(), c = clipping->center();
            planes.push_back(vec4(-n, dot(n, c)));
            if (clipping->cross_section())
                planes.push_back(vec4(n, -dot(n, c) +
                                         clipping->cross_section_width() * clipping->scene_radius()));
            planes = graphics::normalize_clipping_planes(planes);
        }
        return drawable_renderer_->command(drawable, id, *camera(), planes);
    }

    void Picker::screen_to_opengl(int x, int y, int &gl_x, int &gl_y, int width, int height) const {
        gl_x = transform::pixel_index(x, camera()->screen_width(), width);
        const auto top_y = transform::pixel_index(y, camera()->screen_height(), height);
        // Flip a discrete row index (top row 0 <-> bottom row height-1), not a continuous
        // coordinate: for centers, height - (i + 0.5) == (height - 1 - i) + 0.5.
        const int64_t bottom_y = int64_t(height) - 1 - top_y;
        if (bottom_y < std::numeric_limits<int>::min() || bottom_y > std::numeric_limits<int>::max())
            throw std::out_of_range("picker coordinate exceeds integer range");
        gl_y = static_cast<int>(bottom_y);
    }

}
