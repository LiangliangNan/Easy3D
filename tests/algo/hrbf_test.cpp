// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include "hrbf.h"
#include <cmath>
using namespace easy3d;
int main() { return test::run("Hermite radial basis interpolation",[] {
    typedef HRBF<float,3,Rbf_pow3<float>> Fit;Fit fit;
    std::vector<Fit::Vector> points{Fit::Vector(0,0,0),Fit::Vector(1,0,0),Fit::Vector(0,0,2)};
    std::vector<Fit::Vector> normals{Fit::Vector(-1,0,0),Fit::Vector(1,0,0),Fit::Vector(0,0,1)};
    fit.fit(points,normals);
    for(size_t i=0;i<points.size();++i) {
        test::require(std::abs(fit.eval(points[i]))<1e-4,"HRBF missed a sample");
        test::require((fit.grad(points[i])-normals[i]).norm()<1e-4,"HRBF gradient missed a normal");
    }
}); }
