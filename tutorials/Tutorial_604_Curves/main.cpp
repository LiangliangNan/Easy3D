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

#include <easy3d/viewer/viewer.h>
#include <easy3d/core/curve.h>
#include <easy3d/core/spline_curve_fitting.h>
#include <easy3d/core/spline_curve_interpolation.h>
#include <easy3d/renderer/drawable_lines_2D.h>
#include <easy3d/util/initializer.h>
#include <cmath>
#include <algorithm>

/**
 * \example{lineno} Tutorial_604_Curves/main.cpp
 *
 * \brief Creating and evaluating Bezier, B-spline, and Catmull-Rom curves, plus spline fitting and interpolation.
 *
 * <b>Overview</b>
 * This tutorial draws the same six control points through five different curve constructions so their
 * behavior can be compared visually: a Bezier curve, a B-spline, a Catmull-Rom spline, an approximating
 * spline fit, and a cubic spline interpolant. Everything is rendered as 2D polylines in pixel coordinates
 * with LinesDrawable2D, making it a compact reference for "which curve do I want?".
 *
 * <b>The technique</b>
 * A parametric curve maps a parameter t in [0,1] to a point C(t). The constructions differ in how the
 * control points influence C(t):
 * - Bezier: C(t) is a weighted average of all control points using the Bernstein basis polynomials.
 *   The curve passes through the first and last control point and stays inside their convex hull, but
 *   moving any control point reshapes the entire curve (global control).
 * - B-spline: a piecewise polynomial with local control — each control point only affects a few neighboring
 *   segments — giving smoother, more editable shapes. With an open-uniform knot vector it also starts and
 *   ends at the first/last control point. It generally does not pass through the interior control points.
 * - Catmull-Rom: an interpolating cubic spline that passes through every control point, estimating each
 *   tangent from the neighboring points. Convenient when the curve must hit the given samples.
 * - Fitting vs. interpolation: SplineCurveFitting builds an approximating B-spline from the control
 *   points (the curve is pulled toward them but need not pass through them), with a selectable order and
 *   knot-vector type, while SplineCurveInterpolation forces the curve through every data point by solving
 *   for a cubic spline with chosen end conditions (here zero second derivative, the "natural" spline).
 *   Evaluation uses the efficient blossom / de Boor algorithm.
 * References: C. de Boor, "A Practical Guide to Splines", 1978; E. Catmull and R. Rom, "A class of local
 * interpolating splines", 1974; G. Farin, "Curves and Surfaces for CAGD".
 *
 * <b>Easy3D API</b>
 * - Bezier, BSpline, CatmullRom (in easy3d/core/curve.h): templated on the point type; generate() samples
 *   the curve into a polyline.
 * - SplineCurveFitting, SplineCurveInterpolation: set the control/data points, then eval_f(t) evaluates the
 *   curve at parameter t in [0,1].
 * - LinesDrawable2D: a drawable whose vertices are 2D pixel coordinates; update_vertex_buffer() takes the
 *   viewport width/height, update_element_buffer() the line-segment indices.
 * - Viewer::bind_shortcut(): attaches lambdas to keys to adjust the line width interactively.
 *
 * <b>Running it</b>
 * A window opens with the black control polygon and the five colored curves overlaid: red Bezier, green
 * B-spline, blue Catmull-Rom, cyan spline fit, magenta spline interpolation. Press '[' / ']' to
 * decrease/increase the line width and '0' to reset it to 3 pixels.
 *
 * \see Bezier, BSpline, CatmullRom, SplineCurveFitting, SplineCurveInterpolation, LinesDrawable2D, Viewer
 */

using namespace easy3d;

// Turn a sampled polyline (a list of 2D points) into a LinesDrawable2D and add it to the viewer.
void create_drawable(Viewer &viewer, const std::vector<vec2> &curve_points, const vec3 &color) {
    // Build line-segment indices: each consecutive pair of points (i, i+1) forms one segment.
    std::vector<unsigned int> indices;
    for (unsigned int i = 0; i + 1 < curve_points.size(); ++i) {
        indices.push_back(i);
        indices.push_back(i + 1);
    }

    auto drawable = std::make_shared<LinesDrawable2D>();
    viewer.add_drawable(drawable);
    // LinesDrawable2D works in pixel coordinates, so it needs the viewport size to map the
    // 2D points onto the screen.
    const int width = viewer.width(), height = viewer.height();
    drawable->update_vertex_buffer(curve_points, width, height);
    drawable->update_element_buffer(indices);
    drawable->set_uniform_coloring(vec4(color, 1.0f));
    drawable->set_line_width(3.0f);
}

