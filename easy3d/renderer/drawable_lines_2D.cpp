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

#include <easy3d/renderer/drawable_lines_2D.h>
#include <cmath>
#include <stdexcept>

namespace easy3d {
LinesDrawable2D::~LinesDrawable2D() {
#ifdef EASY3D_RENDERER_OPENGL
    if (position_texture_ || index_texture_ || line_vao_) opengl_release_line_resources();
#endif
}
void LinesDrawable2D::set_line_width(float width) {
    if (!std::isfinite(width) || width<=0) throw std::invalid_argument("line width must be positive and finite");
    line_width_=width;
}
void LinesDrawable2D::update_vertex_buffer(const std::vector<vec2>& vertices, int width, int height, bool dynamic) {
    if (width<=0 || height<=0) throw std::invalid_argument("screen-space lines require a positive viewport");
    std::vector<vec2> ndc; ndc.reserve(vertices.size());
    for (const auto& p:vertices) {
        if (!std::isfinite(p.x) || !std::isfinite(p.y)) throw std::invalid_argument("non-finite screen-space line vertex");
        ndc.emplace_back(2*p.x/width-1,1-2*p.y/height);
        if (!std::isfinite(ndc.back().x) || !std::isfinite(ndc.back().y))
            throw std::invalid_argument("screen-space line coordinate overflow");
    }
    if (has_cpu_buffers()) {
        std::vector<vec3> points; points.reserve(ndc.size());
        for (const auto& p:ndc) points.emplace_back(p.x,p.y,0);
        Drawable::update_vertex_buffer(points,dynamic);
    } else opengl_update_vertex_buffer(ndc,dynamic);
}
}
