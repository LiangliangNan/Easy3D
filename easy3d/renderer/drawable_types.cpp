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

#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/drawable_triangles.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/util/setting.h>
namespace easy3d {
    PointsDrawable::PointsDrawable(const std::string &name /*= ""*/, Model* model)
            : Drawable(name, model), point_size_(2.0f), impostor_type_(PLAIN)
    {
        lighting_two_sides_ = setting::points_drawable_two_side_lighting;
        distinct_back_color_ = setting::points_drawable_distinct_backside_color;
        back_color_ = setting::points_drawable_backside_color;
        set_uniform_coloring(vec4(0.0f, 1.0f, 0.0f, 1.0f));
    }
    LinesDrawable::LinesDrawable(const std::string &name, Model *model)
            : Drawable(name, model), line_width_(1.0f), impostor_type_(PLAIN) {
        lighting_two_sides_ = setting::lines_drawable_two_side_lighting;
        distinct_back_color_ = setting::lines_drawable_distinct_backside_color;
        back_color_ = setting::lines_drawable_backside_color;
        set_uniform_coloring(vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }
    TrianglesDrawable::TrianglesDrawable(const std::string& name, Model* model)
            : Drawable(name, model)
            , smooth_shading_(false)
            , opacity_(0.6f)
    {
        lighting_two_sides_ = setting::triangles_drawable_two_side_lighting;
        distinct_back_color_ = setting::triangles_drawable_distinct_backside_color;
        back_color_ = setting::triangles_drawable_backside_color;
        set_uniform_coloring(setting::surface_mesh_faces_color);
    }
    LinesDrawable2D::LinesDrawable2D(const std::string &name)
            : Drawable(name, nullptr)
    {
        set_uniform_coloring(vec4(0.0f, 0.0f, 0.0f, 1.0f));
    }
}
