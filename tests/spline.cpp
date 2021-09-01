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


#include <easy3d/core/types.h>
#include <easy3d/core/spline_curve_fitting.h>
#include <easy3d/core/spline_curve_interpolation.h>


using namespace easy3d;

int test_spline() {

    // these are actually a set of camera positions around the bunny.ply model
    std::vector<vec3> points = {
            {-4.93228e-05, -2.15228, -0.000414636},
            {0.67432, -2.04006, 0.0198827},
            {1.27128, -1.71769, 0.0826002},
            {1.78659, -1.13582, 0.114589},
            {2.04748, -0.424773, 0.16422},
            {2.0299, 0.367363, 0.194508},
            {1.74704, 1.0471, 0.231476},
            {1.20926, 1.60884, 0.276214},
            {0.596261, 1.91678, 0.237454},
            {-0.521621, 1.94807, 0.123359},
            {-1.25919, 1.59415, 0.057158},
            {-1.9597, 0.645731, 0.0169604},
            {-2.08293, -0.167375, 0.014973},
            {-1.85798, -1.01401, -0.00569031},
            {-1.38188, -1.62776, -0.0579325},
            {-0.667969, -2.04237, -0.0196691},
            {0.00960736, -2.15215, 0.0204933},
            {1.02282, -1.88401, 0.0158519}
    };

    const unsigned int resolution = 1000;    // Number of line subdivisions to display the spline

    std::cout << "spine fitting (partially printed)" << std::endl;
    {
        const int order = 3;            // Smoothness of the spline (min 2)
        SplineCurveFitting<vec3> fitter(order, SplineCurveFitting<vec3>::eOPEN_UNIFORM);
        fitter.set_ctrl_points(points);
        for (unsigned int i = 0; i < resolution; i += 100) {
            const vec3 p = fitter.eval_f(float(i) / float(resolution - 1));
            std::cout << "\tcurve point " << i << ": " << p << std::endl;
        }
    }

    // spine interpolation
    std::cout << "spine interpolation use accumulated curve length as parameter (partially printed)" << std::endl;
    {
        SplineCurveInterpolation<vec3> interpolator;
        interpolator.set_boundary(easy3d::SplineCurveInterpolation<vec3>::second_deriv, 0, easy3d::SplineCurveInterpolation<Vec<3, float>>::second_deriv, 0,false);
        interpolator.set_points(points);
        for (unsigned int i = 0; i < resolution; i+=100) {
            const vec3 p = interpolator.eval_f(float(i) / float(resolution - 1));
            std::cout << "\tcurve point " << i << ": " << p << std::endl;
        }
    }

    std::cout << "spine interpolation use accumulated time as parameter (partially printed)" << std::endl;
    {
        SplineCurveInterpolation<vec3> interpolator;
        interpolator.set_boundary(easy3d::SplineCurveInterpolation<vec3>::second_deriv, 0, easy3d::SplineCurveInterpolation<Vec<3, float>>::second_deriv, 0,false);

        std::vector<float> t(points.size(), 0.0f);
        for (std::size_t i = 0; i < points.size(); ++i)
            t[i] = i;
        interpolator.set_points(t, points);

        for (unsigned int i = 0; i < resolution; i+=100) {
            const vec3 p = interpolator.eval_f(float(i) / float(resolution - 1));
            std::cout << "\tcurve point " << i << ": " << p << std::endl;
        }
    }

    return EXIT_SUCCESS;
}
