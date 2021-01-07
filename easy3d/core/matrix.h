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

#ifndef EASY3D_CORE_MATRIX_H
#define EASY3D_CORE_MATRIX_H

#include <vector>
#include <algorithm> // for std::min, std::max
#include <cmath>     // for std::sqrt
#include <cassert>


namespace easy3d {

    /**
     * \brief A matrix representation.
     * \details The matrix classes in #include <easy3d/core/mat.h> has their dimensions templated. So their dimensions
     *      must be constant and known at the compile time. In contrast, this matrix can have dynamic dimensions.
     * \attention Matrix has a row-major storage.
     * \class Matrix easy3d/core/matrix.h
     */
    template<typename FT>
    class Matrix {
    public:
        // constructors and destructor
        Matrix();
        Matrix(const Matrix<FT> &A);
        Matrix(int rows, int cols, const FT &x = FT(0));
        Matrix(int rows, int cols, const FT *v);
        ~Matrix();

        // assignments
        Matrix<FT> &operator=(const Matrix<FT> &A);    // overload evaluate operator = from matrix to matrix
        Matrix<FT> &operator=(const FT &x);            // overload evaluate operator = from scalar to matrix

        FT *operator[](int row);
        const FT *operator[](int row) const;

        FT &operator()(int row, int col);
        const FT &operator()(int row, int col) const;

        // type conversion functions
        operator FT *();
        operator const FT *() const;

        // others
        int rows() const; // the number of rows
        int cols() const; // the number of cols

        Matrix<FT> &resize(int rows, int cols);    // reallocate matrix's size

        /// get the matrix's row vector
        std::vector<FT> get_row(int row) const;
        /// get the matrix's column vector
        std::vector<FT> get_column(int col) const;
        /// set the matrix's row vector
        void set_row(const std::vector<FT> &v, int row);
        /// set the matrix's column vector
        void set_column(const std::vector<FT> &v, int col);

        /// Sets all elements to 0.
        void load_zero();
        /// Sets the matrix to identity.
        void load_identity(FT v = FT(1));

        /// Returns the transpose of the matrix.
        Matrix<FT> &transpose();

        /// Returns the trace of this matrix, i.e. the sum of the coefficients on the main diagonal.
        /// \note The matrix can be of any dimensions, not necessarily square.
        FT trace() const;

        /// Addition
        Matrix<FT> &operator+=(const Matrix<FT> &);
        /// Subtraction
        Matrix<FT> &operator-=(const Matrix<FT> &);

        /// Matrix-scalar product
        template<typename FT2>
        Matrix<FT> &operator*=(const FT2 &);
        /// Matrix-scalar division
        template<typename FT2>
        Matrix<FT> &operator/=(const FT2 &);

        /// Matrix-matrix product
        Matrix<FT> &operator*=(const Matrix<FT> &);
        /// Matrix-vector product
        std::vector<FT> operator*=(const std::vector<FT> &v) const;

    private:

        // 0-based data pointer
        FT *data_;

        // 0-based row pointer's pointer
        FT **prow_;

        // row number, column number and total number
        int nRow;
        int nColumn;
        long nTotal;

        void init(int rows, int cols);

        void copy_from_array(const FT *v); // copy matrix from normal array
        void set_by_scalar(const FT &x); // set matrix by a scalar
        void destroy();    // destroy the matrix

    }; // class Matrix


    //----------------------------- Utilities for Matrix ------------------------------//

    // input and output
    template<typename FT>
    std::ostream &operator<<(std::ostream &, const Matrix<FT> &);    // Overload of the output stream.
    template<typename FT>
    std::istream &operator>>(std::istream &, Matrix<FT> &);        // Overload of the input stream.

