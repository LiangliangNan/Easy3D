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
#include <vector>

using namespace easy3d;


bool solve(const std::vector<double>& matrix_in_rows, const std::vector<double>& rhs) {
    const int m = 6;
    Mat<m, m, double> M;
    for (int i=0; i<m; ++i)
        M.set_row(i, Vec<m, double>(matrix_in_rows.data() + i * 6));

    Vec<m, double> b(rhs.data());

    Mat<m, m, double> alu;        // result of LU decomposition
    Vec<m, double> rowp;    // result row permutation data for alu
    double d;    // sign of determinant

    std::cout << "using LU decomposition followed by back substitution" << std::endl;
    {
        Vec<m, double> x;        // result solution set
        if (lu_decomposition(M, &alu, &rowp, &d)) {    // get lu decomposition
            lu_back_substitution(alu, rowp, b, &x);    // get solution set
            std::cout << "x: " << x << std::endl;
        } else {
            std::cerr << "lu decomposition failed" << std::endl;
            return false;
        }
    }

    std::cout << "using LU decomposition to compute the inverse of the matrix" << std::endl;
    {
        Mat<m, m, double> inv;    // result of inversion
        if (lu_decomposition(M, &alu, &rowp, &d)) { // get lu decomposition once
            for (size_t i = 0; i < m; ++i) {    // find inverse by columns
                Vec<m, double> b;
                for (size_t j = 0; j < m; ++j)
                    b[j] = double(0);
                b[i] = double(1);
                lu_back_substitution(alu, rowp, b, &b);
                inv.set_col(i, b); // set ainv column
            }
            std::cout << "x: " << inv * b << std::endl;
        } else {
            std::cerr << "lu decomposition failed" << std::endl;
            return false;
        }
    }

    std::cout << "using Gauss-Jordan elimination" << std::endl;
    {
        Mat<m, 1, double> b(rhs.data());
        Mat<m, m, double> ainv;    // result of inversion
        Mat<m, 1, double> x;
        if (gauss_jordan_elimination(M, b, &ainv, &x)) {
            std::cout << "x: " << x.col(0) << std::endl;
        } else {
            std::cerr << "Gauss-Jordan elimination failed" << std::endl;
            return false;
        }
    }

    std::cout << "using Cholesky decomposition for direct solve" << std::endl;
    {
        Mat<m, m, double> L;
        Vec<m, double> x;        // result solution set
        if (cholesky_decompose(M,L)) {
            cholesky_solve(L, b, x);
            std::cout << "x: " << x << std::endl;
        }
        else
            std::cout << "input matrix is not symmetric, positive definite (this is intended for the test)" << std::endl;
    }

    std::cout << "using Cholesky decomposition to compute the inverse and then solve" << std::endl;
    {
        Mat<m, m, double> L;
        Vec<m, double> x;        // result solution set
        if (cholesky_decompose(M,L)) {
            Mat<m, m, double> I(1.0);
            Mat<m, m, double> inv;
            cholesky_solve(L, I, inv);
            std::cout << "x: " << inv * b << std::endl;
        }
        else
            std::cout << "input matrix is not symmetric, positive definite (this is intended for the test)" << std::endl;
    }

    return true;
}




int test_linear_solvers() {
    std::cout << "test linear solvers with symmetric input matrix..." << std::endl;
    {
        std::vector<double> rows = {
                10, 0, 0, -2, -1, -1,
                0, 10, 0, 0, -2, 0,
                0, 0, 15, 0, 0, 0,
                -2, 0, 0, 10, -1, 0,
                -1, -2, 0, -1, 1, 0,
                -1, 0, 0, 0, 0, 6
        };
        std::vector<double> rhs = {-9, 10, 45, 33, -4, 35};
        if (!solve(rows, rhs))
            return EXIT_FAILURE;
    }

    std::cout << "test linear solvers with non-symmetric input matrix..." << std::endl;
    {
        std::vector<double> rows = {
                10, 0, 0, 0, 0, 0,
                0, 10, -3, -1, 0, 0,
                0, 0, 15, 0, 0, 0,
                -2, 0, 0, 10, -1, 0,
                -1, -2, 0, -5, 1, -3,
                -1, 0, 0, 0, 0, 6
        };
        std::vector<double> rhs = {10, 7, 45, 33, -38, 35};
        if (!solve(rows, rhs))
            return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
