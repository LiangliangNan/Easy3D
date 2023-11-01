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


#include <easy3d/core/matrix_algo.h>
#include <easy3d/util/logging.h>

#include <3rd_party/eigen/Eigen/Dense>


namespace easy3d {


    double determinant(const MATRIX &A) {
        const int m = A.rows();
        const int n = A.cols();
        Eigen::MatrixXd C(m, n);
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j)
                C(i, j) = A(i, j);
        }
        return C.determinant();
    }


    bool inverse(const MATRIX &A, MATRIX &invA) {
        const int m = A.rows();
        const int n = A.cols();
        if (m != n) {
            LOG(ERROR) << "could not compute inverse (not a square matrix)";
            return false;
        }

        Eigen::MatrixXd C(m, n);
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j)
                C(i, j) = A(i, j);
        }

        invA.resize(m, n);
        const Eigen::MatrixXd invC = C.inverse();
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j)
                invA(i, j) = invC(i, j);
        }
        return true;
    }

    
    MATRIX inverse(const MATRIX &A) {
        const int m = A.rows();
        const int n = A.cols();
        if (m != n) {
            LOG(ERROR) << "could not compute inverse (not a square matrix)";
        }
        MATRIX invA(m, n);
        inverse(A, invA);
        return invA;
    }

    
    void svd_decompose(const MATRIX &A, MATRIX &U, MATRIX &S, MATRIX &V) {
        const int m = A.rows();
        const int n = A.cols();
        Eigen::MatrixXd C(m, n);
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j)
                C(i, j) = A(i, j);
        }

        Eigen::JacobiSVD<Eigen::MatrixXd> svd(C, Eigen::ComputeFullU | Eigen::ComputeFullV);
        const Eigen::MatrixXd &eU = svd.matrixU();
        const Eigen::MatrixXd &eV = svd.matrixV();
        const auto &eS = svd.singularValues();

        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < m; ++j)
                U(i, j) = eU(i, j);
        }

        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < n; ++j)
                V(i, j) = eV(i, j);
        }

        S.load_zero();
        for (int i = 0; i < std::min(m, n); ++i)
            S(i, i) = eS(i);
    }


    bool solve_least_squares(const MATRIX &A, const std::vector<double> &b, std::vector<double> &x) {
        const int m = A.rows();
        const int n = A.cols();
        if (n > m) {
            LOG(ERROR) << "could not solve: A.cols() > A.rows()";
            return false;
        }

        if (m != b.size()) {
            LOG(ERROR) << "could not solve: sizes of A and b don't match";
            return false;
        }

        Eigen::MatrixXd M(m, n);
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j)
                M(i, j) = A(i, j);
        }

        Eigen::MatrixXd C(m, 1);
        for (int i = 0; i < n; ++i)
            C(i, 0) = b[i];

        // https://eigen.tuxfamily.org/dox/group__LeastSquares.html
#if 0
        // The solve() method in the BDCSVD class can be directly used to solve linear squares systems. It is not
        // enough to compute only the singular values (the default for this class); you also need the singular
        // vectors but the thin SVD decomposition suffices for computing least squares solutions.
        Eigen::MatrixXd X = M.bdcSvd(Eigen::ComputeThinU | Eigen::ComputeThinV).solve(C);
#else
        // The solve() method in QR decomposition classes also computes the least squares solution. There are three
        // QR decomposition classes: HouseholderQR (no pivoting, so fast but unstable), ColPivHouseholderQR (column
        // pivoting, thus a bit slower but more accurate) and FullPivHouseholderQR (full pivoting, so slowest and
        // most stable). Here we use the one with column pivoting.
        Eigen::MatrixXd X = M.colPivHouseholderQr().solve(C);
#endif

        x.resize(n);
        for (int i = 0; i < n; ++i)
            x[i] = X(i, 0);

        return true;
    }
}