    // arithmetic operators
    template<typename FT>
    Matrix<FT> operator-(const Matrix<FT> &);  // get negative matrix
    template<typename FT>
    Matrix<FT> operator+(const Matrix<FT> &, const FT &); // matrix-scalar addition
    template<typename FT>
    Matrix<FT> operator+(const FT &, const Matrix<FT> &);    // scalar-matrix addition
    template<typename FT>
    Matrix<FT> operator+(const Matrix<FT> &, const Matrix<FT> &); // matrix-matrix addition
    template<typename FT>
    Matrix<FT> operator-(const Matrix<FT> &, const FT &); // matrix-scalar subtraction
    template<typename FT>
    Matrix<FT> operator-(const FT &, const Matrix<FT> &);    // scalar-matrix subtraction
    template<typename FT>
    Matrix<FT> operator-(const Matrix<FT> &, const Matrix<FT> &); // matrix-matrix subtraction
    template<typename FT>
    Matrix<FT> operator*(const Matrix<FT> &, const Matrix<FT> &);        // matrix-matrix multiplication
    template<typename FT>
    std::vector<FT> operator*(const Matrix<FT> &, const std::vector<FT> &);    // matrix-vector multiplication
    template<typename FT1, typename FT2>
    Matrix<FT1> operator*(const Matrix<FT1> &, const FT2 &);    // matrix-scalar multiplication
    template<typename FT1, typename FT2>
    Matrix<FT1> operator*(const FT2 &, const Matrix<FT1> &);    // scalar-matrix multiplication
    template<typename FT1, typename FT2>
    Matrix<FT1> operator/(const Matrix<FT1> &, const FT2 &);    // matrix-scalar division
    template<typename FT1, typename FT2>
    Matrix<FT1> operator/(const FT2 &, const Matrix<FT1> &);    // scalar-matrix division

    /// \name Multiplication (matrix-matrix, matrix-vector)
    // \{
    // matrix-matrix multiplication (result has already been allocated)
    template<typename FT>
    void mult(const Matrix<FT> &, const Matrix<FT> &, Matrix<FT> &);

    // matrix-vector multiplication (result has already been allocated)
    template<typename FT>
    void mult(const Matrix<FT> &, const std::vector<FT> &, std::vector<FT> &);

    // matrix-matrix multiplication
    template<typename FT>
    Matrix<FT> mult(const Matrix<FT> &, const Matrix<FT> &);

    // matrix-vector multiplication
    template<typename FT>
    std::vector<FT> mult(const Matrix<FT> &, const std::vector<FT> &);
    //\}

    /// \name Transpose and conjugate transpose
    // \{
    template<typename FT>
    Matrix<FT> transpose(const Matrix<FT> &);            // matrix transpose
    template<typename FT>
    Matrix<FT> conjugate_transpose(const Matrix<FT> &);    // matrix conjugate transpose
    //\}

    /// \name Transpose multiplication
    // \{
    template<typename FT>
    Matrix<FT> transpose_mult(const Matrix<FT> &, const Matrix<FT> &);        // A^T * B
    template<typename FT>
    std::vector<FT> transpose_mult(const Matrix<FT> &, const std::vector<FT> &);    // A^T * b
    template<typename FT>
    Matrix<FT> mult_transpose(const Matrix<FT> &, const Matrix<FT> &);        // A * B^T
    template<typename FT>
    Matrix<FT> mult_transpose(const std::vector<FT> &, const std::vector<FT> &);    // a * b^T
    //\}

    /// \name Unit and diagonal matrix
    // \{
    template<typename FT>
    Matrix<FT> identity(int, const FT &);            // Generate an identity matrix.
    template<typename FT>
    Matrix<FT> diagonal(const std::vector<FT> &);    // Generate a diagonal matrix by given its diagonal elements.
    template<typename FT>
    std::vector<FT> diagonal(const Matrix<FT> &);    // Get the diagonal entries of matrix.
    //\}


