// SPDX-License-Identifier: GPL-3.0-or-later
#include <tests/test.h>
#include <easy3d/core/types.h>
#include <cmath>
using namespace easy3d;
int main() { return test::run("linear solver residuals and inversion",[] {
    Mat<3,3,double> m; m.set_row(0,Vec<3,double>(4,1,0));m.set_row(1,Vec<3,double>(1,3,1));m.set_row(2,Vec<3,double>(0,1,2));
    const Vec<3,double> expected(1,2,3),b=m*expected;
    Mat<3,3,double> lu,l;Vec<3,double> permutation,x;double sign;
    test::require(lu_decomposition(m,&lu,&permutation,&sign),"LU decomposition failed");lu_back_substitution(lu,permutation,b,&x);
    for(unsigned i=0;i<3;++i) test::require(std::abs(x[i]-expected[i])<1e-10,"LU residual too large");
    test::require(cholesky_decompose(m,l),"positive definite matrix rejected");
    Mat<3,1,double> rhs(b.data()),solution;Mat<3,3,double> inv;
    test::require(gauss_jordan_elimination(m,rhs,&inv,&solution),"Gauss-Jordan failed");
    const auto identity=m*inv;for(unsigned i=0;i<3;++i) for(unsigned j=0;j<3;++j)
        test::require(std::abs(identity(i,j)-(i==j?1.0:0.0))<1e-10,"inverse residual too large");
    m(0,0)=-1;test::require(!cholesky_decompose(m,l),"indefinite matrix accepted");
}); }
