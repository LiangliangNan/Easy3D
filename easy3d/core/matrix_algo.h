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


#ifndef EASY3D_CORE_MATRIX_ALGO_H
#define EASY3D_CORE_MATRIX_ALGO_H

#include <easy3d/core/matrix.h>


namespace easy3d {

    using MATRIX = easy3d::Matrix<double>;

    /**
     * Compute the determinant of a square matrix.
     * @param A The input matrix.
     * @return The determinant of A.
     */
    double determinant(const MATRIX &A);


    /**
     * Compute the inverse of a square matrix. This is a wrapper around Eigen's inverse function.
     * @param A The input matrix.
     * @param invA The inverse of A.
     * @return false if failed (failure is reported only if the input matrix is not square).
     *      Upon return, invA carries the inverse of A.
     */
    bool inverse(const MATRIX&A, MATRIX&invA);


    /**
     * Compute the inverse of a square matrix. This is a wrapper around Eigen's inverse function.
     * @param A The input matrix.
     * @return The inverse of A.
     */
    MATRIX inverse(const MATRIX&A);


    /**
     * Compute the Singular Value Decomposition (SVD) of an M by N matrix. This is a wrapper around Eigen's JacobiSVD.
     *
     * For an m-by-n matrix A, the singular value decomposition is an m-by-m orthogonal matrix U, an m-by-n diagonal
     * matrix S, and an n-by-n orthogonal matrix V so that A = U*S*V^T.
     *
     * The singular values, s[k] = S[k][k], are sorted in decreasing order, i.e., sigma[i] >= sigma[i+1] for any i.
     *
     * The singular value decomposition always exists, so the decomposition will never fail.
     *
     * @param A The input matrix to be decomposed, which can have an arbitrary size.
     * @param U The left side M by M orthogonal matrix.
     * @param S The middle M by N diagonal matrix, with zero elements outside of its main diagonal.
     * @param V The right side N by N orthogonal matrix V.
     *
     * @return Upon return, U, S, and V carry the result of the SVD decomposition.
     *
     * @attention V is returned (instead of V^T).
     */
    void svd_decompose(const MATRIX&A, MATRIX&U, MATRIX&S, MATRIX&V);


    /**
     * Solve a linear system (Ax=b) in the least squares sense.
     *
     * @param A The m-by-n (m >= n) coefficient matrix.
     * @param b The right-hand constant vector (m dimensional).
     * @param x The result of the system was successfully solved (m dimensional).
     * @return false if failed. If true, x carries the least-squares solution to the linear system.
     */
    bool solve_least_squares(const MATRIX&A, const std::vector<double> &b, std::vector<double> &x);
}

#endif // EASY3D_MATRIX_ALGOTHMS_H