    // utilities
    template<typename FT>
    FT norm(const Matrix<FT> &);               // Compute Frobenius norm of matrix.
    template<typename FT>
    void swap(Matrix<FT> &, Matrix<FT> &);     // Swap two matrices.
    template<typename FT>
    std::vector<FT> sum(const Matrix<FT> &);   // Matrix's column vectors sum.
    template<typename FT>
    std::vector<FT> min(const Matrix<FT> &);   // Minimum of matrix's column vectors.
    template<typename FT>
    std::vector<FT> max(const Matrix<FT> &);   // Maximum of matrix's column vectors.
    template<typename FT>
    std::vector<FT> mean(const Matrix<FT> &);   // Matrix's column vectors mean.


    //-----------------------------  Matrix  Implementation ------------------------------------//


    // initialize
    template<typename FT>
    void Matrix<FT>::init(int rows, int cols) {
        nRow = rows;
        nColumn = cols;
        nTotal = nRow * nColumn;

        data_ = new FT[nTotal];
        prow_ = new FT *[nRow];

        assert(data_ != NULL);
        assert(prow_ != NULL);

        FT *p = data_;
        for (int i = 0; i < nRow; ++i) {
            prow_[i] = p;
            p += nColumn;
        }
    }

    // copy matrix from normal array
    template<typename FT>
    inline void Matrix<FT>::copy_from_array(const FT *v) {
        for (long i = 0; i < nTotal; ++i)
            data_[i] = v[i];
    }

    // set matrix by a scalar
    template<typename FT>
    inline void Matrix<FT>::set_by_scalar(const FT &x) {
        for (long i = 0; i < nTotal; ++i)
            data_[i] = x;
    }

    // destroy the matrix
    template<typename FT>
    void Matrix<FT>::destroy() {
        if (data_ == NULL)
            return;
        else
            delete[] data_;

        if (prow_ != NULL)
            delete[] prow_;
    }

    // constructors and destructor
    template<typename FT>
    Matrix<FT>::Matrix()
            : data_(0), prow_(0), nRow(0), nColumn(0), nTotal(0) {
    }

    template<typename FT>
    Matrix<FT>::Matrix(const Matrix<FT> &A) {
        init(A.nRow, A.nColumn);
        copy_from_array(A.data_);
    }

    template<typename FT>
    Matrix<FT>::Matrix(int rows, int cols, const FT &x) {
        init(rows, cols);
        set_by_scalar(x);
    }

    template<typename FT>
    Matrix<FT>::Matrix(int rows, int cols, const FT *arrays) {
        init(rows, cols);
        copy_from_array(arrays);
    }

    template<typename FT>
    Matrix<FT>::~Matrix() {
        destroy();
    }

    // overload evaluate operator = from matrix to matrix
    template<typename FT>
    Matrix<FT> &Matrix<FT>::operator=(const Matrix<FT> &A) {
        if (data_ == A.data_)
            return *this;

        if (nRow == A.nRow && nColumn == A.nColumn)
            copy_from_array(A.data_);
        else {
            destroy();
            init(A.nRow, A.nColumn);
            copy_from_array(A.data_);
        }

        return *this;
    }

    // overload evaluate operator = from scalar to matrix
    template<typename FT>
    inline Matrix<FT> &Matrix<FT>::operator=(const FT &x) {
        set_by_scalar(x);
        return *this;
    }

    // overload operator [] for 0-offset access
    template<typename FT>
    inline FT *Matrix<FT>::operator[](int row) {
        assert(0 <= row);
        assert(row < nRow);
        return prow_[row];
    }

    template<typename FT>
    inline const FT *Matrix<FT>::operator[](int row) const {
        assert(0 <= row);
        assert(row < nRow);
        return prow_[row];
    }

    template<typename FT>
    inline FT &Matrix<FT>::operator()(int row, int col) {
        assert(0 <= row);
        assert(row < nRow);
        assert(0 <= col);
        assert(col < nColumn);
        return prow_[row][col];
    }

    template<typename FT>
    inline const FT &Matrix<FT>::operator()(int row, int col) const {
        assert(0 <= row);
        assert(row < nRow);
        assert(0 <= col);
        assert(col < nColumn);
        return prow_[row][col];
    }

