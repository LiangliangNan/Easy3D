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
            {-4.93228e-05f, -2.15228f, -0.000414636f},
            {0.67432f, -2.04006f, 0.0198827f},
            {1.27128f, -1.71769f, 0.0826002f},
            {1.78659f, -1.13582f, 0.114589f},
            {2.04748f, -0.424773f, 0.16422f},
            {2.0299f, 0.367363f, 0.194508f},
            {1.74704f, 1.0471f, 0.231476f},
            {1.20926f, 1.60884f, 0.276214f},
            {0.596261f, 1.91678f, 0.237454f},
            {-0.521621f, 1.94807f, 0.123359f},
            {-1.25919f, 1.59415f, 0.057158f},
            {-1.9597f, 0.645731f, 0.0169604f},
            {-2.08293f, -0.167375f, 0.014973f},
            {-1.85798f, -1.01401f, -0.00569031f},
            {-1.38188f, -1.62776f, -0.0579325f},
            {-0.667969f, -2.04237f, -0.0196691f},
            {0.00960736f, -2.15215f, 0.0204933f},
            {1.02282f, -1.88401f, 0.0158519f}
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
        interpolator.set_boundary(easy3d::SplineCurveInterpolation<vec3>::second_deriv, 0, easy3d::SplineCurveInterpolation<Vec<3, float>>::second_deriv, 0);
        interpolator.set_points(points);
        for (unsigned int i = 0; i < resolution; i+=100) {
            const vec3 p = interpolator.eval_f(float(i) / float(resolution - 1));
            std::cout << "\tcurve point " << i << ": " << p << std::endl;
        }
    }

    std::cout << "spine interpolation use accumulated time as parameter (partially printed)" << std::endl;
    {
        SplineCurveInterpolation<vec3> interpolator;
        interpolator.set_boundary(easy3d::SplineCurveInterpolation<vec3>::second_deriv, 0, easy3d::SplineCurveInterpolation<Vec<3, float>>::second_deriv, 0);

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