// Sample a Bezier curve through the control points and draw it. A Bezier curve passes through the
// first and last control point and is pulled toward (but generally misses) the interior ones.
void add_Bezier(Viewer &viewer, const std::vector<vec2> &control_points, int bezier_steps, const vec3 &color) {
    Bezier<Vec, 2, float> curve;
    const auto curve_points = curve.generate(control_points, bezier_steps);
    create_drawable(viewer, curve_points, color);
}

// Sample a B-spline and draw it. A B-spline has local control and is smoother; with an open-uniform
// knot vector it also starts/ends at the first/last control point.
void add_BSpline(Viewer &viewer, const std::vector<vec2> &control_points, int bezier_steps, const vec3 &color) {
    BSpline<Vec, 2, float> curve;
    const auto curve_points = curve.generate(control_points, bezier_steps);
    create_drawable(viewer, curve_points, color);
}

// Sample a Catmull-Rom spline and draw it. Unlike the previous two, it interpolates (passes through)
// every control point.
void add_CatmullRom(Viewer &viewer, const std::vector<vec2> &control_points, int bezier_steps, const vec3 &color) {
    CatmullRom<Vec, 2, float> curve;
    const auto curve_points = curve.generate(control_points, bezier_steps);
    create_drawable(viewer, curve_points, color);
}

void create_curves_drawables(Viewer &viewer) {
    viewer.set_usage("default",
                     "Press '[' / ']' to decrease/increase line width\n" "Press '0' to reset line width to 3 pixels");
    // A shared helper that nudges (or resets) the width of every 2D line drawable, clamped to [1, 20].
    auto adjust_width = [](Viewer *host, float delta, bool reset) {
        for (const auto &drawable : host->drawables()) {
            auto *lines = dynamic_cast<LinesDrawable2D *>(drawable.get());
            if (lines)
                lines->set_line_width(reset ? 3.0f : std::max(1.0f, std::min(20.0f, lines->line_width() + delta)));
        }
        return true;
    };
    // Bind the helper to keys. The Model* argument is unused here (these are viewer-wide drawables).
    viewer.bind_shortcut([adjust_width](Viewer *host) { return adjust_width(host, -1, false); }, Viewer::KEY_LEFT_BRACKET);
    viewer.bind_shortcut([adjust_width](Viewer *host) { return adjust_width(host, 1, false); }, Viewer::KEY_RIGHT_BRACKET);
    viewer.bind_shortcut([adjust_width](Viewer *host) { return adjust_width(host, 0, true); }, Viewer::KEY_0);
    // the control points
    // Six points in pixel coordinates forming a zig-zag; every curve below is built from these.
    std::vector<vec2> points = {
        vec2(100, 100), // 0
        vec2(100, 500), // 1
        vec2(400, 500), // 2
        vec2(400, 100), // 3
        vec2(700, 100), // 4
        vec2(700, 500), // 5
    };
    create_drawable(viewer, points, vec3(0, 0, 0)); // this is the control polygon

    const int resolution = 100; // Number of line subdivisions to display the spline

    // The three classic control-point curves, each in a distinct color for comparison.
    add_Bezier(viewer, points, resolution, vec3(1, 0, 0));
    add_BSpline(viewer, points, resolution, vec3(0, 1, 0));
    add_CatmullRom(viewer, points, resolution, vec3(0, 0, 1));

    {
        // spline curve fitting
        // An approximating B-spline: order 3 (quadratic) with an open-uniform knot vector, so the
        // curve starts/ends at the first/last control point but only approximates the interior ones.
        const int order = 3; // Smoothness of the spline (min 2)
        typedef SplineCurveFitting<Vec, 2, float> Fitter;
        Fitter fitter(order, Fitter::eOPEN_UNIFORM);
        fitter.set_ctrl_points(points);
        // Sample the curve at 'resolution' evenly spaced parameter values in [0, 1].
        std::vector<vec2> curve_points;
        for (int i = 0; i < resolution; ++i) {
            const vec2 p = fitter.eval_f(static_cast<float>(i) / static_cast<float>(resolution - 1));
            curve_points.push_back(p);
        }
        create_drawable(viewer, curve_points, vec3(0, 1, 1));
    }

    {
        // spline curve interpolation
        // A natural cubic spline: it is forced through every data point, with zero second derivative
        // (zero curvature) at both ends. set_boundary() must come before set_points().
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

int main(int argc, char **argv) {
    // Bootstrap Easy3D (logging, resource directory, ...). Must come first.
    initialize();
    Viewer viewer(EXAMPLE_TITLE);

    // Build all the curve drawables (and their key bindings), then enter the interactive loop.
    create_curves_drawables(viewer);
    return viewer.run();
}