    // Clears the matrix.
    // This resets all values to 0 (zero)
    template<typename FT>
    inline
    void Matrix<FT>::load_zero() {
        for (int i = 0; i < nRow; i++) {
            for (int j = 0; j < nColumn; j++) {
                prow_[i][j] = FT(0);
            }
        }
    }

    // Sets the matrix to identity.
    // This sets all coefficients of this matrix to be equal to nROW x nCOL identity matrix.
    template<typename FT> inline
    void Matrix<FT>::load_identity(FT v /* = FT(1.0)*/) {
        for (int i = 0; i < nRow; i++) {
            for (int j = 0; j < nColumn; j++) {
                prow_[i][j] = (i == j) ? v : FT(0);
            }
        }
    }

    // type conversion functions
    template<typename FT>
    inline Matrix<FT>::operator FT *() {
        return data_;
    }

    template<typename FT>
    inline Matrix<FT>::operator const FT *() const {
        return data_;
    }


    template<typename FT>
    inline int Matrix<FT>::rows() const {
        return nRow;
    }

    template<typename FT>
    inline int Matrix<FT>::cols() const {
        return nColumn;
    }

    // reallocate matrix's size
    template<typename FT>
    Matrix<FT> &Matrix<FT>::resize(int rows, int cols) {
        if (rows == nRow && cols == nColumn)
            return *this;

        destroy();
        init(rows, cols);

        return *this;
    }

    // get the matrix's row vector
    template<typename FT>
    std::vector<FT> Matrix<FT>::get_row(int row) const {
        assert(0 <= row);
        assert(row < nRow);
        std::vector<FT> tmp(nColumn);
        for (int j = 0; j < nColumn; ++j)
            tmp[j] = prow_[row][j];

        return tmp;
    }

    // get the matrix's column vector
    template<typename FT>
    std::vector<FT> Matrix<FT>::get_column(int col) const {
        assert(0 <= col);
        assert(col < nColumn);
        std::vector<FT> tmp(nRow);
        for (int i = 0; i < nRow; ++i)
            tmp[i] = prow_[i][col];

        return tmp;
    }

    // set the matrix's row vector
    template<typename FT>
    void Matrix<FT>::set_row(const std::vector<FT> &v, int row) {
        assert(0 <= row);
        assert(row < nRow);
        assert(v.size() == nColumn);
        for (int j = 0; j < nColumn; ++j)
            prow_[row][j] = v[j];
    }

    // set the matrix's column vector
    template<typename FT>
    void Matrix<FT>::set_column(const std::vector<FT> &v, int col) {
        assert(0 <= col);
        assert(col < nColumn);
        assert(v.size() == nRow);
        for (int i = 0; i < nRow; ++i)
            prow_[i][col] = v[i];
    }

    template<typename FT>
    Matrix<FT> &Matrix<FT>::transpose() {
        for (int i = 0; i < nColumn; i++) {
            for (int j = i + 1; j < nRow; j++) {
                std::swap((*this)[i][j], (*this)[j][i]);
            }
        }
        return *this;
    }

    template<typename FT>
    FT Matrix<FT>::trace() const {
        int range = std::min(nRow, nColumn);
        FT trac = 0.0;
        for (int i = 0; i < range; i++) {
            trac += (*this)[i][i];
        }
        return trac;
    }

    template<typename FT>
    Matrix<FT> &Matrix<FT>::operator+=(const Matrix<FT> &rhs) {
        assert(nRow == rhs.rows());
        assert(nColumn == rhs.cols());

        FT **rowPtrL = prow_;
        FT *colPtrL = 0;
        FT **rowPtrR = rhs.prow_;
        const FT *colPtrR = 0;

        for (int i = 0; i < nRow; ++i) {
            colPtrL = *rowPtrL++;
            colPtrR = *rowPtrR++;
            for (int j = 0; j < nColumn; ++j)
                *colPtrL++ += *colPtrR++;
        }

        return *this;
    }

