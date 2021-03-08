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
#include <vector>

using namespace easy3d;


int main(int argc, char *argv[]) {
    logging::initialize();


#if 0 // non-symmetric
    std::vector<double> rows = {
            10, 0, 0, -2, -1, -1,
            0, 10, 0, 0, -2, 0,
            0, 0, 15, 0, 0, 0,
            -2, 0, 0, 10, -1, 0,
            -1, -2, 0, -1, 1, 0,
            -1, 0, 0, 0, 0, 6
    };
#else // symmetric
    std::vector<double> rows = {
            10, 0, 0, -2, -1, -1,
            0, 10, 0, 0, -2, 0,
            0, 0, 15, 0, 0, 0,
            -2, 0, 0, 10, -1, 0,
            -1, -2, 0, -1, 1, 0,
            -1, 0, 0, 0, 0, 6
    };
#endif

    const int m = 6;

    Mat<m, m, double> M;
    for (int i=0; i<m; ++i)
        M.set_row(i, Vec<m, double>(rows.data() + i * 6));
//    std::cout << "M: " << M;

    std::vector<double> rhs = {-9, 10, 45, 33, -4, 35};
    Vec<m, double> b(rhs.data());

    std::cout << "b: " << b << std::endl;

    //	outputs:
    Mat<m, m, double> alu;		// result of LU decomposition
    Vec<m, double> rowp;	// result row permutation data for alu
    double d;	// sign of determinant
    Vec<m, double> x;		// result solution set
    lu_decomposition(M, &alu, &rowp, &d);	// get lu decomposition
    lu_back_substitution(alu, rowp, b, &x);	// get solution set

//    std::cout << "alu \n" << alu;

//    std::cout << "rowp \n" << rowp << std::endl;

    std::cout << "x \n" << x << std::endl;

    std::cout << "d: " << d << std::endl;

    std::cout << "M*x: " << M * x << std::endl;

    std::cout << "inverse(M)*b: " << inverse(M) * b << std::endl;

    return EXIT_SUCCESS;
}