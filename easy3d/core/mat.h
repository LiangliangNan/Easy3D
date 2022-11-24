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

#ifndef EASY3D_CORE_MAT_H
#define EASY3D_CORE_MAT_H

#include <iostream>
#include <iomanip>

#include <easy3d/core/vec.h>
#include <easy3d/core/constant.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    /**
     * @attention: Matrices are stored internally as column - major unless MATRIX_ROW_MAJOR is defined.
     **/
    //#define MATRIX_ROW_MAJOR

    template <typename T>	class Mat2;
    template <typename T>	class Mat3;
    template <typename T>	class Mat4;


    /**
     * \brief Base class for matrix types.
     * \details Mat is a base matrix class that provides generic functionality for N by M matrices.
     * \tparam N The number of rows in this matrix.
     * \tparam M The number of columns in this matrix.
     * \tparam T The scalar type for matrix elements.
     * \note: Matrices are stored internally as column-major unless MATRIX_ROW_MAJOR is defined.
     * \todo Add a transform() method or overload operator* so as to allow matrices to transform vectors that are
     *	    M-1 in size, as vectors in homogeneous space.
     *
     * \class Mat easy3d/core/mat.h
     * \see Mat2, Mat3, and Mat4
     */
    template <size_t N, size_t M, typename T>
    class Mat
    {
    public:
        //	----------------- constructor and destructor -------------------

        /**
         * \brief Default constructor.
         * \note The matrix elements are intentionally not initialized. This is efficient
         *       if the user assigns their values from subsequent computations. Use Mat(T s)
         *       to initialize the elements during construction. */
        Mat() = default;

        /**	\brief Initialized with diagonal as s and others zeros. */
        explicit Mat(T s);

        /**
         * \brief Copy constructor for rN >= N, rM >= M.
         *	For smaller incoming matrices (i.e, rN < N, rM < M ) specialization
         *	is required in order to fill the remaining elements with appropriate
         *	values (usually 0 or 1).
         *	rN: Number of rows in rhs.
         *	rM: Number of columns in rhs.
         *	rhs: rN by rM matrix of type T to copy.
         *	NOTE: This is explicit to prevent 'accidental' assignment of
         *		  differently-sized matrices.
         *	TODO: Can this ever actually get called? A templated constructor for a
         *		templated class seems dodgy! */
        template <size_t rN, size_t rM>
        explicit Mat(const Mat<rN, rM, T> &rhs);

        /**	\brief Initialize elements from an array of type T. */
        explicit Mat(const T *m);

        /**	\brief Static constructor return an N x M identity matrix.
        *	see also load_identity() */
        static Mat<N, M, T> identity();

        //	------------------- size query ---------------------

        /**	\brief Return the number of rows. */
        size_t num_rows() const { return N; }

        /**	\brief Return the number of columns. */
        size_t num_columns() const { return M; }

        //	--------------------- access -----------------------

        /**	\brief Return row r as a vector. */
        Vec<M, T> row(size_t r) const;

        /**	\brief Return col c as a vector. */
        Vec<N, T> col(size_t c) const;

        /**	\brief Const row/column access to elements. */
        const T& operator()(size_t r, size_t c) const;

        /**	\brief Non-const row/column access to elements. */
        T& operator()(size_t r, size_t c);

        /**	\brief const array/low-level access to elements. */
        operator const T*() const;

        /**	\brief Non-const array/low-level access to elements. */
        operator T*();

        //	--------------------- modification  ------------------

        /**	\brief Set all elements 0 */
        void load_zero();

        /**	\brief Set diagonal elements s and others 0 */
        void load_identity(T s = T(1));

        /**
         * \brief Set row r from vector v. This copies the first M components
         *	from v, so vN must be >= M.
         *	vN: Dimension (number of components) of v.
         */
        template <size_t vN>
        void set_row(size_t r, const Vec<vN, T> &v);

        /**
         * \brief Set col c from vector v. This copies the first N components
         *	from v, so vN must be >= N.
         *	vN: Dimension (number of components) in v.
         */
        template <size_t vN>
        void set_col(size_t c, const Vec<vN, T> &v);

        /**	\brief Swaps row a with row b. */
        void swap_rows(size_t a, size_t b);

        /**	\brief Swaps col a with col b. */
        void swap_cols(size_t a, size_t b);

        //	------- matrix-matrix arithmetic operators --------

        /**	\brief Equality test. */
        bool operator==(const Mat<N, M, T> &rhs) const;

        /**	\brief Inequality test. */
        bool operator!=(const Mat<N, M, T> &rhs) const;

        /**
         * \brief Matrix-matrix multiplication. rhs must have the same number of rows as this matrix has columns.
         *	rM: Columns in rhs.
         *	return Matrix of dimensions N x rM
         */
        template <size_t rM>
        Mat<N, rM, T> operator*(const Mat<M, rM, T> &rhs) const;

        /**	\brief Component-wise matrix-matrix addition. */
        Mat<N, M, T> operator+(const Mat<N, M, T> &rhs) const;

        /**	\brief Component-wise matrix-matrix subtraction.*/
        Mat<N, M, T> operator-(const Mat<N, M, T> &rhs) const;

        /**	\brief Component-wise matrix negation. */
        Mat<N, M, T> operator-() const;

        //	------- matrix-vector arithmetic operators --------

        /**
         * \brief Matrix-vector multiplication. rhs must have the same number of elements as this matrix has columns.
         *	return vec of size N
         */
        Vec<N, T> operator*(const Vec<M, T> &rhs) const;

        //	------- matrix-scalar arithmetic operators --------

        /**	\brief Component-wise matrix-scalar multiplication. */
        Mat<N, M, T> operator*(T rhs) const;
        /**	\brief Component-wise matrix-scalar division. */
        Mat<N, M, T> operator/(T rhs) const;

        //	------- matrix-matrix assignment operators --------

        /**	\brief Matrix-matrix multiplication/assignment. */
        Mat<N, M, T>& operator*=(const Mat<N, M, T> &rhs);

        /**	\brief Component-wise matrix-matrix addition/assignment. */
        Mat<N, M, T>& operator+=(const Mat<N, M, T> &rhs);

        /**	\brief Component-wise matrix-matrix subtraction/assignment. */
        Mat<N, M, T>& operator-=(const Mat<N, M, T> &rhs);

        //	------- matrix-scalar assignment operators --------

        /**	\brief Component-wise matrix-scalar multiplication/assignment. */
        Mat<N, M, T>& operator*=(T rhs);

        /**	\brief Component-wise matrix-scalar division/assignment. */
        Mat<N, M, T>& operator/=(T rhs);

        /**	\brief Component-wise matrix-scalar addition/assignment. */
        Mat<N, M, T>& operator+=(T rhs);

        /**	\brief Component-wise matrix-scalar subtraction/assignment. */
        Mat<N, M, T>& operator-=(T rhs);

    protected:
        /**	Matrix component array. */
        T m_[N * M];
    };


    //	------- global scalar-matrix arithmetic operators --------

    /**	\brief Component-wise scalar-matrix multiplication. */
    template <size_t N, size_t M, typename T>
    Mat<N, M, T> operator*(T lhs, const Mat<N, M, T> &rhs);

    //	------- global matrix-matrix multiplication operators --------

    /**	\brief Specialized matrix-matrix multiplication for 2x2 matrices.*/
    template <typename T>
    Mat2<T> operator*(const Mat2<T> &lhs, const Mat2<T> &rhs);

    /**	\brief Specialized matrix-matrix multiplication for 3x3 matrices.*/
    template <typename T>
    Mat3<T> operator*(const Mat3<T> &lhs, const Mat3<T> &rhs);

    /** \brief Specialized matrix-matrix multiplication for 4x4 matrices.*/
    template <typename T>
    Mat4<T> operator*(const Mat4<T> &lhs, const Mat4<T> &rhs);

    //	------- global matrix-vector arithmetic operators --------

    /**
     * \brief Specialized matrix-vector multiplication for 4x4 matrices and 3D vectors.
     *	rhs is treated as an homogeneous vector (with w = 1).
     */
    template <typename T>
    Vec<3, T> operator*(const Mat4<T> &lhs, const Vec<3, T> &rhs);

    /**
     * \brief Specialized matrix-vector multiplication for 3x3 matrices and 2D vectors.
     *	rhs is treated as an homogeneous vector (with z = 1).
     */
    template <typename T>
    Vec<2, T> operator*(const Mat3<T> &lhs, const Vec<2, T> &rhs);

    /**
     * \brief Non-homogeneous, specialized matrix-vector multiplication for 2x2 matrices and 2D vectors.
     */
    template <typename T>
    Vec<2, T> operator*(const Mat2<T> &lhs, const Vec<2, T> &rhs);
    /**
     * \brief Non-homogeneous, specialized matrix-vector multiplication for 3x3 matrices and 3D vectors.
     */
    template <typename T>
    Vec<3, T> operator*(const Mat3<T> &lhs, const Vec<3, T> &rhs);
    /**
     * \brief Non-homogeneous, specialized matrix-vector multiplication for 4x4 matrices and 4D vectors.
     */
    template <typename T>
    Vec<4, T> operator*(const Mat4<T> &lhs, const Vec<4, T> &rhs);

    //	-------------- global matrix related function ---------------

    /**	\brief Return the trace (sum of elements on the main diagonal) of N by N (square) matrix m.*/
    template <size_t N, typename T>
    T trace(const Mat<N, N, T> &m);

    /**
     * \brief Return the determinant of N x N (square) matrix m.
     * \note This is specialized for matrices up to 4x4 in order to achieve better performance. The general case uses
     * LU decomposition.
     */
    template <size_t N, typename T, size_t A>
    T determinant(const Mat<N, N, T> &m);

    /**	\brief Transpose m. */
    template <size_t N, size_t M, typename T>
    Mat<M, N, T> transpose(const Mat<N, M, T> &m);

    /**
     * \brief Return the inverse of N x N (square) matrix m.
     * \note This is specialized for matrices up to 4x4 in order to achieve better performance. The general case uses
     *      Gauss-Jordan elimination (which uses less memory than the LU decomposition).
     */
    template <size_t N, typename T>
    Mat<N, N, T> inverse(const Mat<N, N, T> &m);

    /**
     * Return the tensor product (outer product) of vectors u and v, where u is treated as a column vector and v is
     * treated as a row vector.
     */
    template <size_t M, size_t N, typename T>
    Mat<N, M, T> tensor(const Vec<M, T> &u, const Vec<N, T> &v);

    /**
     * \brief Perform Gauss-Jordan elimination to solve a set of linear equations and additionally compute the inverse
     *      of the input coefficient matrix.
     * \param a: N x N input matrix.
     * \param b: N x M input matrix containing right-hand vectors.
     * \param ainv: Output inverse of a. This may safely be the same location as a (a will be overwritten).
     * \param x: Output solution set. This may safely be the same location as b (b will be overwritten).
     * \return \c true on success, false if a is a singular matrix.
     */
    template<size_t N, size_t M, typename T>
    bool gauss_jordan_elimination(const Mat<N, N, T> &a, const Mat<N, M, T> &b, Mat<N, N, T> *ainv, Mat<N, M, T> *x);

    /**
     * \brief Perform LU decomposition of a square matrix.
     * \details The outputs from this method can further be used for multiple purposes:
     *      - with lu_back_substitution() to solve a set of linear equations;
     *      - compute the inverse of the input matrix matrix;
     *      - compute the determinant of the input matrix matrix.
     * \param a: N x N input matrix.
     * \param alu: Output N x N matrix, containing the LU decomposition of a row-wise permutation of a. This may safely
     *      be the same location as a (a will be overwritten).
     * \param rowp: Output row permutation data for alu.
     * \param d: The sign of determinant
     * \return \c true on success, false if a is a singular matrix.
     */
    template<size_t N, typename T>
    bool lu_decomposition(const Mat<N, N, T> &a, Mat<N, N, T> *alu, Vec<N, T> *rowp, T *d);

    /**
     * \brief Solve a set of linear equations using outputs from lu_decomposition() as inputs.
     * \param alu: N x N matrix, which is the result of a call to lu_decomposition().
     * \param rowp: Row permutation data for alu, which is the result of a call to lu_decomposition().
     * \param b: N-dimensional input right-hand vector.
     * \param x: Output N-dimensional solution set.
     *
     *	Solve a linear system:
     *	@code
     *	    //	inputs:
     *	    Mat<N, N, T> a; // input rhs matrix
     *	    Vec<N, T> b;	// input lhs vector
     *	    //	outputs:
     *	    Mat<N, N, T> alu;		// result of LU decomposition
     *	    Vec<N, size_t> rowp;	// result row permutation data for alu
     *	    T d;	// sign of determinant
     *	    Vec<N, size_t> x;		// result solution set
     *	    ...
     *	    lu_decomposition(a, &alu, &rowp, &d);	// get lu decomposition
     *	    lu_back_substitution(alu, rowp, b, &x);	// get solution set
     *	@endcode
     *	The last line may be repeated for any number of b vectors using the same alu and rowp inputs.
     *
     *	Find the inverse of a matrix:
     *	@code
     *	    //	inputs:
     *	    Mat<N, N, T> a;		// input matrix
     *	    //	outputs:
     *	    Mat<N, N, T> alu;	// result of LU decomposition
     *	    Mat<N, N, T> ainv;	// result of inversion
     *	    Vec<N, size_t> rowp; // result row permutation data for alu
     *	    T d;	// sign of determinant
     *	    ...
     *	    lu_decomposition(a, &alu, &rowp, &d); // get lu decomposition once
     *	    for (size_t i = 0; i < N; ++i) {	// find inverse by columns
     *	        Vec<N, T> b;
     *	        for (size_t j = 0; j < N; ++j)
     *	            b[j] = T(0);
     *	        b[i] = T(1);
     *	        lu_back_substitution(alu, rowp, b, &b);
     *	        ainv.set_col(i, b); // set ainv column
     *	    }
     *	@endcode
     *
     *	Find the determinant of a matrix:
     *	@code
     *	    //	inputs:
     *	    Mat<N, N, T> a; // input matrix
     *	    //	outpus:
     *	    Mat<N, N, T> alu; // result of LU decomposition
     *	    Vec<N, size_t> rowp; // result row permutation data for alu
     *	    T d; // output determinant
     *	    lu_decomposition(a, &alu, &rowp, &d);
     *	    for (size_t i = 0; i < N; ++i)
     *	    	d *= alu(i, i);
     *	@endcode
     */
    template <size_t N, typename T>
    void lu_back_substitution(const Mat<N, N, T> &alu, const Vec<N, T> &rowp, const Vec<N, T> &b, Vec<N, T> *x);

    /**
     * \brief Cholesky decomposition of a symmetric, positive definite matrix.
     * \details For a symmetric, positive definite matrix A, this function computes the Cholesky factorization,
     *      i.e. it computes a lower triangular matrix L such that A = L*L'. If the matrix is not symmetric or
     *      positive definite, the function computes only a partial decomposition.
     * \return true if the input matrix is symmetric, positive definite (and then the factorization was successful).
     */
    template<size_t N, typename FT>
    bool cholesky_decompose(const Mat<N, N, FT> &A, Mat<N, N, FT> &L);

    /**
     * Solve a linear system A*x = b, using the previously computed Cholesky factorization of A: L*L'.
     * \param L: N x N matrix, which is the result of a call to cholesky_decompose().
     * \param b: N-dimensional input right-hand vector.
     * \param x: Output N-dimensional solution set.
     */
    template <size_t N, typename FT>
    void cholesky_solve(const Mat<N, N, FT> &L, const Vec<N, FT> &b, Vec<N, FT>& x);

    /**
     * Solve a set (i.e, M) of linear systems A*X = B, using the previously computed Cholesky factorization of A: L*L'.
     * (this function can be used to solve for the inverse of a symmetric, positive definite matrix, by using B = I).
     * \param L: N x N matrix, which is the result of a call to cholesky_decompose().
     * \param B: N x M right-hand matrix.
     * \param X: Output N x M solution matrix.
     */
    template<size_t N, size_t M, typename T>
    void cholesky_solve(const Mat<N, N, T> &L, const Mat<N, M, T> &B, Mat<N, M, T>& X);


    template <size_t N, size_t M, typename T>
    std::ostream& operator<< (std::ostream& output, const Mat<N, M, T>& m);

    template <size_t N, size_t M, typename T>
    std::istream& operator>> (std::istream& input, Mat<N, M, T>& m);



    /*******************************************************************************

    IMPLEMENTATION:

    *******************************************************************************/

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<N, M, T>::Mat(T s) {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j)
                if (i == j)
                    (*this)(i, j) = T(s);
                else
                    (*this)(i, j) = T(0);
        }
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    template <size_t rN, size_t rM>
    inline Mat<N, M, T>::Mat(const Mat<rN, rM, T> &rhs) {
        assert(rN >= N);
        assert(rM >= M);

        for (size_t i = 0; i < N; ++i)
            for (size_t j = 0; j < M; ++j)
                (*this)(i, j) = rhs(i, j);
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<N, M, T>::Mat(const T *m) {
        assert(m != 0);

        for (size_t i = 0; i < N * M; ++i)
            m_[i] = m[i];
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<N, M, T> Mat<N, M, T>::identity() {
        Mat<N, M, T> result;
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j)
                if (i == j)
                    result(i, j) = T(1);
                else
                    result(i, j) = T(0);
        }
        return result;
    }


    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Vec<M, T> Mat<N, M, T>::row(size_t r) const {
        assert(r < N);

        Vec<M, T> result;
        for (size_t i = 0; i < M; ++i)
            result[i] = (*this)(r, i);
        return result;
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Vec<N, T> Mat<N, M, T>::col(size_t c) const {
        assert(c < M);

        Vec<N, T> result;
        for (size_t i = 0; i < N; ++i)
            result[i] = (*this)(i, c);
        return result;
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    template <size_t vN>
    inline void Mat<N, M, T>::set_row(size_t r, const Vec<vN, T> &v) {
        assert(r < N);
        assert(vN >= M);

        for (size_t i = 0; i < M; ++i)
            (*this)(r, i) = v[i];
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    template <size_t vN>
    inline void Mat<N, M, T>::set_col(size_t c, const Vec<vN, T> &v) {
        assert(c < M);
        assert(vN >= N);

        for (size_t i = 0; i < N; ++i)
            (*this)(i, c) = v[i];
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline void Mat<N, M, T>::swap_rows(size_t a, size_t b) {
        assert(a < N);
        assert(b < N);

        for (size_t i = 0; i < M; ++i) {
            T tmp = (*this)(a, i);
            (*this)(a, i) = (*this)(b, i);
            (*this)(b, i) = tmp;
        }
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline void Mat<N, M, T>::load_zero() {
        size_t size = N * M;
        for (size_t i = 0; i < size; ++i)
            m_[i] = T(0);
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline void Mat<N, M, T>::load_identity(T s /* = T(1)*/) {
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j)
                if (i == j)
                    (*this)(i, j) = T(s);
                else
                    (*this)(i, j) = T(0);
        }
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline void Mat<N, M, T>::swap_cols(size_t a, size_t b) {
        assert(a < M);
        assert(b < M);

        for (size_t i = 0; i < N; ++i) {
            T tmp = (*this)(i, a);
            (*this)(i, a) = (*this)(i, b);
            (*this)(i, b) = tmp;
        }
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<N, M, T>::operator const T*() const {
        return m_;
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<N, M, T>::operator T*() {
        return m_;
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline const T& Mat<N, M, T>::operator()(size_t row, size_t col) const {
        assert(row < N);
        assert(col < M);

    #ifdef MATRIX_ROW_MAJOR
        return m_[row * M + col]; // row-major
    #else
        return m_[col * N + row]; // column-major
    #endif
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline T& Mat<N, M, T>::operator()(size_t row, size_t col) {
        assert(row < N);
        assert(col < M);

    #ifdef MATRIX_ROW_MAJOR
        return m_[row * M + col]; // row-major
    #else
        return m_[col * N + row]; // column-major
    #endif
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline bool Mat<N, M, T>::operator==(const Mat<N, M, T> &rhs) const {
        bool result = true;
        for (size_t i = 0; i < N * M; ++i)
            result &= equal(m_[i], rhs[i]);
        return result;
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline bool Mat<N, M, T>::operator!=(const Mat<N, M, T> &rhs) const {
        return !(*this == rhs);
    }


    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    template <size_t rM>
    inline Mat<N, rM, T> Mat<N, M, T>::operator*(const Mat<M, rM, T> &rhs) const {
        Mat<N, rM, T> result;
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < rM; ++j) {
                result(i, j) = T(0);
                for (size_t k = 0; k < M; ++k) {
                    result(i, j) += (*this)(i, k) * rhs(k, j);
                }
            }
        }
        return result;
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<N, M, T> Mat<N, M, T>::operator+(const Mat<N, M, T> &rhs) const {
        Mat<N, M, T> result;
        for (size_t i = 0; i < N * M; ++i)
            result[i] = m_[i] + rhs[i];
        return result;
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<N, M, T> Mat<N, M, T>::operator-(const Mat<N, M, T> &rhs) const {
        Mat<N, M, T> result;
        for (size_t i = 0; i < N * M; ++i)
            result[i] = m_[i] - rhs[i];
        return result;
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<N, M, T> Mat<N, M, T>::operator-() const {
        Mat<N, M, T> result;
        for (size_t i = 0; i < N * M; ++i)
            result[i] = -m_[i];
        return result;
    }


    //	MATRIX-VECTOR ARITHMETIC OPERATORS:

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Vec<N, T> Mat<N, M, T>::operator*(const Vec<M, T> &rhs) const {
        Vec<N, T> result;
        for (size_t i = 0; i < N; ++i) {
            result[i] = 0;
            for (size_t j = 0; j < M; ++j) {
                result[i] += (*this)(i, j) * rhs[j];
            }
        }
        return result;
    }


    //	MATRIX-SCALAR ARITHMETIC OPERATORS:

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<N, M, T> Mat<N, M, T>::operator*(T rhs) const {
        Mat<N, M, T> result;
        for (size_t i = 0; i < N * M; ++i)
            result[i] = m_[i] * rhs;
        return result;
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<N, M, T> Mat<N, M, T>::operator/(T rhs) const {
        Mat<N, M, T> result;
        for (size_t i = 0; i < N * M; ++i)
            result[i] = m_[i] / rhs;
        return result;
    }

    //	MATRIX-MATRIX ASSIGNMENT OPERATORS:

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<N, M, T>& Mat<N, M, T>::operator*=(const Mat<N, M, T> &rhs) {
        Mat<N, M, T> tmp;
        tmp = *this * rhs;
        *this = tmp;
        return *this;
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<N, M, T>& Mat<N, M, T>::operator+=(const Mat<N, M, T> &rhs) {
        for (size_t i = 0; i < N * M; ++i)
            m_[i] += rhs[i];
        return *this;
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<N, M, T>& Mat<N, M, T>::operator-=(const Mat<N, M, T> &rhs) {
        for (size_t i = 0; i < N * M; ++i)
            m_[i] -= rhs[i];
        return *this;
    }


    //	MATRIX-SCALAR ASSIGNMENT OPERATORS:

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<N, M, T>& Mat<N, M, T>::operator*=(T rhs) {
        for (size_t i = 0; i < N * M; ++i)
            m_[i] *= rhs;
        return *this;
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<N, M, T>& Mat<N, M, T>::operator/=(T rhs) {
        for (size_t i = 0; i < N * M; ++i)
            m_[i] /= rhs;
        return *this;
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<N, M, T>& Mat<N, M, T>::operator+=(T rhs) {
        for (size_t i = 0; i < N * M; ++i)
            m_[i] += rhs;
        return *this;
    }

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<N, M, T>& Mat<N, M, T>::operator-=(T rhs) {
        for (size_t i = 0; i < N * M; ++i)
            m_[i] -= rhs;
        return *this;
    }


    //	GLOBAL SCALAR-MATRIX ARITHMETIC OPERATORS:

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<N, M, T> operator*(T lhs, const Mat<N, M, T> &rhs) {
        Mat<N, M, T> result;
        for (size_t i = 0; i < N * M; ++i)
            result[i] = lhs * rhs[i];
        return result;
    }


    //	------- global matrix-matrix multiplication operators --------

    template <typename T>
    inline Mat2<T> operator*(const Mat2<T> &lhs, const Mat2<T> &rhs) {
        Mat2<T> result;
        for (size_t i = 0; i < 2; ++i) {
            for (size_t j = 0; j < 2; ++j) {
                result(i, j) = T(0);
                for (size_t k = 0; k < 2; ++k) {
                    result(i, j) += lhs(i, k) * rhs(k, j);
                }
            }
        }
        return result;
    }

    template <typename T>
    inline Mat3<T> operator*(const Mat3<T> &lhs, const Mat3<T> &rhs) {
        Mat3<T> result;
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 3; ++j) {
                result(i, j) = T(0);
                for (size_t k = 0; k < 3; ++k) {
                    result(i, j) += lhs(i, k) * rhs(k, j);
                }
            }
        }
        return result;
    }

    template <typename T>
    inline Mat4<T> operator*(const Mat4<T> &lhs, const Mat4<T> &rhs) {
        Mat4<T> result;
        for (size_t i = 0; i < 4; ++i) {
            for (size_t j = 0; j < 4; ++j) {
                result(i, j) = T(0);
                for (size_t k = 0; k < 4; ++k) {
                    result(i, j) += lhs(i, k) * rhs(k, j);
                }
            }
        }
        return result;
    }

    //	GLOBAL MATRIX-VECTOR ARITHMETIC OPERATORS:

    /*----------------------------- homogeneous version --------------------------*/
    template <typename T>
    inline Vec<3, T> operator*(const Mat4<T> &lhs, const Vec<3, T> &rhs) {
        Vec<4, T> tmp(rhs, T(1));
        Vec<4, T> result = lhs * tmp;
        result /= result[3];
        return Vec<3, T>((T*)result);
    }

    /*----------------------------- homogeneous version --------------------------*/
    template <typename T>
    inline Vec<2, T> operator*(const Mat3<T> &lhs, const Vec<2, T> &rhs) {
        Vec<3, T> tmp(rhs, T(1));
        Vec<3, T> result = lhs * tmp;
        result /= result[2];
        return Vec<2, T>((T*)result);
    }


    /*--------------------------- non-homogeneous version ------------------------*/
    template <typename T>
    inline Vec<2, T> operator*(const Mat2<T> &lhs, const Vec<2, T> &rhs) {
        Vec<2, T> result;
        for (size_t i = 0; i < 2; ++i) {
            result[i] = 0;
            for (size_t j = 0; j < 2; ++j) {
                result[i] += lhs(i, j) * rhs[j];
            }
        }
        return result;
    }

    /*--------------------------- non-homogeneous version ------------------------*/
    /** Matrix-vector multiplication for 3x3 matrices and 3D vectors. **/
    template <typename T>
    inline Vec<3, T> operator*(const Mat3<T> &lhs, const Vec<3, T> &rhs) {
        Vec<3, T> result;
        for (size_t i = 0; i < 3; ++i) {
            result[i] = 0;
            for (size_t j = 0; j < 3; ++j) {
                result[i] += lhs(i, j) * rhs[j];
            }
        }
        return result;
    }

    /*--------------------------- non-homogeneous version ------------------------*/
    /** Matrix-vector multiplication for 4x4 matrices and 4D vectors. **/
    template <typename T>
    inline Vec<4, T> operator*(const Mat4<T> &lhs, const Vec<4, T> &rhs) {
        Vec<4, T> result;
        for (size_t i = 0; i < 4; ++i) {
            result[i] = 0;
            for (size_t j = 0; j < 4; ++j) {
                result[i] += lhs(i, j) * rhs[j];
            }
        }
        return result;
    }



    //	GLOBAL SERVICES:

    /*----------------------------------------------------------------------------*/
    template <size_t N, size_t M, typename T>
    inline Mat<M, N, T> transpose(const Mat<N, M, T> &m) {
        Mat<M, N, T> result;
        for (size_t i = 0; i < N; ++i) {
            for (size_t j = 0; j < M; ++j) {
                result(j, i) = m(i, j);
            }
        }
        return result;
    }

    /*----------------------------------------------------------------------------*/
    /** Compute the trace of a square matrix. **/
    template <size_t D, typename T>
    inline T trace(const Mat<D, D, T> &m) {
        T result = m(0, 0);
        for (size_t i = 1; i < D; ++i)
            result += m(i, i);
        return result;
    }

    /*----------------------------------------------------------------------------*/
    /** Compute the determinant of a square matrix. **/
    template <size_t N, typename T>
    inline T determinant(const Mat<N, N, T> &m) {
        /*	Use LU decomposition to find the determinant. */
        Mat<N, N, T> tmp;
        Vec<N, size_t> rowp;
        T result;
        lu_decomposition(m, &tmp, &rowp, &result);
        for (size_t i = 0; i < N; ++i)
            result *= tmp(i, i);
        return result;
    }

    /** Partial specialization for computing the determinant of a 2x2 matrix. **/
    template <typename T>
    inline T determinant(const Mat2<T> &m) {
        return m(0, 0) * m(1, 1) - m(0, 1) * m(1, 0);
    }

    /** Partial specialization for computing the determinant of a 3x3 matrix. **/
    template <typename T>
    inline T determinant(const Mat3<T> &m) {
        return
            m(0, 0) * (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2)) +
            m(0, 1) * (m(2, 0) * m(1, 2) - m(1, 0) * m(2, 2)) +
            m(0, 2) * (m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1));
    }

    /** Partial specialization for computing the determinant of a 4x4 matrix. **/
    template <typename T>
    inline T determinant(const Mat4<T> &m) {
        return
            m(0, 3) * m(1, 2) * m(2, 1) * m(3, 0) - m(0, 2) * m(1, 3) * m(2, 1) * m(3, 0) -
            m(0, 3) * m(1, 1) * m(2, 2) * m(3, 0) + m(0, 1) * m(1, 3) * m(2, 2) * m(3, 0) +
            m(0, 2) * m(1, 1) * m(2, 3) * m(3, 0) - m(0, 1) * m(1, 2) * m(2, 3) * m(3, 0) -
            m(0, 3) * m(1, 2) * m(2, 0) * m(3, 1) + m(0, 2) * m(1, 3) * m(2, 0) * m(3, 1) +
            m(0, 3) * m(1, 0) * m(2, 2) * m(3, 1) - m(0, 0) * m(1, 3) * m(2, 2) * m(3, 1) -
            m(0, 2) * m(1, 0) * m(2, 3) * m(3, 1) + m(0, 0) * m(1, 2) * m(2, 3) * m(3, 1) +
            m(0, 3) * m(1, 1) * m(2, 0) * m(3, 2) - m(0, 1) * m(1, 3) * m(2, 0) * m(3, 2) -
            m(0, 3) * m(1, 0) * m(2, 1) * m(3, 2) + m(0, 0) * m(1, 3) * m(2, 1) * m(3, 2) +
            m(0, 1) * m(1, 0) * m(2, 3) * m(3, 2) - m(0, 0) * m(1, 1) * m(2, 3) * m(3, 2) -
            m(0, 2) * m(1, 1) * m(2, 0) * m(3, 3) + m(0, 1) * m(1, 2) * m(2, 0) * m(3, 3) +
            m(0, 2) * m(1, 0) * m(2, 1) * m(3, 3) - m(0, 0) * m(1, 2) * m(2, 1) * m(3, 3) -
            m(0, 1) * m(1, 0) * m(2, 2) * m(3, 3) + m(0, 0) * m(1, 1) * m(2, 2) * m(3, 3);
    }

    /*----------------------------------------------------------------------------*/
    /** Compute the inverse of a square matrix. **/
    template <size_t N, typename T>
    inline Mat<N, N, T> inverse(const Mat<N, N, T> &m) {
        // Use Gauss-Jordan elimination to find inverse. This uses less memory than the LU decomposition method.
        // See lu_back_substitution() for an example of computing inverse using LU decomposition.
        size_t indxc[N], indxr[N], ipiv[N];

        Mat<N, N, T> result = m;

        for (size_t i = 0; i < N; ++i)
            ipiv[i] = 0;

        for (size_t i = 0; i < N; ++i) { // for each column
            T max = T(0);
            size_t maxc, maxr;
            for (size_t j = 0; j < N; ++j) { // search for pivot
                if (ipiv[j] != 1) {
                    for (size_t k = 0; k < N; ++k) { // for each column
                        if (ipiv[k] == 0) {
                            T element = std::abs(result(j, k));
                            if (element > max) {
                                max = element;
                                maxr = j;
                                maxc = k;
                            }
                        }
                    }
                }
            }
            ++ipiv[maxc];

            if (maxr != maxc)
                result.swap_rows(maxr, maxc);

            indxr[i] = maxr;
            indxc[i] = maxc;

            //	check for singular matrix:
            if (std::abs(result(maxc, maxc)) < epsilon<T>()) {
                LOG(ERROR) << "input matrix is singular";
                return result; // return partial result
            }

            //	multiply row by 1/pivot:
            T rpivot = T(1) / result(maxc, maxc);
            result(maxc, maxc) = T(1);
            result.set_row(maxc, result.row(maxc) * rpivot);

            //	reduce rows (except pivot):
            for (size_t j = 0; j < N; ++j) {
                if (j != maxc) {
                    T dum = result(j, maxc);
                    result(j, maxc) = T(0);
                    for (size_t k = 0; k < N; ++k)
                        result(j, k) -= result(maxc, k) * dum;
                }
            }
        }

        //	undo column swap:
        size_t i = N;
        do {
            --i;
            if (indxr[i] != indxc[i]) // if swap occurred
                result.swap_cols(indxr[i], indxc[i]);
        } while (i != 0);

        return result;
    }

    /** Partial specialization for computing the inverse of a 2x2 matrix. **/
    template <typename T>
    inline Mat2<T> inverse(const Mat2<T> &m) {
        Mat2<T> result;
        result(0, 0) = m(1, 1);
        result(0, 1) = -m(0, 1);
        result(1, 0) = -m(1, 0);
        result(1, 1) = m(0, 0);

        T det = determinant(m);
        det = T(1) / det;
        result *= det;

        return result;
    }

    /** Partial specialization for computing the inverse of a 3x3 matrix. **/
    template <typename T>
    inline Mat3<T> inverse(const Mat3<T> &m) {
        Mat3<T> result;
        result(0, 0) = (m(1, 1) * m(2, 2) - m(2, 1) * m(1, 2));
        result(0, 1) = -(m(0, 1) * m(2, 2) - m(0, 2) * m(2, 1));
        result(0, 2) = (m(0, 1) * m(1, 2) - m(0, 2) * m(1, 1));
        result(1, 0) = -(m(1, 0) * m(2, 2) - m(1, 2) * m(2, 0));
        result(1, 1) = (m(0, 0) * m(2, 2) - m(0, 2) * m(2, 0));
        result(1, 2) = -(m(0, 0) * m(1, 2) - m(1, 0) * m(0, 2));
        result(2, 0) = (m(1, 0) * m(2, 1) - m(2, 0) * m(1, 1));
        result(2, 1) = -(m(0, 0) * m(2, 1) - m(2, 0) * m(0, 1));
        result(2, 2) = (m(0, 0) * m(1, 1) - m(1, 0) * m(0, 1));

        T det = determinant(m);
        det = T(1) / det;
        result *= det;

        return result;
    }

    /** Partial specialization for computing the inverse of a 4x4 matrix. **/
    template <typename T>
    inline Mat4<T> inverse(const Mat4<T> &m) {
        Mat4<T> result;
        result(0, 0) = m(1, 2) * m(2, 3) * m(3, 1) - m(1, 3) * m(2, 2) * m(3, 1) + m(1, 3) * m(2, 1) * m(3, 2) - m(1, 1) * m(2, 3) * m(3, 2) - m(1, 2) * m(2, 1) * m(3, 3) + m(1, 1) * m(2, 2) * m(3, 3);
        result(0, 1) = m(0, 3) * m(2, 2) * m(3, 1) - m(0, 2) * m(2, 3) * m(3, 1) - m(0, 3) * m(2, 1) * m(3, 2) + m(0, 1) * m(2, 3) * m(3, 2) + m(0, 2) * m(2, 1) * m(3, 3) - m(0, 1) * m(2, 2) * m(3, 3);
        result(0, 2) = m(0, 2) * m(1, 3) * m(3, 1) - m(0, 3) * m(1, 2) * m(3, 1) + m(0, 3) * m(1, 1) * m(3, 2) - m(0, 1) * m(1, 3) * m(3, 2) - m(0, 2) * m(1, 1) * m(3, 3) + m(0, 1) * m(1, 2) * m(3, 3);
        result(0, 3) = m(0, 3) * m(1, 2) * m(2, 1) - m(0, 2) * m(1, 3) * m(2, 1) - m(0, 3) * m(1, 1) * m(2, 2) + m(0, 1) * m(1, 3) * m(2, 2) + m(0, 2) * m(1, 1) * m(2, 3) - m(0, 1) * m(1, 2) * m(2, 3);
        result(1, 0) = m(1, 3) * m(2, 2) * m(3, 0) - m(1, 2) * m(2, 3) * m(3, 0) - m(1, 3) * m(2, 0) * m(3, 2) + m(1, 0) * m(2, 3) * m(3, 2) + m(1, 2) * m(2, 0) * m(3, 3) - m(1, 0) * m(2, 2) * m(3, 3);
        result(1, 1) = m(0, 2) * m(2, 3) * m(3, 0) - m(0, 3) * m(2, 2) * m(3, 0) + m(0, 3) * m(2, 0) * m(3, 2) - m(0, 0) * m(2, 3) * m(3, 2) - m(0, 2) * m(2, 0) * m(3, 3) + m(0, 0) * m(2, 2) * m(3, 3);
        result(1, 2) = m(0, 3) * m(1, 2) * m(3, 0) - m(0, 2) * m(1, 3) * m(3, 0) - m(0, 3) * m(1, 0) * m(3, 2) + m(0, 0) * m(1, 3) * m(3, 2) + m(0, 2) * m(1, 0) * m(3, 3) - m(0, 0) * m(1, 2) * m(3, 3);
        result(1, 3) = m(0, 2) * m(1, 3) * m(2, 0) - m(0, 3) * m(1, 2) * m(2, 0) + m(0, 3) * m(1, 0) * m(2, 2) - m(0, 0) * m(1, 3) * m(2, 2) - m(0, 2) * m(1, 0) * m(2, 3) + m(0, 0) * m(1, 2) * m(2, 3);
        result(2, 0) = m(1, 1) * m(2, 3) * m(3, 0) - m(1, 3) * m(2, 1) * m(3, 0) + m(1, 3) * m(2, 0) * m(3, 1) - m(1, 0) * m(2, 3) * m(3, 1) - m(1, 1) * m(2, 0) * m(3, 3) + m(1, 0) * m(2, 1) * m(3, 3);
        result(2, 1) = m(0, 3) * m(2, 1) * m(3, 0) - m(0, 1) * m(2, 3) * m(3, 0) - m(0, 3) * m(2, 0) * m(3, 1) + m(0, 0) * m(2, 3) * m(3, 1) + m(0, 1) * m(2, 0) * m(3, 3) - m(0, 0) * m(2, 1) * m(3, 3);
        result(2, 2) = m(0, 1) * m(1, 3) * m(3, 0) - m(0, 3) * m(1, 1) * m(3, 0) + m(0, 3) * m(1, 0) * m(3, 1) - m(0, 0) * m(1, 3) * m(3, 1) - m(0, 1) * m(1, 0) * m(3, 3) + m(0, 0) * m(1, 1) * m(3, 3);
        result(2, 3) = m(0, 3) * m(1, 1) * m(2, 0) - m(0, 1) * m(1, 3) * m(2, 0) - m(0, 3) * m(1, 0) * m(2, 1) + m(0, 0) * m(1, 3) * m(2, 1) + m(0, 1) * m(1, 0) * m(2, 3) - m(0, 0) * m(1, 1) * m(2, 3);
        result(3, 0) = m(1, 2) * m(2, 1) * m(3, 0) - m(1, 1) * m(2, 2) * m(3, 0) - m(1, 2) * m(2, 0) * m(3, 1) + m(1, 0) * m(2, 2) * m(3, 1) + m(1, 1) * m(2, 0) * m(3, 2) - m(1, 0) * m(2, 1) * m(3, 2);
        result(3, 1) = m(0, 1) * m(2, 2) * m(3, 0) - m(0, 2) * m(2, 1) * m(3, 0) + m(0, 2) * m(2, 0) * m(3, 1) - m(0, 0) * m(2, 2) * m(3, 1) - m(0, 1) * m(2, 0) * m(3, 2) + m(0, 0) * m(2, 1) * m(3, 2);
        result(3, 2) = m(0, 2) * m(1, 1) * m(3, 0) - m(0, 1) * m(1, 2) * m(3, 0) - m(0, 2) * m(1, 0) * m(3, 1) + m(0, 0) * m(1, 2) * m(3, 1) + m(0, 1) * m(1, 0) * m(3, 2) - m(0, 0) * m(1, 1) * m(3, 2);
        result(3, 3) = m(0, 1) * m(1, 2) * m(2, 0) - m(0, 2) * m(1, 1) * m(2, 0) + m(0, 2) * m(1, 0) * m(2, 1) - m(0, 0) * m(1, 2) * m(2, 1) - m(0, 1) * m(1, 0) * m(2, 2) + m(0, 0) * m(1, 1) * m(2, 2);

        T det = determinant(m);
        det = T(1) / det;
        result *= det;

        return result;
    }

    /*----------------------------------------------------------------------------*/
    template <size_t M, size_t N, typename T>
    inline Mat<N, M, T> tensor(const Vec<M, T> &u, const Vec<N, T> &v) {
        Mat<N, 1, T> mu(u); // column vector u
        Mat<1, M, T> mv(v); // row vector v
        return mu * mv; // use matrix multiplication
    }

    /*----------------------------------------------------------------------------*/
    /*	Adapted from "Numerical Recipes in C" (Press et al.) */
    template <size_t N, size_t M, typename T>
    inline bool gauss_jordan_elimination(
        const Mat<N, N, T> &a,
        const Mat<N, M, T> &b,
        Mat<N, N, T> *ainv,
        Mat<N, M, T> *x) {

        size_t indxc[N], indxr[N], ipiv[N];

        Mat<N, N, T> &amat = *ainv;
        amat = a; // copy A
        Mat<N, M, T> &bmat = *x;
        bmat = b; // copy B

        for (size_t i = 0; i < N; ++i)
            ipiv[i] = 0;

        for (size_t i = 0; i < N; ++i) { // for each column
            T max = T(0);
            size_t maxc, maxr;
            for (size_t j = 0; j < N; ++j) { // search for pivot
                if (ipiv[j] != 1) {
                    for (size_t k = 0; k < N; ++k) { // for each column
                        if (ipiv[k] == 0) {
                            T element = std::abs(amat(j, k));
                            if (element > max) {
                                max = element;
                                maxr = j;
                                maxc = k;
                            }
                        }
                    }
                }
            }
            ++ipiv[maxc];

            if (maxr != maxc) {
                amat.swap_rows(maxr, maxc);
                bmat.swap_rows(maxr, maxc);
            }

            indxr[i] = maxr;
            indxc[i] = maxc;

            //	check for singular matrix:
            if (std::abs(amat(maxc, maxc)) < epsilon<T>())
                return false;

            //	multiply row by 1/pivot:
            T rpivot = T(1) / amat(maxc, maxc);
            amat(maxc, maxc) = T(1);
            amat.set_row(maxc, amat.row(maxc) * rpivot);
            bmat.set_row(maxc, bmat.row(maxc) * rpivot);

            //	reduce rows (except pivot):
            for (size_t j = 0; j < N; ++j) {
                if (j != maxc) {
                    T dum = amat(j, maxc);
                    amat(j, maxc) = T(0);
                    for (size_t k = 0; k < N; ++k)
                        amat(j, k) -= amat(maxc, k) * dum;
                    for (size_t k = 0; k < M; ++k)
                        bmat(j, k) -= bmat(maxc, k) * dum;
                }
            }
        }

        //	unscramble a by interchanging columns:
        size_t i = N;
        do {
            --i;
            if (indxr[i] != indxc[i]) // if swap occurred
                amat.swap_cols(indxr[i], indxc[i]);
        } while (i != 0);

        return true;
    }

    /*----------------------------------------------------------------------------*/
    /*	Adapted from "Numerical Recipes in C" (Press et al.) */
    template <size_t N, typename T>
    inline bool lu_decomposition(
        const Mat<N, N, T> &a,
        Mat<N, N, T> *alu,
        Vec<N, T> *rowp,
        T *d) {

        Vec<N, T> scalev; // stores implicit scaling of each row
        *d = T(1); // no rows changed
        Mat<N, N, T> &amat = *alu;
        amat = a; // copy a

        for (size_t i = 0; i < N; ++i) { // for each row
            //	get implicit scaling:
            T max = T(0);
            for (size_t j = 0; j < N; ++j) {
                T element = std::abs(amat(i, j));
                if (element > max)
                    max = element;
            }

            //	check for singular matrix:
            if (std::abs(max) < min<T>())
                return false;

            scalev[i] = T(1) / max; // save scaling factor
        }

        for (size_t j = 0; j < N; ++j) { // for each column
            for (size_t i = 0; i < j; ++i) {
                T sum = amat(i, j);
                for (size_t k = 0; k < i; ++k)
                    sum -= amat(i, k) * amat(k, j);
                amat(i, j) = sum;
            }

            T max = T(0);
            size_t imax;
            for (size_t i = j; i < N; ++i) {
                T sum = amat(i, j);
                for (size_t k = 0; k < j; ++k)
                    sum -= amat(i, k) * amat(k, j);
                amat(i, j) = sum;

                T dum = scalev[i] * std::abs(sum);
                if (dum >= max) {
                    max = dum;
                    imax = i;
                }
            }

            if (j != imax) { // if we need to swap rows
                amat.swap_rows(imax, j);
                scalev[imax] = scalev[j]; // also swap scale factor
                *d = -(*d); // change parity of d
            }
            (*rowp)[j] = imax;

            //	check for singular matrix:
            if (std::abs(amat(j, j)) < epsilon<T>())
                return false;

            //	divide by the pivot element:
            if (j != N) {
                T dum = T(1) / amat(j, j);
                for (size_t i = j + 1; i < N; ++i)
                    amat(i, j) *= dum;
            }

        }

        return true;
    }

    /*----------------------------------------------------------------------------*/
    /*	Adapted from "Numerical Recipes in C" (Press et al.) */
    template <size_t N, typename T>
    inline void lu_back_substitution(
        const Mat<N, N, T> &alu,
        const Vec<N, T> &rowp,
        const Vec<N, T> &b,
        Vec<N, T> *x
        )
    {
        Vec<N, T> &result = *x;
        result = b; // copy b to result

        size_t ii = 0;
        for (size_t i = 0; i < N; ++i) {
            size_t ip = rowp[i];
            assert(ip < N);

            T sum = result[ip];
            result[ip] = result[i];
            if (ii != 0) {
                for (size_t j = ii - 1; j < i; ++j)
                    sum -= alu(i, j) * result[j];
            }
            else if (std::abs(sum) > epsilon<T>()) {
                ii = i + 1;
            }
            result[i] = sum;
        }

        size_t i = N;
        do {
            --i;
            T sum = result[i];
            for (size_t j = i + 1; j < N; ++j)
                sum -= alu(i, j) * result[j];
            result[i] = sum / alu(i, i);
        } while (i != 0);
    }


    /*----------------------------------------------------------------------------*/
    // Adapted from Template Numerical Toolkit.
    template<size_t N, typename FT>
    bool cholesky_decompose(const Mat<N, N, FT> &A, Mat<N, N, FT> &L) {
        bool spd = true;
        for (size_t j = 0; j < N; ++j) {
            FT d = 0;
            for (size_t k = 0; k < j; ++k) {
                FT s = 0;
                for (size_t i = 0; i < k; ++i)
                    s += L(k, i) * L(j, i);

                L(j, k) = s = (A(j, k) - s) / L(k, k);
                d = d + s * s;
                spd = spd && (A(k, j) == A(j, k));
            }

            d = A(j, j) - d;
            spd = spd && (d > 0);

            L(j, j) = std::sqrt(d > 0 ? d : 0);
            for (size_t k = j + 1; k < N; ++k)
                L(j, k) = 0;
        }
        return spd;
    }

    /*----------------------------------------------------------------------------*/
    // Adapted from Template Numerical Toolkit.
    template <size_t N, typename FT>
    void cholesky_solve(const Mat<N, N, FT> &L, const Vec<N, FT> &b, Vec<N, FT>& x) {
        x = b;
        // solve L*y = b
        for (size_t k = 0; k < N; ++k) {
            for (size_t i = 0; i < k; ++i)
                x[k] -= x[i] * L(k, i);
            x[k] /= L(k, k);
        }

        // solve L'*x = y
        for (int k = N - 1; k >= 0; --k) {  // attention: size_t will not work
            for (size_t i = k + 1; i < N; ++i)
                x[k] -= x[i] * L(i, k);
            x[k] /= L(k, k);
        }
    }

    template<size_t N, size_t M, typename T>
    void cholesky_solve(const Mat<N, N, T> &L, const Mat<N, M, T> &B, Mat<N, M, T> &X) {
        X = B;
        // solve L_*Y = B
        for (size_t j = 0; j < M; ++j) {
            for (size_t k = 0; k < N; ++k) {
                for (size_t i = 0; i < k; ++i)
                    X(k, j) -= X(i, j) * L(k, i);

                X(k, j) /= L(k, k);
            }
        }

        // solve L'*X = Y
        for (size_t j = 0; j < M; ++j) {
            for (int k = N - 1; k >= 0; --k) {  // attention: size_t will not work
                for (size_t i = k + 1; i < N; ++i)
                    X(k, j) -= X(i, j) * L(i, k);

                X(k, j) /= L(k, k);
            }
        }
    }

    /*----------------------------------------------------------------------------*/

    /** Output stream support for Mat. **/
    template <size_t N, size_t M, typename T> inline
    std::ostream& operator<< (std::ostream& output, const Mat<N, M, T>& m) {
        output << std::fixed << std::setprecision(8);
        const char sep = ' ';
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                output << sep  << std::setw(7) << m(i, j);
            }
            output << std::endl;
        }
        output << std::resetiosflags(std::ios_base::fixed | std::ios_base::floatfield);
        return output;
    }

    /*----------------------------------------------------------------------------*/
    /** Input stream support for Mat. **/
    template <size_t N, size_t M, typename T>
    std::istream& operator>> (std::istream& input, Mat<N, M, T>& m) {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                input >> m(i, j);
            }
        }
        return input;
    }


    /*----------------------------------------------------------------------------*/

    /** \brief Convert a N-dimensional vector into a N by 1 matrix. */
    template<size_t N, typename FT>
    Mat<N, 1, FT> to_matrix(const Vec<N, FT>& v) {
        return Mat<N, 1, FT>(v.data());
    }


    /**	\brief Construct a 1 by N matrix from a N-dimensional vector. */
    template<size_t N, typename FT>
    Mat<1, N, FT> transpose(const Vec<N, FT>& v) {
        return Mat<1, N, FT>(v.data());
    }

    /*----------------------------------------------------------------------------*/

    /**	\brief Test if a matrix has NaN entry. */
    template <size_t N, size_t M, typename T>
    inline bool has_nan(const Mat<N, M, T>& m) {
        for (int i = 0; i < N; i++) {
            for (int j = 0; j < M; j++) {
                if (std::isnan(m(i, j)) || std::isinf(m(i, j)))
                    return true;
            }
        }
        return false;
    }


    /*******************************************************************************

    --------------------------------  2x2 matrix  ----------------------------------

    *******************************************************************************/

    /**
     * \brief 2 by 2 matrix. Extends Mat with 2D-specific functionality and constructors.
     * \tparam T The scalar type for vector elements.
     * \see Mat.
     * \class Mat2 easy3d/core/mat.h
     */
    template <typename T>
    class Mat2 : public Mat<2, 2, T>
    {
    public:
        /**
         * \brief Default constructor.
         * \note The matrix elements are intentionally not initialized. This is efficient
         *       if the user assigns their values from subsequent computations. Use Mat2(T s)
         *       to initialize the elements during construction.
         */
        Mat2() = default;

        /**	\brief Initialized with diagonal as s and others zeros.*/
        explicit Mat2(T s);

        /**	\brief Copy constructor. This provides compatibility with generic operations implemented by Mat. */
        Mat2(const Mat<2, 2, T> &rhs);

        /**
         * \brief Copies the top-left corner of rhs. This provides compatibility with generic operations implemented
         * by Mat.
         */
        explicit Mat2(const Mat<3, 3, T> &rhs);

        /**
         * \brief Initialize elements from individual scalars. The digits following s in the parameter names indicate
         * the row/column of the element being set.
         */
        Mat2(
            T s00, T s01,
            T s10, T s11
            );

        /**	\brief Initialize elements from an array of type T. */
        explicit Mat2(const T *m);

        /**
         * \brief Initialize elements from two vectors. If MATRIX_ROW_MAJOR is defined, x and y specify rows of the
         * matrix, else columns.
         */
        Mat2(
            const Vec<2, T> &x,
            const Vec<2, T> &y
            );

        /**
         * \brief Static constructor return a 2D rotation matrix.
         * \param angle Angle of rotation in radians.
         * \note Positive values of angle rotate counter-clockwise as per the right-hand rule.
         */
        static Mat2<T> rotation(T angle);

        /**
         * \brief Static constructor return a 2D uniform scale matrix.
         * @param s x, y scale (uniform).
         */
        static Mat2<T> scale(T s);

        /**
         * \brief Static constructor return a 2D non-uniform scale matrix.
         * @param x x scale.
         * @param y y scale.
         */
        static Mat2<T> scale(T x, T y);

    }; // class Mat2

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat2<T>::Mat2(T s) {
        for (size_t i = 0; i < 2; ++i) {
            for (size_t j = 0; j < 2; ++j)
                if (i == j)
                    (*this)(i, j) = T(s);
                else
                    (*this)(i, j) = T(0);
        }
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat2<T>::Mat2(const Mat<2, 2, T> &rhs) {
        for (size_t i = 0; i < 4; ++i)
            (*this).m_[i] = rhs[i];
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat2<T>::Mat2(const Mat<3, 3, T> &rhs) {
        (*this)(0, 0) = rhs(0, 0); (*this)(0, 1) = rhs(0, 1);
        (*this)(1, 0) = rhs(1, 0); (*this)(1, 1) = rhs(1, 1);
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat2<T>::Mat2(
        T s00, T s01,
        T s10, T s11
        ) {
        (*this)(0, 0) = s00; (*this)(0, 1) = s01;
        (*this)(1, 0) = s10; (*this)(1, 1) = s11;
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat2<T>::Mat2(const T *m) {
        assert(m != 0);

    #ifdef MATRIX_ROW_MAJOR
        (*this)(0, 0) = m[0]; (*this)(0, 1) = m[1];
        (*this)(1, 0) = m[2]; (*this)(1, 1) = m[3];
    #else
        (*this)(0, 0) = m[0]; (*this)(0, 1) = m[2];
        (*this)(1, 0) = m[1]; (*this)(1, 1) = m[3];
    #endif
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat2<T>::Mat2(const Vec<2, T> &x, const Vec<2, T> &y) {
    #ifdef MATRIX_ROW_MAJOR
        (*this).set_row(0, x);
        (*this).set_row(1, y);
    #else
        (*this).set_col(0, x);
        (*this).set_col(1, y);
    #endif
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat2<T> Mat2<T>::rotation(T angle) {
        return Mat2<T>(
            std::cos(angle), -std::sin(angle),
            std::sin(angle),  std::cos(angle)
            );
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat2<T> Mat2<T>::scale(T s) {
        return Mat2<T>(
            s, T(0),
            T(0), s
            );
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat2<T> Mat2<T>::scale(T x, T y) {
        return Mat2<T>(
            x, T(0),
            T(0), y
            );
    }

    /*******************************************************************************

    --------------------------------  3x3 matrix  ----------------------------------

    *******************************************************************************/

    template <typename T>	class Quat;

    /**
     * \brief 3x3 matrix. Extends Mat with 3D-specific functionality and constructors.
     * \tparam T The scalar type for vector elements. \see Mat.
     * \class Mat3 easy3d/core/mat.h
     */
    template <typename T>
    class Mat3 : public Mat<3, 3, T> {
    public:
        /**
         * \brief Default constructor.
         * \note The matrix elements are intentionally not initialized. This is efficient
         *       if the user assigns their values from subsequent computations. Use Mat3(T s)
         *       to initialize the elements during construction.
         */
        Mat3() = default;

        /**	\brief Initialized with diagonal as s and others zeros.*/
        explicit Mat3(T s);

        /** \brief Copy constructor. This provides compatibility with generic operations implemented by Mat. */
        Mat3(const Mat<3, 3, T> &rhs);

        /**
         * \brief Copies the top-left corner of rhs. This provides compatibility with generic operations implemented
         * by Mat.
         */
        explicit Mat3(const Mat<4, 4, T> &rhs);

        /**
         * \brief Initialize elements from individual scalars. The digits following s in the parameter names indicate
         * the row/column of the element being set.
         */
        Mat3(
            T s00, T s01, T s02,
            T s10, T s11, T s12,
            T s20, T s21, T s22
            );

        /**	Initialize elements from an array of type T. */
        explicit Mat3(const T *m);

        /**
         * \brief Initialize elements from three vectors. If MATRIX_ROW_MAJOR is defined, x, y and z specify rows of
         * the matrix, else columns.
         */
        Mat3(
            const Vec<3, T> &x,
            const Vec<3, T> &y,
            const Vec<3, T> &z
            );

        /**
         * Initialize from a 2D matrix. rhs forms the upper-left 2x2 sub-matrix, other elements are initialized as the
         * identity.
         */
        explicit Mat3(const Mat<2, 2, T> &rhs);

        /** \brief the upper-left 2x2 sub-matrix. */
        Mat2<T> sub() const;

        /**
         * \brief Static constructor returning a 3D uniform scale matrix.
         * \param s x, y, z scale (uniform).
         */
        static Mat3<T> scale(T s);

        /**
         * \brief Static constructor returning a 3D non-uniform scale matrix.
         * @param x x scale.
         * @param y y scale.
         * @param z z scale.
         */
        static Mat3<T> scale(T x, T y, T z);

        /**
         * \brief Static constructor returning a 3D rotation matrix defined by its axis and angle.
         * @param axis: Axis of rotation. This MUST be normalized.
         * @param angle: Angle of rotation in radians. Positive values of angle
         *      rotate counter-clockwise about axis as per the right-hand rule.
         * \note The axis defines only the direction of the rotation axis, i.e., the
         *      rotation is about the axis passing through the origin.
         */
        static Mat3<T> rotation(const Vec<3, T> &axis, T angle);
        
        /**
         * \brief Static constructor returning a 3D rotation matrix defined by the axis–angle
         *      representation. Both the axis and the angle are represented by a vector
         *      codirectional with the rotation axis whose length is the rotation angle.
         * @param axis_angle: direction is the axis and length is the angle (in radian)
         * \note The axis defines only the direction of the rotation axis, i.e., the
         *      rotation is about the axis passing through the origin.
         */
        static Mat3<T> rotation(const Vec<3, T> &axis_angle);
        
        /**
         * \brief Static constructor return a 3D rotation matrix defined by a quaternion.
         * \note q is a unit quaternion representing a rotation. */
        static Mat3<T> rotation(const Quat<T> &q);
        
        /**
         * \brief Static constructor returning a 3D rotation matrix defined by Euler angles.
         *      The three rotations are applied successively.
         * @param x, y, z: the rotation angles (in radians) around X, Y, and Z axes respectively.
         * @param order: the order of the rotations to be applied. 1 first and 3 for last.
         * \note Using a different order yields different results. The default order is first Y, then Z, then X.
         */
        static Mat3<T> rotation(T x, T y, T z, int order = 312);

    }; // class Mat3


    /*******************************************************************************

    IMPLEMENTATION:

    *******************************************************************************/

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat3<T>::Mat3(T s) {
        for (size_t i = 0; i < 3; ++i) {
            for (size_t j = 0; j < 3; ++j)
                if (i == j)
                    (*this)(i, j) = T(s);
                else
                    (*this)(i, j) = T(0);
        }
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat3<T>::Mat3(const Mat<3, 3, T> &rhs) {
        for (size_t i = 0; i < 9; ++i)
            (*this).m_[i] = rhs[i];
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat3<T>::Mat3(const Mat<4, 4, T> &rhs) {
        (*this)(0, 0) = rhs(0, 0); (*this)(0, 1) = rhs(0, 1); (*this)(0, 2) = rhs(0, 2);
        (*this)(1, 0) = rhs(1, 0); (*this)(1, 1) = rhs(1, 1); (*this)(1, 2) = rhs(1, 2);
        (*this)(2, 0) = rhs(2, 0); (*this)(2, 1) = rhs(2, 1); (*this)(2, 2) = rhs(2, 2);
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat3<T>::Mat3(
        T s00, T s01, T s02,
        T s10, T s11, T s12,
        T s20, T s21, T s22
        ) {
        (*this)(0, 0) = s00; (*this)(0, 1) = s01; (*this)(0, 2) = s02;
        (*this)(1, 0) = s10; (*this)(1, 1) = s11; (*this)(1, 2) = s12;
        (*this)(2, 0) = s20; (*this)(2, 1) = s21; (*this)(2, 2) = s22;
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat3<T>::Mat3(const T *m) {
        assert(m != 0);

    #ifdef MATRIX_ROW_MAJOR
        (*this)(0, 0) = m[0]; (*this)(0, 1) = m[1]; (*this)(0, 2) = m[2];
        (*this)(1, 0) = m[3]; (*this)(1, 1) = m[4]; (*this)(1, 2) = m[5];
        (*this)(2, 0) = m[6]; (*this)(2, 1) = m[7]; (*this)(2, 2) = m[8];
    #else
        (*this)(0, 0) = m[0]; (*this)(0, 1) = m[3]; (*this)(0, 2) = m[6];
        (*this)(1, 0) = m[1]; (*this)(1, 1) = m[4]; (*this)(1, 2) = m[7];
        (*this)(2, 0) = m[2]; (*this)(2, 1) = m[5]; (*this)(2, 2) = m[8];
    #endif
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat3<T>::Mat3(const Vec<3, T> &x, const Vec<3, T> &y, const Vec<3, T> &z) {
    #ifdef MATRIX_ROW_MAJOR
        (*this).set_row(0, x);
        (*this).set_row(1, y);
        (*this).set_row(2, z);
    #else
        (*this).set_col(0, x);
        (*this).set_col(1, y);
        (*this).set_col(2, z);
    #endif
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat3<T>::Mat3(const Mat<2, 2, T> &rhs) {
        (*this)(0, 0) = rhs(0, 1);	(*this)(0, 0) = rhs(0, 1);	(*this)(0, 2) = T(0);
        (*this)(1, 0) = rhs(1, 1);	(*this)(1, 0) = rhs(1, 1);	(*this)(1, 2) = T(0);
        (*this)(2, 0) = T(0);		(*this)(2, 0) = T(0);		(*this)(2, 2) = T(1);
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat2<T> Mat3<T>::sub() const {
        Mat2<T> mat;
        for (size_t i = 0; i < 2; i++) {
            for (size_t j = 0; j < 2; j++) {
                mat(i, j) = (*this)(i, j);
            }
        }
        return mat;
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat3<T> Mat3<T>::scale(T s) {
        return Mat3<T>(
            s,    T(0), T(0),
            T(0), s,    T(0),
            T(0), T(0), s
            );
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat3<T> Mat3<T>::scale(T x, T y, T z) {
        return Mat3<T>(
            x,    T(0), T(0),
            T(0), y,    T(0),
            T(0), T(0), z
            );
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat3<T> Mat3<T>::rotation(const Vec<3, T> &axis, T angle) {
        assert(std::abs(axis.length() - 1) < epsilon<T>());

        //	cross-product matrix of axis:
        const Mat3<T> cpm(
             T(0),     -axis[2],   axis[1],
             axis[2],   T(0),     -axis[0],
            -axis[1],   axis[0],   T(0)
            );

        //	axis-axis tensor product:
        const Mat3<T> tpm = tensor(axis, axis);

        //	trig coefficients:
        const T c = std::cos(angle);
        const T rc = T(1) - c;
        const T s = std::sin(angle);

        return Mat3<T>::identity() * c + cpm * s + tpm * rc;
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat3<T> Mat3<T>::rotation(const Vec<3, T> &axis_angle) {
        const T len = axis_angle.length();
        return rotation(axis_angle/len, len);
    }
    
    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat3<T> Mat3<T>::rotation(const Quat<T> &q) {
        // input must be unit quaternion
        assert(std::abs(q.length() - 1) < epsilon<T>());
        const T x = q.x;
        const T y = q.y;
        const T z = q.z;
        const T w = q.w;
        Mat3<T> m;
        m(0, 0) = 1.0 - 2.0 * (y*y + z*z);    m(0, 1) = 2.0 * (x*y - w*z);          m(0, 2) = 2.0 * (x*z + w*y);
        m(1, 0) = 2.0 * (x*y + w*z);          m(1, 1) = 1.0 - 2.0 * (x*x + z*z);    m(1, 2) = 2.0 * (y*z - w*x);
        m(2, 0) = 2.0 * (x*z - w*y);          m(2, 1) = 2.0 * (y*z + w*x);          m(2, 2) = 1.0 - 2.0 * (x*x + y*y);
        return m;
    }
    
    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat3<T> Mat3<T>::rotation(T x, T y, T z, int order) {
        // The code is not optimized. The final matrix can be directly derived.
        // http://www.songho.ca/opengl/gl_anglestoaxes.html
        // http://inside.mines.edu/fs_home/gmurray/ArbitraryAxisRotation/
        Mat3<T> rx; // Rotation about X-axis (Pitch)
        rx(0, 0) = T(1);         rx(0, 1) = T(0);           rx(0, 2) = T(0);
        rx(1, 0) = T(0);         rx(1, 1) = std::cos(x);    rx(1, 2) = -std::sin(x);
        rx(2, 0) = T(0);         rx(2, 1) = std::sin(x);    rx(2, 2) = std::cos(x);
 
        Mat3<T> ry; // Rotation about Y-axis (Yaw, Heading)
        ry(0, 0) = std::cos(y);  ry(0, 1) = T(0);           ry(0, 2) = std::sin(y);
        ry(1, 0) = T(0);         ry(1, 1) = T(1);           ry(1, 2) = T(0);
        ry(2, 0) = -std::sin(y); ry(2, 1) = T(0);           ry(2, 2) = std::cos(y);
        
        Mat3<T> rz; // Rotation about Z-axis (Roll)
        rz(0, 0) = std::cos(z);  rz(0, 1) = -std::sin(z);   rz(0, 2) = T(0);
        rz(1, 0) = std::sin(z);  rz(1, 1) = std::cos(z);    rz(1, 2) = T(0);
        rz(2, 0) = T(0);         rz(2, 1) = T(0);           rz(2, 2) = T(1);
        
        switch (order) {
            case 123:   return rz * ry * rx;
            case 132:   return ry * rz * rx;
            case 213:   return rz * rx * ry;
            case 231:   return rx * rz * ry;
            case 312:   return ry * rx * rz;
            case 321:   return rx * ry * rz;
            default:
                LOG(ERROR) << "invalid rotation order";
                return rx * rz * ry;
        }
    }

    /*******************************************************************************

    --------------------------------  4x4 matrix  ----------------------------------

    *******************************************************************************/

    /**
     * \brief 4x4 matrix. Extends Mat with 4D-specific functionality and constructors.
     * \tparam T The scalar type for vector elements. \see Mat.
     * \class Mat4 easy3d/core/mat.h
     */
    template <typename T>
    class Mat4 : public Mat<4, 4, T> {
    public:
        /**
         * \brief Default constructor.
         * \note The matrix elements are intentionally not initialized. This is efficient
         *       if the user assigns their values from subsequent computations. Use Mat4(T s)
         *       to initialize the elements during construction.
         */
        Mat4() = default;

        /**	\brief Initialized with diagonal as s and others zeros.*/
        explicit Mat4(T s);

        /**	\brief Copy constructor. This provides compatibility with generic operations implemented by Mat. */
        Mat4(const Mat<4, 4, T> &rhs);

        /**
         * \brief Initialize elements from individual scalars. The digits following s in the parameter names indicate
         * the row/column of the element being set.
         */
        Mat4(
            T s00, T s01, T s02, T s03,
            T s10, T s11, T s12, T s13,
            T s20, T s21, T s22, T s23,
            T s30, T s31, T s32, T s33
            );

        /**	\brief Initialize elements from an array of type T. */
        explicit Mat4(const T *m);

        /**
         * \brief Initialize elements from four vectors. If MATRIX_ROW_MAJOR is defined, x, y, z and w specify rows of
         * the matrix, else columns.
         */
        Mat4(
            const Vec<4, T> &x,
            const Vec<4, T> &y,
            const Vec<4, T> &z,
            const Vec<4, T> &w
            );

        /**
         * \brief Initialize from a 3D matrix. rhs forms the upper-left 3x3 sub-matrix, other elements are initialized
         * as the identity.
         */
        explicit Mat4(const Mat<3, 3, T> &rhs);

        /**	\brief Initialize from scale/rotation/translation. */
        Mat4(const Vec<3, T> &s, const Quat<T> &r, const Vec<3, T> &t);

        // the upper-left 3x3 sub-matrix,
        Mat3<T> sub() const;

        /**
         * \brief Static constructor returning a 4D uniform scale matrix.
         * \param s x, y, z, w scale (uniform).
         */
        static Mat4<T> scale(T s);

        /**
         * \brief Static constructor returning a 4D non-uniform scale matrix,
         * \param x x scale.
         * \param y y scale.
         * \param z z scale.
         * \param w w scale.
         */
        static Mat4<T> scale(const Vec<4, T>& s);  // set w to 1 for 3D scaling
        static Mat4<T> scale(T x, T y, T z, T w);  // set w to 1 for 3D scaling
        
        /**
         * \brief Static constructor returning a 3D rotation matrix defined by its axis and angle.
         * \param axis: Axis of rotation. This MUST be normalized.
         * \param angle: Angle of rotation in radians. Positive values of angle
         *      rotate counter-clockwise about axis as per the right-hand rule.
         * \note The axis defines only the direction of the rotation axis, i.e., the
         *      rotation is about the axis passing through the origin.
         */
        static Mat4<T> rotation(const Vec<3, T> &axis, T angle);
        
        /**
         * \brief Static constructor returning a 3D rotation matrix defined by the axis–angle
         *      representation. Both the axis and the angle are represented by a vector
         *      codirectional with the rotation axis whose length is the rotation angle.
         * \param axis_angle: direction is the axis and length is the angle (in radian)
         * \note The axis defines only the direction of the rotation axis, i.e., the
         *      rotation is about the axis passing through the origin.
         */
        static Mat4<T> rotation(const Vec<3, T> &axis_angle);

        /**
         * \brief Static constructor returning a 3D rotation matrix defined by a quaternion.
         * \note q is a unit quaternion representing a rotation.
         */
        static Mat4<T> rotation(const Quat<T> &q);

        /**
         * \brief Static constructor returning a 3D rotation matrix defined by Euler angles.
         *      The three rotations are applied successively.
         * \param x, y, z: the rotation angles (in radians) around X, Y, and Z axes respectively.
         * \param order: the order of the rotations to be applied. 1 first and 3 for last.
         * \note Using a different order yields different results. The default order is first Y, then Z, then X.
         *      See http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-17-quaternions/
         */
        static Mat4<T> rotation(T x, T y, T z, int order = 312);

        /**
         * \brief Static constructor return a 3D translation matrix (as a 4D affine transformation).
         * \param t Translation vector.
         */
        static Mat4<T> translation(const Vec<3, T> &t);
        static Mat4<T> translation(T x, T y, T z);

    }; // class Mat4


    /*******************************************************************************

    IMPLEMENTATION:

    *******************************************************************************/

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat4<T>::Mat4(T s) {
        for (size_t i = 0; i < 4; ++i) {
            for (size_t j = 0; j < 4; ++j)
                if (i == j)
                    (*this)(i, j) = T(s);
                else
                    (*this)(i, j) = T(0);
        }
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat4<T>::Mat4(const Mat<4, 4, T> &rhs) {
        for (size_t i = 0; i < 16; ++i)
            (*this).m_[i] = rhs[i];
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat4<T>::Mat4(
        T s00, T s01, T s02, T s03,
        T s10, T s11, T s12, T s13,
        T s20, T s21, T s22, T s23,
        T s30, T s31, T s32, T s33
        ) {
        (*this)(0, 0) = s00; (*this)(0, 1) = s01; (*this)(0, 2) = s02; (*this)(0, 3) = s03;
        (*this)(1, 0) = s10; (*this)(1, 1) = s11; (*this)(1, 2) = s12; (*this)(1, 3) = s13;
        (*this)(2, 0) = s20; (*this)(2, 1) = s21; (*this)(2, 2) = s22; (*this)(2, 3) = s23;
        (*this)(3, 0) = s30; (*this)(3, 1) = s31; (*this)(3, 2) = s32; (*this)(3, 3) = s33;
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat4<T>::Mat4(const T *m) {
        assert(m != 0);

    #ifdef MATRIX_ROW_MAJOR
        (*this)(0, 0) = m[0];  (*this)(0, 1) = m[1];  (*this)(0, 2) = m[2];  (*this)(0, 3) = m[3];
        (*this)(1, 0) = m[4];  (*this)(1, 1) = m[5];  (*this)(1, 2) = m[6];  (*this)(1, 3) = m[7];
        (*this)(2, 0) = m[8];  (*this)(2, 1) = m[9];  (*this)(2, 2) = m[10]; (*this)(2, 3) = m[11];
        (*this)(3, 0) = m[12]; (*this)(3, 1) = m[13]; (*this)(3, 2) = m[14]; (*this)(3, 3) = m[15];
    #else
        (*this)(0, 0) = m[0]; (*this)(0, 1) = m[4]; (*this)(0, 2) = m[8];  (*this)(0, 3) = m[12];
        (*this)(1, 0) = m[1]; (*this)(1, 1) = m[5]; (*this)(1, 2) = m[9];  (*this)(1, 3) = m[13];
        (*this)(2, 0) = m[2]; (*this)(2, 1) = m[6]; (*this)(2, 2) = m[10]; (*this)(2, 3) = m[14];
        (*this)(3, 0) = m[3]; (*this)(3, 1) = m[7]; (*this)(3, 2) = m[11]; (*this)(3, 3) = m[15];
    #endif
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat4<T>::Mat4(
        const Vec<4, T> &x,
        const Vec<4, T> &y,
        const Vec<4, T> &z,
        const Vec<4, T> &w
        ) {

    #ifdef MATRIX_ROW_MAJOR
        (*this).set_row(0, x);
        (*this).set_row(1, y);
        (*this).set_row(2, z);
        (*this).set_row(3, w);
    #else
        (*this).set_col(0, x);
        (*this).set_col(1, y);
        (*this).set_col(2, z);
        (*this).set_col(3, w);
    #endif
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat4<T>::Mat4(const Mat<3, 3, T> &rhs) {
        (*this)(0, 0) = rhs(0, 0);	(*this)(0, 1) = rhs(0, 1);	(*this)(0, 2) = rhs(0, 2);	(*this)(0, 3) = T(0);
        (*this)(1, 0) = rhs(1, 0);	(*this)(1, 1) = rhs(1, 1);	(*this)(1, 2) = rhs(1, 2);	(*this)(1, 3) = T(0);
        (*this)(2, 0) = rhs(2, 0);	(*this)(2, 1) = rhs(2, 1);	(*this)(2, 2) = rhs(2, 2);	(*this)(2, 3) = T(0);
        (*this)(3, 0) = T(0);		(*this)(3, 1) = T(0);		(*this)(3, 2) = T(0);		(*this)(3, 3) = T(1);
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat3<T> Mat4<T>::sub() const {
        Mat3<T> mat;
        for (size_t i = 0; i < 3; i++) {
            for (size_t j = 0; j < 3; j++) {
                mat(i, j) = (*this)(i, j);
            }
        }
        return mat;
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat4<T>::Mat4(const Vec<3, T> &s, const Quat<T> &rot, const Vec<3, T> &t) {
        assert(std::abs(rot.length() - 1) < epsilon<T>());

        //	get rotation matrix from quaternion:
        Mat3<T> r(rot);

        //	incorporate scale (cheaper than a matrix multiply):
        r(0, 0) = r(0, 0) * s[0]; r(0, 1) = r(0, 1) * s[1]; r(0, 2) = r(0, 2) * s[2];
        r(1, 0) = r(1, 0) * s[0]; r(1, 1) = r(1, 1) * s[1]; r(1, 2) = r(1, 2) * s[2];
        r(2, 0) = r(2, 0) * s[0]; r(2, 1) = r(2, 1) * s[1]; r(2, 2) = r(2, 2) * s[2];

        //	combine rotation matrix, scale and translation:
        (*this)(0, 0) = r(0, 0);	(*this)(0, 1) = r(0, 1);	(*this)(0, 2) = r(0, 2);	(*this)(0, 3) = t[0];
        (*this)(1, 0) = r(1, 0);	(*this)(1, 1) = r(1, 1);	(*this)(1, 2) = r(1, 2);	(*this)(1, 3) = t[1];
        (*this)(2, 0) = r(2, 0);	(*this)(2, 1) = r(2, 1);	(*this)(2, 2) = r(2, 2);	(*this)(2, 3) = t[2];
        (*this)(3, 0) = T(0); 		(*this)(3, 1) = T(0); 		(*this)(3, 2) = T(0); 		(*this)(3, 3) = T(1);
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat4<T> Mat4<T>::scale(T s) {
        return Mat4<T>(
            s, T(0), T(0), T(0),
            T(0), s, T(0), T(0),
            T(0), T(0), s, T(0),
            T(0), T(0), T(0), T(1)
            );
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat4<T> Mat4<T>::scale(T x, T y, T z, T w) {
        return Mat4<T>(
            x,    T(0), T(0), T(0),
            T(0), y,    T(0), T(0),
            T(0), T(0), z,    T(0),
            T(0), T(0), T(0), w
            );
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat4<T> Mat4<T>::scale(const Vec<4, T>& s) {
        return Mat4<T>(
            s.x,  T(0), T(0), T(0),
            T(0), s.y,  T(0), T(0),
            T(0), T(0), s.z,  T(0),
            T(0), T(0), T(0), s.w
            );
    }

    /*----------------------------------------------------------------------------*/
    
    template <typename T>
    inline Mat4<T> Mat4<T>::rotation(const Vec<3, T> &axis, T angle) {
        assert(std::abs(axis.length() - 1) < epsilon<T>());
        return Mat4<T>(Mat3<T>::rotation(axis, angle)); // gen 3x3 rotation matrix as arg to Mat4 constructor
    }
    
    /*----------------------------------------------------------------------------*/
    
    template <typename T>
    inline Mat4<T> Mat4<T>::rotation(const Vec<3, T> &axis_angle) {
        const T len = axis_angle.length();
        return Mat4<T>(Mat3<T>::rotation(axis_angle/len, len));
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat4<T> Mat4<T>::rotation(const Quat<T> &q) {
        // input must be unit quaternion
        assert(std::abs(q.length() - 1) < epsilon<T>());
        const T x = q.x;
        const T y = q.y;
        const T z = q.z;
        const T w = q.w;
        Mat4<T> m;
        m(0, 0) = 1.0 - 2.0 * (y*y + z*z);    m(0, 1) = 2.0 * (x*y - w*z);          m(0, 2) = 2.0 * (x*z + w*y);          m(0, 3) = 0.0;
        m(1, 0) = 2.0 * (x*y + w*z);          m(1, 1) = 1.0 - 2.0 * (x*x + z*z);    m(1, 2) = 2.0 * (y*z - w*x);          m(1, 3) = 0.0;
        m(2, 0) = 2.0 * (x*z - w*y);          m(2, 1) = 2.0 * (y*z + w*x);          m(2, 2) = 1.0 - 2.0 * (x*x + y*y);    m(2, 3) = 0.0;
        m(3, 0) = 0.0;                        m(3, 1) = 0.0;                        m(3, 2) = 0.0;                        m(3, 3) = 1.0;
        return m;
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat4<T> Mat4<T>::rotation(T x, T y, T z, int order) {
        // The code is not optimized. The final matrix can be directly derived.
        // http://www.songho.ca/opengl/gl_anglestoaxes.html
        // http://inside.mines.edu/fs_home/gmurray/ArbitraryAxisRotation/
        Mat4<T> rx; // Rotation about X-axis (Pitch)
        rx(0, 0) = T(1);         rx(0, 1) = T(0);           rx(0, 2) = T(0);            rx(0, 3) = T(0);
        rx(1, 0) = T(0);         rx(1, 1) = std::cos(x);    rx(1, 2) = -std::sin(x);    rx(1, 3) = T(0);
        rx(2, 0) = T(0);         rx(2, 1) = std::sin(x);    rx(2, 2) = std::cos(x);     rx(2, 3) = T(0);
        rx(3, 0) = T(0);         rx(3, 1) = T(0);           rx(3, 2) = T(0);            rx(3, 3) = T(1);

        Mat4<T> ry; // Rotation about Y-axis (Yaw, Heading)
        ry(0, 0) = std::cos(y);  ry(0, 1) = T(0);           ry(0, 2) = std::sin(y);     ry(0, 3) = T(0);
        ry(1, 0) = T(0);         ry(1, 1) = T(1);           ry(1, 2) = T(0);            ry(1, 3) = T(0);
        ry(2, 0) = -std::sin(y); ry(2, 1) = T(0);           ry(2, 2) = std::cos(y);     ry(2, 3) = T(0);
        ry(3, 0) = T(0);         ry(3, 1) = T(0);           ry(3, 2) = T(0);            ry(3, 3) = T(1);

        Mat4<T> rz; // Rotation about Z-axis (Roll)
        rz(0, 0) = std::cos(z);  rz(0, 1) = -std::sin(z);   rz(0, 2) = T(0);            rz(0, 3) = T(0);
        rz(1, 0) = std::sin(z);  rz(1, 1) = std::cos(z);    rz(1, 2) = T(0);            rz(1, 3) = T(0);
        rz(2, 0) = T(0);         rz(2, 1) = T(0);           rz(2, 2) = T(1);            rz(2, 3) = T(0);
        rz(3, 0) = T(0);         rz(3, 1) = T(0);           rz(3, 2) = T(0);            rz(3, 3) = T(1);

        switch (order) {
            case 123:   return rz * ry * rx;
            case 132:   return ry * rz * rx;
            case 213:   return rz * rx * ry;
            case 231:   return rx * rz * ry;
            case 312:   return ry * rx * rz;
            case 321:   return rx * ry * rz;
            default:
                LOG(ERROR) << "invalid rotation order";
                return rx * rz * ry;
        }
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat4<T> Mat4<T>::translation(const Vec<3, T> &t) {
        return Mat4<T>(
            T(1), T(0), T(0), t[0],
            T(0), T(1), T(0), t[1],
            T(0), T(0), T(1), t[2],
            T(0), T(0), T(0), T(1)
            );
    }

    /*----------------------------------------------------------------------------*/
    template <typename T>
    inline Mat4<T> Mat4<T>::translation(T x, T y, T z) {
        return Mat4<T>(
            T(1), T(0), T(0), x,
            T(0), T(1), T(0), y,
            T(0), T(0), T(1), z,
            T(0), T(0), T(0), T(1)
            );
    }

}

#endif // EASY3D_CORE_MAT_H