    template<typename FT>
    Matrix<FT> &Matrix<FT>::operator-=(const Matrix<FT> &rhs) {
        assert(nRow == rhs.rows());
        assert(nColumn == rhs.cols());

        FT **rowPtrL = prow_;
        FT *colPtrL = 0;
        FT **rowPtrR = rhs.prow_;
        const FT *colPtrR = 0;

        for (int i = 0; i < nRow; ++i) {
            colPtrL = *rowPtrL++;
            colPtrR = *rowPtrR++;
            for (int j = 0; j < nColumn; ++j)
                *colPtrL++ -= *colPtrR++;
        }

        return *this;
    }

    // Matrix-scalar product
    // compound assignment operators *=
    template<typename FT>
    template<typename FT2>
    Matrix<FT> &Matrix<FT>::operator*=(const FT2 &x) {
        FT **rowPtr = prow_;
        FT *colPtr = 0;

        for (int i = 0; i < nRow; ++i) {
            colPtr = *rowPtr++;
            for (int j = 0; j < nColumn; ++j)
                *colPtr++ *= x;
        }

        return *this;
    }


    template<typename FT>
    Matrix<FT> &Matrix<FT>::operator*=(const Matrix<FT> &B) {
        Matrix<FT> A = *this;
        const int M = A.rows();
        const int N = B.cols();
        const int K = A.cols();
        assert(B.rows() == K);

        resize(M, N);
        FT sum = 0;
        const FT *pRow, *pCol;

        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; ++j) {
                pRow = &A[i][0];
                pCol = &B[0][j];
                sum = 0;

                for (int k = 0; k < K; ++k) {
                    sum += (*pRow) * (*pCol);
                    pRow++;
                    pCol += N;
                }
                prow_[i][j] = sum;
            }
        }
        return *this;
    }

    // Matrix-scalar division
    // compound assignment operators *=
    template<typename FT>
    template<typename FT2>
    Matrix<FT> &Matrix<FT>::operator/=(const FT2 &x) {
        FT **rowPtr = prow_;
        FT *colPtr = 0;

        for (int i = 0; i < nRow; ++i) {
            colPtr = *rowPtr++;
            for (int j = 0; j < nColumn; ++j)
                *colPtr++ /= x;
        }

        return *this;
    }


    // Overload the output stream function.
    template<typename FT>
    std::ostream &operator<<(std::ostream &out, const Matrix<FT> &A) {
        const int rows = A.rows();
        const int cols = A.cols();
        out << rows << " " << cols << std::endl;

        for (int i = 0; i < rows; ++i) {
            for (int j = 0; j < cols; ++j) {
                if (std::abs(A[i][j]) < 1e-6)
                    out << 0 << "\t";
                else
                    out << A[i][j] << "\t";
            }
            out << "\n";
        }

        return out;
    }

    // Overload the input stream function.
    template<typename FT>
    std::istream &operator>>(std::istream &in, Matrix<FT> &A) {
        int rows, cols;
        in >> rows >> cols;

        if (!(rows == A.rows() && cols == A.cols()))
            A.resize(rows, cols);

        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < cols; ++j)
                in >> A[i][j];

        return in;
    }

    // get negative matrix
    template<typename FT>
    Matrix<FT> operator-(const Matrix<FT> &A) {
        int rows = A.rows();
        int cols = A.cols();

        Matrix<FT> tmp(rows, cols);
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < cols; ++j)
                tmp[i][j] = -A[i][j];

        return tmp;
    }

    // matrix-scalar addition
    template<typename FT>
    inline Matrix<FT> operator+(const Matrix<FT> &A, const FT &x) {
        Matrix<FT> tmp(A);
        return tmp += x;
    }

    template<typename FT>
    inline Matrix<FT> operator+(const FT &x, const Matrix<FT> &A) {
        return A + x;
    }

    // matrix-matrix addition
    template<typename FT>
    inline Matrix<FT> operator+(const Matrix<FT> &A1, const Matrix<FT> &A2) {
        Matrix<FT> tmp(A1);
        return tmp += A2;
    }

    // matrix-scalar subtraction
    template<typename FT>
    inline Matrix<FT> operator-(const Matrix<FT> &A, const FT &x) {
        Matrix<FT> tmp(A);
        return tmp -= x;
    }

    template<typename FT>
    inline Matrix<FT> operator-(const FT &x, const Matrix<FT> &A) {
        Matrix<FT> tmp(A);
        return -tmp += x;
    }

    // matrix-matrix subtraction
    template<typename FT>
    inline Matrix<FT> operator-(const Matrix<FT> &A1, const Matrix<FT> &A2) {
        Matrix<FT> tmp(A1);
        return tmp -= A2;
    }

    // matrix-matrix multiplication
    template<typename FT>
    Matrix<FT> operator*(const Matrix<FT> &A1, const Matrix<FT> &A2) {
        assert(A1.cols() == A2.rows());

        int rows = A1.rows();
        int cols = A2.cols();
        //	int K = A1.cols();

        Matrix<FT> tmp(rows, cols);
        //	for( int i=0; i<rows; ++i )
        //		for( int j=0; j<cols; ++j )
        //		{
        //            tmp[i][j] = 0;
        //			for( int k=0; k<K; ++k )
        //			    tmp[i][j] += A1[i][k] * A2[k][j];
        //		}

        mult(A1, A2, tmp);

        return tmp;
    }

    // matrix-vector multiplication
    template<typename FT>
    std::vector<FT> operator*(const Matrix<FT> &A, const std::vector<FT> &b) {
        assert(A.cols() == b.size());

        int rows = A.rows();
        //	int cols = A.cols();

        std::vector<FT> tmp(rows);
        mult(A, b, tmp);

        return tmp;
    }

    // matrix-scalar multiplication
    template<typename FT1, typename FT2>
    inline
    Matrix<FT1> operator*(const Matrix<FT1> &A, const FT2 &s) {
        Matrix<FT1> tmp(A);
        return tmp *= s;
    }

    // scalar-matrix multiplication
    template<typename FT1, typename FT2>
    inline
    Matrix<FT1> operator*(const FT2 &s, const Matrix<FT1> &A) {
        return A * s;
    }

    // matrix-scalar division
    template<typename FT1, typename FT2>
    inline
    Matrix<FT1> operator/(const Matrix<FT1> &A, const FT2 &s) {
        Matrix<FT1> tmp(A);
        return tmp /= s;
    }

    // scalar-matrix division
    template<typename FT1, typename FT2>
    inline
    Matrix<FT1> operator/(const FT2 &s, const Matrix<FT1> &A) {
        int rows = A.rows();
        int clumns = A.cols();

        Matrix<FT1> tmp(rows, clumns);
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < clumns; ++j)
                tmp[i][j] = s / A[i][j];

        return tmp;
    }

    // This is an optimized version of matrix multiplication, where the destination matrix has already been allocated.
    template<typename FT>
    void mult(const Matrix<FT> &A, const Matrix<FT> &B, Matrix<FT> &C) {
        int M = A.rows();
        int N = B.cols();
        int K = A.cols();

        assert(B.rows() == K);

        C.resize(M, N);
        FT sum = 0;
        const FT *pRow, *pCol;

        for (int i = 0; i < M; i++) {
            for (int j = 0; j < N; ++j) {
                pRow = &A[i][0];
                pCol = &B[0][j];
                sum = 0;

                for (int k = 0; k < K; ++k) {
                    sum += (*pRow) * (*pCol);
                    pRow++;
                    pCol += N;
                }
                C[i][j] = sum;
            }
        }
    }

    // This is an optimized version of matrix and vector multiplication, where the destination vector has already been allocated.
    template<typename FT>
    void mult(const Matrix<FT> &A, const std::vector<FT> &b, std::vector<FT> &c) {
        int M = A.rows();
        int N = A.cols();

        assert(b.size() == N);

        c.resize(M);
        FT sum = 0;
        const FT *pRow, *pCol;

        for (int i = 0; i < M; i++) {
            pRow = &A[i][0];
            pCol = &b[0];
            sum = 0;

            for (int j = 0; j < N; ++j) {
                sum += (*pRow) * (*pCol);
                pRow++;
                pCol++;
            }
            c[i] = sum;
        }
    }

    // This is an optimized version of matrix multiplication, where the destination matrix has already been allocated.
    template<typename FT>
    Matrix<FT> mult(const Matrix<FT> &A, const Matrix<FT> &B) {
        int M = A.rows();
        int N = B.cols();
        int K = A.cols();

        assert(B.rows() == K);

        Matrix<FT> C(M, N);
        FT sum = 0;
        const FT *pRow, *pCol;

        for (int i = 0; i < M; i++)
            for (int j = 0; j < N; ++j) {
                pRow = &A[i][0];
                pCol = &B[0][j];
                sum = 0;

                for (int k = 0; k < K; ++k) {
                    sum += (*pRow) * (*pCol);
                    pRow++;
                    pCol += N;
                }
                C[i][j] = sum;
            }
        return C;
    }

    // This is an optimized version of matrix and vector multiplication, where the destination vector has already
    // been allocated.
    template<typename FT>
    std::vector<FT> mult(const Matrix<FT> &A, const std::vector<FT> &b) {
        int M = A.rows();
        int N = A.cols();

        assert(b.size() == N);

        std::vector<FT> c(M);
        FT sum = 0;
        const FT *pRow, *pCol;

        for (int i = 0; i < M; i++) {
            pRow = &A[i][0];
            pCol = &b[0];
            sum = 0;

            for (int j = 0; j < N; ++j) {
                sum += (*pRow) * (*pCol);
                pRow++;
                pCol++;
            }
            c[i] = sum;
        }
        return c;
    }


    // matrix transpose
    template<typename FT>
    Matrix<FT> transpose(const Matrix<FT> &A) {
        int rows = A.cols();
        int clumns = A.rows();

        Matrix<FT> tmp(rows, clumns);
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < clumns; ++j)
                tmp[i][j] = A[j][i];

        return tmp;
    }

    // matrix conjugate transpose
    template<typename FT>
    Matrix<FT> conjugate_transpose(const Matrix<FT> &A) {
        int rows = A.cols();
        int clumns = A.rows();

        Matrix<FT> tmp(rows, clumns);
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < clumns; ++j)
                tmp[i][j] = conj(A[j][i]);

        return tmp;
    }

    // matrix-matrix tranpose multiplication: A^T * B.
    template<typename FT>
    Matrix<FT> transpose_mult(const Matrix<FT> &A1, const Matrix<FT> &A2) {
        assert(A1.rows() == A2.rows());

        int rows = A1.cols();
        int cols = A2.cols();
        int K = A1.rows();

        Matrix<FT> tmp(rows, cols);
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < cols; ++j)
                for (int k = 0; k < K; ++k)
                    tmp[i][j] += A1[k][i] * A2[k][j];

        return tmp;
    }

    // matrix-vector tranpose multiplication: A^T * b.
    template<typename FT>
    std::vector<FT> transpose_mult(const Matrix<FT> &A, const std::vector<FT> &v) {
        assert(A.rows() == v.size());

        int rows = A.rows();
        int cols = A.cols();

        std::vector<FT> tmp(cols);
        for (int i = 0; i < cols; ++i)
            for (int j = 0; j < rows; ++j)
                tmp[i] += A[j][i] * v[j];

        return tmp;
    }

    // matrix-matrix tranpose multiplication: A * B^T.
    template<typename FT>
    Matrix<FT> mult_transpose(const Matrix<FT> &A1, const Matrix<FT> &A2) {
        assert(A1.cols() == A2.cols());

        int rows = A1.rows();
        int cols = A2.rows();
        int K = A1.cols();

        Matrix<FT> tmp(rows, cols);
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < cols; ++j)
                for (int k = 0; k < K; ++k)
                    tmp[i][j] += A1[i][k] * A2[j][k];

        return tmp;
    }

    // vector-vector tranpose multiplication: a * b^T.
    template<typename FT>
    Matrix<FT> mult_transpose(const std::vector<FT> &a, const std::vector<FT> &b) {
        int rows = a.size();
        int cols = b.size();

        Matrix<FT> tmp(rows, cols);
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < cols; ++j)
                tmp[i][j] = a[i] * b[j];

        return tmp;
    }

    // Generate the identity matrix.
    template<typename FT>
    Matrix<FT> identity(int N, const FT &x) {
        Matrix<FT> tmp(N, N);
        for (int i = 0; i < N; ++i)
            tmp[i][i] = x;

        return tmp;
    }

    // Get the diagonal entries of matrix.
    template<typename FT>
    std::vector<FT> diagonal(const Matrix<FT> &A) {
        int nColumn = A.rows();
        if (nColumn > A.cols())
            nColumn = A.cols();

        std::vector<FT> tmp(nColumn);
        for (int i = 0; i < nColumn; ++i)
            tmp[i] = A[i][i];

        return tmp;
    }

    // Generate the diagonal of matrix by given its diagonal elements.
    template<typename FT>
    Matrix<FT> diagonal(const std::vector<FT> &d) {
        int N = static_cast<int>(d.size());

        Matrix<FT> tmp(N, N);
        for (int i = 0; i < N; ++i)
            tmp[i][i] = d[i];

        return tmp;
    }


    // Compute Frobenius norm of matrix.
    template<typename FT>
    FT norm(const Matrix<FT> &A) {
        int m = A.rows();
        int n = A.cols();

        FT sum = 0;
        for (int i = 0; i < m; ++i)
            for (int j = 0; j < n; ++j)
                sum += A[i][j] * A[i][j];

        return std::sqrt(sum);
    }

    // Swap two matrices.
    template<typename FT>
    void swap(Matrix<FT> &lhs, Matrix<FT> &rhs) {
        int m = lhs.rows();
        int n = lhs.cols();

        assert(m == rhs.rows());
        assert(n == rhs.cols());

        for (int i = 0; i < m; ++i)
            for (int j = 0; j < n; ++j)
                std::swap(lhs(i, j), rhs(i, j));
    }

    // Matrix's column vectors sum.
    template<typename FT>
    std::vector<FT> sum(const Matrix<FT> &A) {
        int m = A.rows();
        int n = A.cols();
        std::vector<FT> s(n);

        for (int j = 0; j < n; ++j)
            for (int i = 0; i < m; ++i)
                s[j] += A[i][j];
        return s;
    }

    // Minimum of matrix's column vectors.
    template<typename FT>
    std::vector<FT> min(const Matrix<FT> &A) {
        int m = A.rows();
        int n = A.cols();
        std::vector<FT> sum(n);

        for (int j = 0; j < n; ++j) {
            FT tmp = A[0][j];
            for (int i = 1; i < m; ++i)
                if (tmp > A[i][j])
                    tmp = A[i][j];
            sum[j] = tmp;
        }

        return sum;
    }

    //  Maximum of matrix's column vectors.
    template<typename FT>
    std::vector<FT> max(const Matrix<FT> &A) {
        int m = A.rows();
        int n = A.cols();
        std::vector<FT> sum(n);

        for (int j = 0; j < n; ++j) {
            FT tmp = A[0][j];
            for (int i = 1; i < m; ++i)
                if (tmp < A[i][j])
                    tmp = A[i][j];
            sum[j] = tmp;
        }

        return sum;
    }

    // Matrix's column vectors mean.
    template<typename FT>
    inline std::vector<FT> mean(const Matrix<FT> &A) {
        return sum(A) / FT(A.rows());
    }

}

#endif
