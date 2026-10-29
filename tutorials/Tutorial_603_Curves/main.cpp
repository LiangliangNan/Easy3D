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

#include "../viewer_backend.h"
#include "../native_viewer_options.h"
#include <easy3d/core/curve.h>
#include <easy3d/core/spline_curve_fitting.h>
#include <easy3d/core/spline_curve_interpolation.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/util/initializer.h>
#include <cmath>
#include <algorithm>

/**
 * \example{lineno} Tutorial_603_Curves/main.cpp
 *
 * This example shows how to create/evaluate several types of curves and curve fitting/interpolation methods in Easy3D.
 */

using namespace easy3d;

void create_drawable(TutorialViewer& viewer, const std::vector<vec2>& curve_points, const vec3& color) {
    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i + 1 < curve_points.size(); ++i) {
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    auto drawable = std::make_shared<LinesDrawable2D>();
    viewer.add_drawable(drawable); // selects CPU buffers before upload in native/dual builds
#if EASY3D_TUTORIAL_USES_VULKAN
    const float dpi=viewer.dpi_scaling();
    const int width=int(std::lround(viewer.width()/dpi)),height=int(std::lround(viewer.height()/dpi));
#else
    const int width=viewer.width(),height=viewer.height();
#endif
    drawable->update_vertex_buffer(curve_points,width,height);
    drawable->update_element_buffer(indices);
    drawable->set_uniform_coloring(vec4(color, 1.0f));
    drawable->set_line_width(3.0f);
}

void add_Bezier(TutorialViewer& viewer, const std::vector<vec2>& control_points, int bezier_steps, const vec3& color) {
    Bezier<Vec, 2, float> curve;
    const auto curve_points = curve.generate(control_points, bezier_steps);
    create_drawable(viewer, curve_points, color);
}

void add_BSpline(TutorialViewer& viewer, const std::vector<vec2>& control_points, int bezier_steps, const vec3& color) {
    BSpline<Vec, 2, float> curve;
    const auto curve_points = curve.generate(control_points, bezier_steps);
    create_drawable(viewer, curve_points, color);
}

void add_CatmullRom(TutorialViewer& viewer, const std::vector<vec2>& control_points, int bezier_steps, const vec3& color) {
    CatmullRom<Vec, 2, float> curve;
    const auto curve_points = curve.generate(control_points, bezier_steps);
    create_drawable(viewer, curve_points, color);
}


void create_curves_drawables(TutorialViewer& viewer) {
    viewer.set_usage("default", "Press '[' / ']' to decrease/increase line width\n"
                                "Press '0' to reset line width to 3 pixels");
    auto adjust_width = [](TutorialViewer* host, float delta, bool reset) {
        for (const auto& drawable : host->drawables()) {
            auto* lines = dynamic_cast<LinesDrawable2D*>(drawable.get());
            if (lines) lines->set_line_width(reset ? 3.0f : std::max(1.0f, std::min(20.0f, lines->line_width() + delta)));
        }
        return true;
    };
    viewer.bind([adjust_width](TutorialViewer* host, Model*) { return adjust_width(host, -1, false); },
                nullptr, TutorialViewer::KEY_LEFT_BRACKET);
    viewer.bind([adjust_width](TutorialViewer* host, Model*) { return adjust_width(host, 1, false); },
                nullptr, TutorialViewer::KEY_RIGHT_BRACKET);
    viewer.bind([adjust_width](TutorialViewer* host, Model*) { return adjust_width(host, 0, true); },
                nullptr, TutorialViewer::KEY_0);
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

}

#ifndef EASY3D_TUTORIAL_TEST
int main(int argc,char** argv) {
    initialize();
#ifdef EASY3D_NO_OPENGL
    NativeTutorialOptions options;
    if (!options.parse(argc,argv)) return EXIT_FAILURE;
    TutorialViewer viewer(EXAMPLE_TITLE,options.viewer);
#else
    TutorialViewer viewer(EXAMPLE_TITLE);
#endif
    create_curves_drawables(viewer);
#ifdef EASY3D_NO_OPENGL
    return options.run(viewer);
#else
    return viewer.run();
#endif
}
#endif // EASY3D_TUTORIAL_TEST
