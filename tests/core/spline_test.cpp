// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/core/types.h>
#include <easy3d/core/spline_curve_fitting.h>
#include <easy3d/core/spline_curve_interpolation.h>
#include <cmath>
using namespace easy3d;
int main() { return test::run("spline fitting and interpolation",[] {
    std::vector<vec3> points{vec3(0,0,0),vec3(1,0,0),vec3(2,1,0),vec3(3,1,0)};
    SplineCurveFitting<Vec,3,float> fit(3,SplineCurveFitting<Vec,3,float>::eOPEN_UNIFORM);fit.set_ctrl_points(points);
    SplineCurveInterpolation<Vec,3,float> curve;curve.set_points(points);
    test::require((curve.eval_f(0)-points.front()).length()<1e-5f && (curve.eval_f(1)-points.back()).length()<1e-5f,"interpolation endpoints differ");
    for(unsigned i=0;i<=100;++i) for(const auto& p:{fit.eval_f(i/100.0f),curve.eval_f(i/100.0f)})
        test::require(std::isfinite(p.x) && std::isfinite(p.y) && std::abs(p.z)<1e-5f,"non-finite/nonplanar spline");
    SplineCurveInterpolation<Vec,3,float> timed;
    timed.set_points(std::vector<float>{0,1,2,3},points);
    for(unsigned i=0;i<points.size();++i) test::require((timed.eval_f(float(i)/3)-points[i]).length()<1e-5f,"timed interpolation missed a knot");
}); }
