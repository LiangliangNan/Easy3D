/**
 * Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
 * https://3d.bk.tudelft.nl/liangliang/
 *
 * This file is part of Easy3D. If it is useful in your research/work,
 * I would be grateful if you show your appreciation by citing it:
 * ------------------------------------------------------------------
 *      Liangliang Nan.
 *      Easy3D: a lightweight, easy-to-use, and efficient C++
 *      library for processing and rendering 3D data. 2018.
 * ------------------------------------------------------------------
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
 */

#include <easy3d/util/logging.h>
#include <easy3d/core/types.h>
#include <easy3d/core/spline_curve_fitting.h>
#include <easy3d/core/spline_curve_interpolation.h>
#include <easy3d/renderer/key_frame_interpolator.h>


using namespace easy3d;

int main(int argc, char *argv[]) {
    logging::initialize();

#if 0
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

    const int resolution = 1000;    // Number of line subdivisions to display the spline

    // spine fitting
    std::ofstream output_fitting("/Users/lnan/Projects/Easy3D/resources/data/cameras_spine_fitting_for_bunny.xyz");
    const int order = 3;            // Smoothness of the spline (min 2)
    SplineCurveFitting<vec3> fitter(order, SplineCurveFitting<vec3>::eOPEN_UNIFORM);
    fitter.set_ctrl_points(points);
    for (int i = 0; i < resolution; ++i) {
        const vec3 p = fitter.eval_f(float(i) / float(resolution - 1));
        output_fitting << p << std::endl;
    }

    // spine interpolation
    std::ofstream output_interpolation("/Users/lnan/Projects/Easy3D/resources/data/cameras_spine_interpolation_for_bunny.xyz");
    SplineCurveInterpolation<vec3> interpolator;
    interpolator.set_boundary(easy3d::SplineCurveInterpolation<vec3>::second_deriv, 0, easy3d::SplineCurveInterpolation<Vec<3, float>>::second_deriv, 0,false);
#if 1 // use accumulated curve length as parameter
    interpolator.set_points(points);
#else // use accumulated time as parameter
    std::vector<float> t(points.size(), 0.0f);
    for (std::size_t i=0; i<points.size(); ++i)
        t[i] = i;
    interpolator.set_points(t, points);
#endif
    for (int i = 0; i < resolution; ++i) {
        const vec3 p = interpolator.eval_f(float(i) / float(resolution - 1));
        output_interpolation << p << std::endl;
    }

#else
    std::string file = "/Users/lnan/Dropbox/Students/2020-PhD-Zexin/data/Lille-3D/test.kf";
//    std::string file = "/Users/lnan/Projects/Easy3D/resources/data/bunny.kf";
    KeyFrameInterpolator kfi;
    kfi.read_keyframes(file);
    int num = kfi.number_of_keyframes();

    typedef Vec<7, float>  KeyFrame;
    std::vector< KeyFrame > keyframes(num);
    for (int i=0; i<num; ++i) {
        auto frame = kfi.keyframe(i);
        auto p = frame.position();
        for (int j=0; j<3; ++j)
            keyframes[i][j] = p[j];
        auto q = frame.orientation();
        for (int j=0; j<4; ++j)
            keyframes[i][j+3] = q[j];
    }


    const int resolution = 100;    // Number of line subdivisions to display the spline

    // spine fitting
    std::string fitting_file = "keyframes_spine_fitting.kf";
    kfi.delete_path();
    const int order = 3;            // Smoothness of the spline (min 2)
    SplineCurveFitting<KeyFrame> fitter(order, SplineCurveFitting<KeyFrame>::eOPEN_UNIFORM);
    fitter.set_ctrl_points(keyframes);
    for (int i = 0; i < resolution; ++i) {
        const KeyFrame frame = fitter.eval_f(float(i) / float(resolution - 1));
        vec3 pos(frame.data());
        quat orient;
        for (int j=0; j<4; ++j)
            orient[j] = frame[j+3];
        orient.normalize();
        kfi.add_keyframe(Frame(pos, orient));
    }
    kfi.save_keyframes(fitting_file);

    // spine interpolation
    std::string interpolation_file = "keyframes_spine_interpolation.kf";
    kfi.delete_path();
    SplineCurveInterpolation<KeyFrame> interpolator;
    interpolator.set_boundary(easy3d::SplineCurveInterpolation<KeyFrame>::second_deriv, 0, easy3d::SplineCurveInterpolation<KeyFrame>::second_deriv, 0,false);
#if 1
    interpolator.set_points(keyframes);
#else // use accumulated time as parameter
    std::vector<float> t(num, 0.0f);
    for (std::size_t i=0; i<num; ++i)
        t[i] = i;
    interpolator.set_points(t, keyframes);
#endif
    for (int i = 0; i < resolution; ++i) {
        const KeyFrame frame = interpolator.eval_f(float(i) / float(resolution - 1));
        vec3 pos(frame.data());
        quat orient;
        for (int j=0; j<4; ++j)
            orient[j] = frame[j+3];
        orient.normalize();
        kfi.add_keyframe(Frame(pos, orient));
    }
    kfi.save_keyframes(interpolation_file);

#endif

    return EXIT_SUCCESS;
}
