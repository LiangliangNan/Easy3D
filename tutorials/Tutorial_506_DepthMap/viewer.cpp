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

#include "viewer.h"

#include <easy3d/core/model.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/renderer.h>


using namespace easy3d;

// \cond

TutorialDepthMap::TutorialDepthMap(const std::string& title)
    : Viewer(title)
{
    camera()->setUpVector(vec3(0, 1, 0));
    camera()->setViewDirection(vec3(0, 0, -1));
    camera()->showEntireScene();
}


TutorialDepthMap::~TutorialDepthMap() = default;

void TutorialDepthMap::pre_draw() {
    Viewer::pre_draw();
    draw_depth();
}

void TutorialDepthMap::draw_depth() const {
    const float dpi=dpi_scaling();
    // The device generates and samples current scene depth in its render passes.
    // Keep only the inset's immutable geometry here; presentation stays GPU-only.
    if (framebuffer_width()!=depth_width_ || framebuffer_height()!=depth_height_ || dpi!=depth_dpi_) {
        const Rect quad(20*dpi,20*dpi+framebuffer_width()/4.0f,40*dpi,40*dpi+framebuffer_height()/4.0f);
        depth_commands_={graphics::shape::draw_depth_texture(quad,framebuffer_width(),framebuffer_height()),
            graphics::shape::draw_quad_wire(quad,vec4(1,0,0,1),framebuffer_width(),framebuffer_height(),dpi)};
        depth_width_=framebuffer_width();depth_height_=framebuffer_height();depth_dpi_=dpi;
    }
    const_cast<TutorialDepthMap*>(this)->set_overlays(depth_commands_);
}
