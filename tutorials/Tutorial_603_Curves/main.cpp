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

#include <easy3d/viewer/viewer.h>
#include <easy3d/core/curve.h>
#include <easy3d/core/spline_curve_fitting.h>
#include <easy3d/core/spline_curve_interpolation.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/util/initializer.h>

/**
 * \example{lineno} Tutorial_603_Curves/main.cpp
 *
 * This example shows how to create/evaluate several types of curves and curve fitting/interpolation methods in Easy3D.
 */

using namespace easy3d;

void create_drawable(Viewer& viewer, const std::vector<vec2>& curve_points, const vec3& color) {
    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i < curve_points.size() - 1; ++i) {
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    LinesDrawable2D *drawable = new LinesDrawable2D;
    drawable->update_vertex_buffer(curve_points, viewer.width(), viewer.height());
    drawable->update_element_buffer(indices);
    drawable->set_uniform_coloring(vec4(color, 1.0f));
    viewer.add_drawable(drawable);
}

void add_Bezier(Viewer& viewer, const std::vector<vec2>& control_points, int bezier_steps, const vec3& color) {
    Bezier<Vec, 2, float> curve;
    const auto curve_points = curve.generate(control_points, bezier_steps);
    create_drawable(viewer, curve_points, color);
}

void add_BSpline(Viewer& viewer, const std::vector<vec2>& control_points, int bezier_steps, const vec3& color) {
    BSpline<Vec, 2, float> curve;
    const auto curve_points = curve.generate(control_points, bezier_steps);
    create_drawable(viewer, curve_points, color);
}

void add_CatmullRom(Viewer& viewer, const std::vector<vec2>& control_points, int bezier_steps, const vec3& color) {
    CatmullRom<Vec, 2, float> curve;
    const auto curve_points = curve.generate(control_points, bezier_steps);
    create_drawable(viewer, curve_points, color);
}


int main(int argc, char** argv) {
    // initialize Easy3D.
    initialize();

    // create the default Easy3D viewer.
    // (a viewer must be created before creating any drawables).
    Viewer viewer(EXAMPLE_TITLE);

    // the control points
    std::vector<vec2> points = {
        vec2(100, 100), // 0
        vec2(100, 500), // 1
        vec2(400, 500), // 2
        vec2(400, 100), // 3
        vec2(700, 100), // 4
        vec2(700, 500), // 5
    };
    create_drawable(viewer, points, vec3(0, 0, 0)); // this is the control polygon

    const int resolution = 100;    // Number of line subdivisions to display the spline

    add_Bezier(viewer, points, resolution, vec3(1, 0, 0));
    add_BSpline(viewer, points, resolution, vec3(0, 1, 0));
    add_CatmullRom(viewer, points, resolution, vec3(0, 0, 1));

    {
        // spline curve fitting
        const int order = 3;           // Smoothness of the spline (min 2)
        typedef SplineCurveFitting<Vec, 2, float> Fitter;
        Fitter fitter(order, Fitter::eOPEN_UNIFORM);
        fitter.set_ctrl_points(points);
        std::vector<vec2> curve_points;
        for(int i = 0; i < resolution; ++i) {
            const vec2 p = fitter.eval_f( static_cast<float>(i) / static_cast<float>(resolution - 1) );
            curve_points.push_back(p);
        }
        create_drawable(viewer, curve_points, vec3(0, 1, 1));
    }

    {
        // spline curve interpolation
        typedef SplineCurveInterpolation<Vec, 2, float> Interpolator;
        Interpolator interpolator;
        interpolator.set_boundary(Interpolator::second_deriv, 0, Interpolator::second_deriv, 0);
        interpolator.set_points(points);
        std::vector<vec2> curve_points;
        for (int i = 0; i < resolution; ++i) {
            const vec2 p = interpolator.eval_f(static_cast<float>(i) / static_cast<float>(resolution - 1));
            curve_points.push_back(p);
        }
        create_drawable(viewer, curve_points, vec3(1, 0, 1));
    }

    // run the viewer
    return viewer.run();
}