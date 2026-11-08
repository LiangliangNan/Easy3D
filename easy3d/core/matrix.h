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

#ifndef EASY3D_CORE_MATRIX_H
#define EASY3D_CORE_MATRIX_H

#include <vector>
#include <complex>
#include <algorithm> // for std::min, std::max
#include <cmath>     // for std::sqrt

#include <easy3d/core/random.h>


namespace easy3d {

    /**
     * \brief A matrix representation, which supports dynamic sizes.
     * \attention Matrix has a row-major storage.
     * \note The matrix classes in easy3d/core/mat.h has their dimensions templated. So their dimensions
     *      must be constant and known at the compile time. In contrast, this matrix can have dynamic dimensions.
     * \class Matrix easy3d/core/matrix.h
     */
    template<typename FT>
    class Matrix {
    public:
        /**
         * Default constructor (that constructs an empty matrix).
         */
        Matrix();

        /**
         * Copy constructor (i.e., construct a matrix from another matrix). The new matrix will be the same as the input
         * matrix.
         * @param A Input matrix.
         */
        Matrix(const Matrix<FT> &A);

        /**
         * Construct a matrix by specifying its dimension (i.e., number of rows and number of columns) and initialize
         * all entries to zero.
         * @param rows Number of rows
         * @param cols Number of columns.
         * @param x Default value of each element of the matrix.
         */
        Matrix(int rows, int cols, const FT &x = FT(0));

        /**
         * Construct a matrix by specifying its dimension (i.e., number of rows and number of columns) and initialize
         * the matrix from an array. The first row of the matrix will be initialized by the first cols elements of
         * the array. Then the second row will be initialized by the next cols elements of the array, and so on.
         * @param rows Number of rows
         * @param cols Number of columns.
         * @param array An array containing at least (rows * cols) elements.
         * @attention: Matrices are stored internally as row-major.
         */
        Matrix(int rows, int cols, const std::vector<FT> &array);

        /**
         * This is an override of the above constructor.
         * Construct a matrix by specifying its dimension (i.e., number of rows and number of columns) and initialize
         * the matrix from an array. The first row of the matrix will be initialized by the first cols elements of
         * the array. Then the second row will be initialized by the next cols elements of the array, and so on.
         * @param rows Number of rows
         * @param cols Number of columns.
         * @param array The pointer to the first element of an array used to initialize the matrix. It must have at
         *      least (rows * cols) elements.
         * @attention: Matrices are stored internally as row-major.
         */
        Matrix(int rows, int cols, const FT *array);

        ~Matrix();

        /// Assign another matrix to this matrix.
        /// \param A Matrix to copy.
        /// \return Reference to this matrix.
        Matrix<FT> &operator=(const Matrix<FT> &A);    // overload evaluate operator = from matrix to matrix
        /// Assign a scalar to every entry.
        /// \param x Value assigned to all entries.
        /// \return Reference to this matrix.
        Matrix<FT> &operator=(const FT &x);            // overload evaluate operator = from scalar to matrix

        /// Return a pointer to a row.
        /// \param row Zero-based row index.
        /// \return Pointer to the first element in the row.
        FT *operator[](int row);

        /// Return a const pointer to a row.
        /// \param row Zero-based row index.
        /// \return Pointer to the first element in the row.
        const FT *operator[](int row) const;

        /// Access a matrix element.
        /// \param row Zero-based row index.
        /// \param col Zero-based column index.
        /// \return Reference to the selected element.
        FT &operator()(int row, int col);

        /// Access a matrix element read-only.
        /// \param row Zero-based row index.
        /// \param col Zero-based column index.
        /// \return Const reference to the selected element.
        const FT &operator()(int row, int col) const;

        /// Set a matrix element.
        /// \param row Zero-based row index.
        /// \param col Zero-based column index.
        /// \param v New value.
        void set(int row, int col, FT v);

        /// Read a matrix element.
        /// \param row Zero-based row index.
        /// \param col Zero-based column index.
        /// \return Const reference to the selected element.
        const FT &get(int row, int col) const;

        /**
         * Return the data of the matrix as a 1D array.
         * Attention: Matrices are stored internally as row-major (i.e., the rows are concatenated in the returned array).
         * \return Pointer to the first row-major element.
         */
        FT *data() { return data_; }

        /**
         * Return the data of the matrix as a 1D array (const version).
         * Attention: Matrices are stored internally as row-major (i.e., the rows are concatenated in the returned array).
         * \return Const pointer to the first row-major element.
         */
        const FT *data() const { return data_; }

        /// Return the number of rows.
        /// \return Row count.
        int rows() const;

        /// Return the number of columns.
        /// \return Column count.
        int cols() const;

        /// Change the dimensions of the matrix.
        /// \param rows New row count.
        /// \param cols New column count.
        /// \return Reference to this matrix.
        Matrix<FT> &resize(int rows, int cols);

        /// Copy a row into a vector.
        /// \param row Zero-based row index.
        /// \return Values in the selected row.
        std::vector<FT> get_row(int row) const;

        /// Copy a column into a vector.
        /// \param col Zero-based column index.
        /// \return Values in the selected column.
        std::vector<FT> get_column(int col) const;

        /// Replace a row.
        /// \param v Values to assign.
        /// \param row Zero-based row index.
        void set_row(const std::vector<FT> &v, int row);

        /// Replace a column.
        /// \param v Values to assign.
        /// \param col Zero-based column index.
        void set_column(const std::vector<FT> &v, int col);

        /// Set all elements to zero.
        void load_zero();

        /// Make this matrix diagonal with the requested diagonal value.
        /// @note This function also allow to set the elements on the diagonal to have values other than 1.
        void load_identity(FT v = FT(1));

        /// Set all elements to a random value (in the range [0.0, 1.0])
        void load_random();

        /// Return the transposed matrix.
        /// \return Matrix with rows and columns exchanged.
        Matrix<FT> transpose() const;

        /// Return the inverse of a square matrix.
        /// \return Inverse matrix; caller must ensure the matrix is invertible.
        Matrix<FT> inverse() const;

        /// Return the determinant of a square matrix.
        /// \return Determinant.
        FT determinant() const;

        /// Return the trace, the sum of the main diagonal.
        /// \return Sum over the diagonal extent.
        /// \note The matrix need not be square.
        FT trace() const;

        /// Add a scalar to every element of this matrix.
        /// \param x Scalar addend.
        /// \return Reference to this matrix.
        Matrix<FT> &operator+=(const FT &x);    //compound assignment operators +=
        /// Subtract a scalar from every element of this matrix.
        /// \param x Scalar subtrahend.
        /// \return Reference to this matrix.
        Matrix<FT> &operator-=(const FT &x);    //compound assignment operators -=
        /// Multiply every element of this matrix by a scalar.
        /// \param x Scalar factor.
        /// \return Reference to this matrix.
        template<typename FT2>
        Matrix<FT> &operator*=(const FT2 &x);   //compound assignment operators *=
        /// Divide every element of this matrix by a scalar.
        /// \param x Divisor.
        /// \return Reference to this matrix.
        template<typename FT2>
        Matrix<FT> &operator/=(const FT2 &x);   //compound assignment operators /=
        /// Add a matrix elementwise to this matrix.
        /// \param rhs Matrix with the same dimensions.
        /// \return Reference to this matrix.
        Matrix<FT> &operator+=(const Matrix<FT> &rhs);

        /// Subtract a matrix elementwise from this matrix.
        /// \param rhs Matrix with the same dimensions.
        /// \return Reference to this matrix.
        Matrix<FT> &operator-=(const Matrix<FT> &rhs);

    private:

        // 0-based data pointer
        FT *data_;

        // 0-based row pointer's pointer
        FT **prow_;

        // row number, column number and total number
        int nRow_;
        int nColumn_;
        long nTotal_;

        void init(int rows, int cols);

        void copy_from_array(const FT *v); // copy matrix from normal array
        void set_by_scalar(const FT &x); // set matrix by a scalar
        void destroy();    // destroy the matrix

        Matrix<FT> cofactor(int i, int j) const;

    }; // class Matrix



    //----------------------------- Utilities for Matrix ------------------------------//

    /// input and output
    template<typename FT>
    std::ostream &operator<<(std::ostream &, const Matrix<FT> &);    // Overload of the output stream.
    template<typename FT>
    std::istream &operator>>(std::istream &, Matrix<FT> &);        // Overload of the input stream.

    /// arithmetic operators
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


    /// multiplication
    template<typename FT>
    void mult(const Matrix<FT> &, const Matrix<FT> &,
              Matrix<FT> &);            // matrix-matrix multiplication (result has already been allocated)
    template<typename FT>
    void mult(const Matrix<FT> &, const std::vector<FT> &,
              std::vector<FT> &);    // matrix-vector multiplication (result has already been allocated)
    template<typename FT>
    Matrix<FT> mult(const Matrix<FT> &, const Matrix<FT> &);        // matrix-matrix multiplication
    template<typename FT>
    std::vector<FT> mult(const Matrix<FT> &, const std::vector<FT> &);    // matrix-vector multiplication

    /// element-wise multiplication / division
    template<typename FT>
    Matrix<FT> elem_mult(const Matrix<FT> &, const Matrix<FT> &);    // "*"
    template<typename FT>
    Matrix<FT> elem_divd(const Matrix<FT> &, const Matrix<FT> &);    // "/"
    template<typename FT>
    Matrix<FT> &elem_mult_eq(Matrix<FT> &, const Matrix<FT> &);        // "*="
    template<typename FT>
    Matrix<FT> &elem_divd_eq(Matrix<FT> &, const Matrix<FT> &);        // "/="

    /// transpose and conjugate transpose
    template<typename FT>
    Matrix<FT> transpose(const Matrix<FT> &);            // matrix transpose
    template<typename FT>
    Matrix<FT> conjugate_transpose(const Matrix<FT> &);    // matrix conjugate transpose

    /// transpose multiplication
    template<typename FT>
    Matrix<FT> transpose_mult(const Matrix<FT> &, const Matrix<FT> &);        // A^T * B
    template<typename FT>
    std::vector<FT> transpose_mult(const Matrix<FT> &, const std::vector<FT> &);    // A^T * b
    template<typename FT>
    Matrix<FT> mult_transpose(const Matrix<FT> &, const Matrix<FT> &);        // A * B^T
    template<typename FT>
    Matrix<FT> mult_transpose(const std::vector<FT> &, const std::vector<FT> &);    // a * b^T

    /// Generate a matrix with all its elements randomly initialized in the range [0.0, 1.0].
    template<typename FT>
    Matrix<FT> random(int row, int col);

    /// Generate an unity/identity matrix (i.e., all elements on the diagonal have a value of 1).
    /// @note This function also allow to set the elements on the diagonal to have values other than 1.
    template<typename FT>
    Matrix<FT> identity(int N, const FT& v = FT(1));

    /// diagonal matrix
    template<typename FT>
    Matrix<FT> diagonal(const std::vector<FT> &);     // Generate a diagonal matrix by given its diagonal elements.
    template<typename FT>
    std::vector<FT> diagonal(const Matrix<FT> &);     // Get the diagonal entries of matrix.

    /// the trace of this matrix, i.e. the sum of the coefficients on the main diagonal.
    /// NOTE: the matrix can be any matrix, not necessarily square.
    template<typename FT>
    FT trace(const Matrix<FT> &);

    /// utilities
    template<typename FT>
    FT norm(const Matrix<FT> &);                // Compute Frobenius norm of matrix.
    template<typename FT>
    void swap(Matrix<FT> &, Matrix<FT> &);    // Swap two matrices.
    template<typename FT>
    std::vector<FT> sum(const Matrix<FT> &);    // Matrix's column vectors sum.
    template<typename FT>
    std::vector<FT> min(const Matrix<FT> &);    // Minimum of matrix's column vectors.
    template<typename FT>
    std::vector<FT> max(const Matrix<FT> &);    // Maximum of matrix's column vectors.
    template<typename FT>
    std::vector<FT> mean(const Matrix<FT> &);    // Matrix's column vectors mean.
    template<typename FT>
    Matrix<FT> abs(const Matrix<std::complex < FT>

    > &A);// Get magnitude of a std::complex matrix.
    template<typename FT>
    Matrix<FT> arg(const Matrix<std::complex < FT>

    > &A);// Get angle of a std::complex matrix.
    template<typename FT>
    Matrix<FT> real(const Matrix<std::complex < FT>

    > &A);// Get real part of a std::complex matrix.
    template<typename FT>
    Matrix<FT> imag(const Matrix<std::complex < FT>

    > &A);// Get imaginary part of a std::complex matrix.

    /// Convert real matrix to complex matrix.
    template<typename FT>
    Matrix<std::complex < FT> >

    complex_matrix(const Matrix<FT> &);

    template<typename FT>
    Matrix<std::complex < FT> >

    complex_matrix(const Matrix<FT> &, const Matrix<FT> &); // A for real, B for imaginary


    //----------------------------- Utilities for std::vector ------------------------------//

    /// input and output
    template<typename FT>
    std::ostream &operator<<(std::ostream &, const std::vector<FT> &);// Overload of the output stream.
    template<typename FT>
    std::istream &operator>>(std::istream &, std::vector<FT> &);        // Overload of the input stream.

    /// arithmetic operators
    template<typename FT>
    std::vector<FT> operator-(const std::vector<FT> &);    // get negative vector
    template<typename FT>
    std::vector<FT> operator+(const std::vector<FT> &, const std::vector<FT> &);    // vector-vector addition
    template<typename FT>
    std::vector<FT> operator+(const std::vector<FT> &, const FT &);    // vector-scalar addition
    template<typename FT>
    std::vector<FT> operator+(const FT &, const std::vector<FT> &);    // scalar-vector addition
    template<typename FT>
    std::vector<FT> operator-(const std::vector<FT> &, const std::vector<FT> &);    // vector-vector subtraction
    template<typename FT>
    std::vector<FT> operator-(const std::vector<FT> &, const FT &);    // vector-scalar subtraction
    template<typename FT>
    std::vector<FT> operator-(const FT &, const std::vector<FT> &);    // scalar-vector subtraction
    template<typename FT1, typename FT2>
    std::vector<FT1> operator*(const std::vector<FT1> &, const FT2 &);    // complex vector-scalar multiplication
    template<typename FT1, typename FT2>
    std::vector<FT1> operator*(const FT2 &, const std::vector<FT1> &);    // scalar-complex vector multiplication
    template<typename FT1, typename FT2>
    std::vector<FT1> operator/(const std::vector<FT1> &, const FT2 &);    // complex vector-scalar division
    template<typename FT1, typename FT2>
    std::vector<FT1> operator/(const FT2 &, const std::vector<FT1> &);    // scalar-complex vector division


    /// element-wise multiplication / division
    template<typename FT>
    std::vector<FT> elem_mult(const std::vector<FT> &, const std::vector<FT> &);    // "*"
    template<typename FT>
    std::vector<FT> elem_divd(const std::vector<FT> &, const std::vector<FT> &);    // "/"
    template<typename FT>
    std::vector<FT> &elem_mult_eq(std::vector<FT> &, const std::vector<FT> &);    // "*="
    template<typename FT>
    std::vector<FT> &elem_divd_eq(std::vector<FT> &, const std::vector<FT> &);    // "/="

    /// dot product
    template<typename FT>
    FT operator*(const std::vector<FT> &, const std::vector<FT> &);    // Inner product for vectors.
    template<typename FT>
    FT dot(const std::vector<FT> &, const std::vector<FT> &);            // Inner product for vectors.

    /// Generate a vector with elements randomly initialized in the range [0, 1].
    /// \param size Number of elements.
    /// \return Randomly initialized vector.
    template<typename FT>
    std::vector<FT> random(int size);

    /// utilities
    template<typename FT>
    FT norm(const std::vector<FT> &);                 /// Euclidean norm.
    template<typename FT>
    FT norm(const std::vector<std::complex<FT>> &);   /// Euclidean norm.
    template<typename FT>
    void swap(std::vector<FT> &, std::vector<FT> &); /// Swap two vectors.
    template<typename FT>
    std::vector<FT> linspace(FT, FT, int); /// Generates a vector of n points linearly spaced between and including a and b.
    template<typename FT>
    FT sum(const std::vector<FT> &);    /// vector sum.
    template<typename FT>
    FT min(const std::vector<FT> &);    /// Minimum value of vector.
    template<typename FT>
    FT max(const std::vector<FT> &);    /// Maximum value of vector.
    template<typename FT>
    FT mean(const std::vector<FT> &);   /// Mean value of vector.

    template<typename FT>
    std::vector<FT> abs(const std::vector<std::complex<FT>> &);    /// Get magnitude of a complex vector.
    template<typename FT>
    std::vector<FT> arg(const std::vector<std::complex<FT>> &);    /// Get angle of a complex vector.
    template<typename FT>
    std::vector<FT> real(const std::vector<std::complex<FT>> &);   /// Get real part of a complex vector.
    template<typename FT>
    std::vector<FT> imag(const std::vector<std::complex<FT>> &);   /// Get imaginary part of a complex vector.

    /// Convert a real vector to a complex vector with zero imaginary parts.
    /// \param rv Input real vector.
    /// \return Complex-valued vector.
    template<typename FT>
    std::vector<std::complex<FT>> complex_vector(const std::vector<FT> &);

    template<typename FT>
    std::vector<std::complex<FT>>
    complex_vector(const std::vector<FT> &, const std::vector<FT> &); // A for real, B for imaginary
}

    //-----------------------------  Matrix  Implementation ------------------------------------//

#include <cassert>

namespace easy3d {

    /**
    * initialize
    */
    template<typename FT>
    void Matrix<FT>::init(int rows, int cols) {
        nRow_ = rows;
        nColumn_ = cols;
        nTotal_ = nRow_ * nColumn_;

        data_ = new FT[nTotal_];
        prow_ = new FT *[nRow_];

        assert(data_ != NULL);
        assert(prow_ != NULL);

        FT *p = data_;
        for (int i = 0; i < nRow_; ++i) {
            prow_[i] = p;
            p += nColumn_;
        }
    }


    /**
    * copy matrix from normal array
    */
    template<typename FT>
    inline void Matrix<FT>::copy_from_array(const FT *v) {
        for (long i = 0; i < nTotal_; ++i)
            data_[i] = v[i];
    }


    /**
    * set matrix by a scalar
    */
    template<typename FT>
    inline void Matrix<FT>::set_by_scalar(const FT &x) {
        for (long i = 0; i < nTotal_; ++i)
            data_[i] = x;
    }


    /**
    * destroy the matrix
    */
    template<typename FT>
    void Matrix<FT>::destroy() {
        if (data_ == NULL)
            return;
        else
            delete[] data_;

        if (prow_ != NULL)
            delete[] prow_;
    }


    template<typename FT>
    Matrix<FT> Matrix<FT>::cofactor(int _i, int _j) const {
        assert(0 <= _i && _i < nRow_ && 0 <= _j && _j < nColumn_);
        Matrix<FT> mat(nRow_ - 1, nColumn_ - 1);
        for(int i = 0; i < mat.rows(); i++) {
            for(int j = 0; j < mat.cols(); j++){
                if(i < _i){
                    if(j < _j){
                        mat.data()[i * mat.cols() + j] = data_[i * nColumn_ + j];
                    } else {
                        mat.data()[i * mat.cols() + j] = data_[i * nColumn_ + (j + 1)];
                    }
                } else {
                    if(j < _j){
                        mat.data()[i * mat.cols() + j] = data_[(i + 1) * nColumn_ + j];
                    } else {
                        mat.data()[i * mat.cols() + j] = data_[(i + 1) * nColumn_ + (j + 1)];
                    }
                }
            }
        }
        return mat;
    }



    /**
    * constructors and destructor
    */
    template<typename FT>
    Matrix<FT>::Matrix()
            : data_(0), prow_(0), nRow_(0), nColumn_(0), nTotal_(0) {
    }

    template<typename FT>
    Matrix<FT>::Matrix(const Matrix<FT> &A) {
        init(A.nRow_, A.nColumn_);
        copy_from_array(A.data_);
    }

    template<typename FT>
    Matrix<FT>::Matrix(int rows, int cols, const FT &x) {
        init(rows, cols);
        set_by_scalar(x);
    }

    template<typename FT>
    Matrix<FT>::Matrix(int rows, int cols, const std::vector<FT> &array) {

        init(rows, cols);
        copy_from_array(array.data());
    }

    template<typename FT>
    Matrix<FT>::Matrix(int rows, int cols, const FT *array) {
        init(rows, cols);
        copy_from_array(array);
    }

    template<typename FT>
    Matrix<FT>::~Matrix() {
        destroy();
    }


    /**
    * overload evaluate operator = from matrix to matrix
    */
    template<typename FT>
    Matrix<FT> &Matrix<FT>::operator=(const Matrix<FT> &A) {
        if (data_ == A.data_)
            return *this;

        if (nRow_ == A.nRow_ && nColumn_ == A.nColumn_)
            copy_from_array(A.data_);
        else {
            destroy();
            init(A.nRow_, A.nColumn_);
            copy_from_array(A.data_);
        }

        return *this;
    }


    /**
    * overload evaluate operator = from scalar to matrix
    */
    template<typename FT>
    inline Matrix<FT> &Matrix<FT>::operator=(const FT &x) {
        set_by_scalar(x);
        return *this;
    }


    /**
    * overload operator [] for 0-offset access
    */
    template<typename FT>
    inline FT *Matrix<FT>::operator[](int row) {
        assert(0 <= row);
        assert(row < nRow_);
        return prow_[row];
    }

    template<typename FT>
    inline const FT *Matrix<FT>::operator[](int row) const {
        assert(0 <= row);
        assert(row < nRow_);
        return prow_[row];
    }

    template<typename FT>
    inline FT &Matrix<FT>::operator()(int row, int col) {
        assert(0 <= row);
        assert(row < nRow_);
        assert(0 <= col);
        assert(col < nColumn_);
        return prow_[row][col];
    }

    template<typename FT>
    inline const FT &Matrix<FT>::operator()(int row, int col) const {
        assert(0 <= row);
        assert(row < nRow_);
        assert(0 <= col);
        assert(col < nColumn_);
        return prow_[row][col];
    }


    template<typename FT>
    inline
    void Matrix<FT>::set(int row, int col, FT v) {
        assert(0 <= row);
        assert(row < nRow_);
        assert(0 <= col);
        assert(col < nColumn_);
        prow_[row][col] = v;
    }

    template<typename FT>
    inline
    const FT &Matrix<FT>::get(int row, int col) const {
        assert(0 <= row);
        assert(row < nRow_);
        assert(0 <= col);
        assert(col < nColumn_);
        return prow_[row][col];
    }

    /**
     * Clears the matrix such that all values are set to 0 (zero)
     */
    template<typename FT>
    inline void Matrix<FT>::load_zero() {
        for (int i = 0; i < nRow_; i++) {
            for (int j = 0; j < nColumn_; j++) {
                prow_[i][j] = FT(0);
            }
        }
    }

    /**
     * \brief Set this matrix to a rectangular identity matrix.
     * \param v Value placed on the main diagonal (defaults to one).
     * Entries outside the diagonal are set to zero.
     */
    template<typename FT>
    inline void Matrix<FT>::load_identity(FT v /* = FT(1.0)*/) {
        for (int i = 0; i < nRow_; i++) {
            for (int j = 0; j < nColumn_; j++) {
                prow_[i][j] = (i == j) ? v : FT(0);
            }
        }
    }

    /// Set all elements to a random value (in the range [0.0, 1.0])
    template<typename FT>
    inline void Matrix<FT>::load_random() {
        for (int i = 0; i < nRow_; i++) {
            for (int j = 0; j < nColumn_; j++) {
                prow_[i][j] = random_float();
            }
        }
    }

    template<typename FT>
    inline int Matrix<FT>::rows() const {
        return nRow_;
    }

    template<typename FT>
    inline int Matrix<FT>::cols() const {
        return nColumn_;
    }


    /**
    * reallocate matrix's size
    */
    template<typename FT>
    Matrix<FT> &Matrix<FT>::resize(int rows, int cols) {
        if (rows == nRow_ && cols == nColumn_)
            return *this;

        destroy();
        init(rows, cols);

        return *this;
    }


    /**
    * get the matrix's row vector
    */
    template<typename FT>
    std::vector<FT> Matrix<FT>::get_row(int row) const {
        assert(0 <= row);
        assert(row < nRow_);
        std::vector<FT> tmp(nColumn_);
        for (int j = 0; j < nColumn_; ++j)
            tmp[j] = prow_[row][j];

        return tmp;
    }


    /**
    * get the matrix's column vector
    */
    template<typename FT>
    std::vector<FT> Matrix<FT>::get_column(int col) const {
        assert(0 <= col);
        assert(col < nColumn_);
        std::vector<FT> tmp(nRow_);
        for (int i = 0; i < nRow_; ++i)
            tmp[i] = prow_[i][col];

        return tmp;
    }


    /**
    * set the matrix's row vector
    */
    template<typename FT>
    void Matrix<FT>::set_row(const std::vector<FT> &v, int row) {
        assert(0 <= row);
        assert(row < nRow_);
        assert(v.size() == nColumn_);
        for (int j = 0; j < nColumn_; ++j)
            prow_[row][j] = v[j];
    }


    /**
    * set the matrix's column vector
    */
    template<typename FT>
    void Matrix<FT>::set_column(const std::vector<FT> &v, int col) {
        assert(0 <= col);
        assert(col < nColumn_);
        assert(v.size() == nRow_);
        for (int i = 0; i < nRow_; ++i)
            prow_[i][col] = v[i];
    }


    template<typename FT>
    Matrix<FT> Matrix<FT>::transpose() const {
        Matrix<FT> t(nColumn_, nRow_);
        for (int r = 0; r < nRow_; r++) {
            for (int c = 0; c < nColumn_; c++) {
                t(c, r) = (*this)(r, c);
            }
        }
        return t;
    }


    template<typename FT>
    FT Matrix<FT>::determinant() const {
        assert(nRow_ == nColumn_ && nRow_ != 0);
        if(nRow_ == 1) {
            return data_[0];
        } else if(nRow_ == 2) {
            return data_[0]*data_[3] - data_[1]*data_[2];
        } else if(nRow_ == 3) {
            return - data_[8]*data_[1]*data_[3] - data_[7]*data_[5]*data_[0] - data_[2]*data_[4]*data_[6]
                   + data_[6]*data_[1]*data_[5] + data_[7]*data_[3]*data_[2] + data_[0]*data_[4]*data_[8];
        } else {
            FT value = FT();
            for(int i = 0; i < nRow_; i++){
                value += std::pow(-1.0, i) * data_[i * nColumn_] * cofactor(i, 0).determinant();
            }
            return value;
        }
    }


    template<typename FT>
    Matrix<FT> Matrix<FT>::inverse() const {
        assert(nRow_ == nColumn_);
        Matrix<FT> mat = Matrix<FT>(nRow_, nColumn_);
        if(nRow_ == 1){
            mat.data()[0] = 1.0 / data_[0];
            return mat;
        } else {
            for(int i = 0; i < nRow_; i++){
                for(int j = 0; j < nColumn_; j++){
                    mat.data()[i * mat.cols() + j] = std::pow(-1.0, i + j) * cofactor(j, i).determinant();
                }
            }
            return mat / determinant();
        }
    }


    template<typename FT>
    FT Matrix<FT>::trace() const {
        int range = std::min(nRow_, nColumn_);
        FT trac = 0.0;
        for (int i = 0; i < range; i++) {
            trac += (*this)[i][i];
        }
        return trac;
    }

    template<typename FT>
    Matrix<FT> &Matrix<FT>::operator+=(const FT &x) {
        FT **rowPtr = prow_;
        FT *colPtr = 0;

        for (int i = 0; i < nRow_; ++i) {
            colPtr = *rowPtr++;
            for (int j = 0; j < nColumn_; ++j)
                *colPtr++ += x;
        }

        return *this;
    }

    template<typename FT>
    Matrix<FT> &Matrix<FT>::operator+=(const Matrix<FT> &rhs) {
        assert(nRow_ == rhs.rows());
        assert(nColumn_ == rhs.cols());

        FT **rowPtrL = prow_;
        FT *colPtrL = 0;
        FT **rowPtrR = rhs.prow_;
        const FT *colPtrR = 0;

        for (int i = 0; i < nRow_; ++i) {
            colPtrL = *rowPtrL++;
            colPtrR = *rowPtrR++;
            for (int j = 0; j < nColumn_; ++j)
                *colPtrL++ += *colPtrR++;
        }

        return *this;
    }


    template<typename FT>
    Matrix<FT> &Matrix<FT>::operator-=(const FT &x) {
        FT **rowPtr = prow_;
        FT *colPtr = 0;

        for (int i = 0; i < nRow_; ++i) {
            colPtr = *rowPtr++;
            for (int j = 0; j < nColumn_; ++j)
                *colPtr++ -= x;
        }

        return *this;
    }

    template<typename FT>
    Matrix<FT> &Matrix<FT>::operator-=(const Matrix<FT> &rhs) {
        assert(nRow_ == rhs.rows());
        assert(nColumn_ == rhs.cols());

        FT **rowPtrL = prow_;
        FT *colPtrL = 0;
        FT **rowPtrR = rhs.prow_;
        const FT *colPtrR = 0;

        for (int i = 0; i < nRow_; ++i) {
            colPtrL = *rowPtrL++;
            colPtrR = *rowPtrR++;
            for (int j = 0; j < nColumn_; ++j)
                *colPtrL++ -= *colPtrR++;
        }

        return *this;
    }


    template<typename FT>
    template<typename FT2>
    Matrix<FT> &Matrix<FT>::operator*=(const FT2 &x) {
        FT **rowPtr = prow_;
        FT *colPtr = 0;

        for (int i = 0; i < nRow_; ++i) {
            colPtr = *rowPtr++;
            for (int j = 0; j < nColumn_; ++j)
                *colPtr++ *= x;
        }

        return *this;
    }


    template<typename FT>
    template<typename FT2>
    Matrix<FT> &Matrix<FT>::operator/=(const FT2 &x) {
        FT **rowPtr = prow_;
        FT *colPtr = 0;

        for (int i = 0; i < nRow_; ++i) {
            colPtr = *rowPtr++;
            for (int j = 0; j < nColumn_; ++j)
                *colPtr++ /= x;
        }

        return *this;
    }

    /**
     * \brief Write a matrix as its dimensions followed by its rows.
     * \param out Output stream.
     * \param A Matrix to write.
     * \return The output stream.
     */
    template<typename FT>
    std::ostream &operator<<(std::ostream &out, const Matrix<FT> &A) {
        int rows = A.rows();
        int cols = A.cols();

        out << "size: " << rows << " by " << cols << "\n";
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


    /**
     * \brief Read matrix dimensions and row-major element values.
     * \param in Input stream.
     * \param A Receives the matrix data and is resized when needed.
     * \return The input stream.
     */
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


    /**
     * \brief Return the elementwise negation of a matrix.
     * \param A Input matrix.
     * \return Matrix with every element negated.
     */
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


    /// Add a scalar to every matrix element.
    /// \param A Input matrix.
    /// \param x Scalar addend.
    /// \return Resulting matrix.
    template<typename FT>
    inline Matrix<FT> operator+(const Matrix<FT> &A, const FT &x) {
        Matrix<FT> tmp(A);
        return tmp += x;
    }

    /// Add a scalar to every matrix element.
    /// \param x Scalar addend.
    /// \param A Input matrix.
    /// \return Resulting matrix.
    template<typename FT>
    inline Matrix<FT> operator+(const FT &x, const Matrix<FT> &A) {
        return A + x;
    }


    /**
     * \brief Add two matrices elementwise.
     * \param A1 First matrix.
     * \param A2 Second matrix with matching dimensions.
     * \return Sum matrix.
     */
    template<typename FT>
    inline Matrix<FT> operator+(const Matrix<FT> &A1, const Matrix<FT> &A2) {
        Matrix<FT> tmp(A1);
        return tmp += A2;
    }


    /// Subtract a scalar from every matrix element.
    /// \param A Input matrix.
    /// \param x Scalar subtrahend.
    /// \return Resulting matrix.
    template<typename FT>
    inline Matrix<FT> operator-(const Matrix<FT> &A, const FT &x) {
        Matrix<FT> tmp(A);
        return tmp -= x;
    }

    /// Subtract every matrix element from a scalar.
    /// \param x Scalar minuend.
    /// \param A Input matrix.
    /// \return Resulting matrix.
    template<typename FT>
    inline Matrix<FT> operator-(const FT &x, const Matrix<FT> &A) {
        Matrix<FT> tmp(A);
        return -tmp += x;
    }


    /// Subtract two matrices elementwise.
    /// \param A1 First matrix.
    /// \param A2 Second matrix with matching dimensions.
    /// \return Difference matrix.
    template<typename FT>
    inline Matrix<FT> operator-(const Matrix<FT> &A1, const Matrix<FT> &A2) {
        Matrix<FT> tmp(A1);
        return tmp -= A2;
    }

    /**
     * \brief Multiply two matrices.
     * \param A1 Left matrix.
     * \param A2 Right matrix; its row count must match A1's column count.
     * \return Product matrix.
     */
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


    /**
     * \brief Multiply a matrix by a vector.
     * \param A Matrix.
     * \param b Vector with one value per matrix column.
     * \return Product vector with one value per matrix row.
     */
    template<typename FT>
    std::vector<FT> operator*(const Matrix<FT> &A, const std::vector<FT> &b) {
        assert(A.cols() == b.size());

        int rows = A.rows();
        //	int cols = A.cols();

        std::vector<FT> tmp(rows);
        mult(A, b, tmp);

        return tmp;
    }

    /// Multiply every matrix element by a scalar.
    /// \param A Input matrix.
    /// \param s Scalar factor.
    /// \return Scaled matrix.
    template<typename FT1, typename FT2>
    inline
    Matrix<FT1> operator*(const Matrix<FT1> &A, const FT2 &s) {
        Matrix<FT1> tmp(A);
        return tmp *= s;
    }

    /// Multiply every matrix element by a scalar.
    /// \param s Scalar factor.
    /// \param A Input matrix.
    /// \return Scaled matrix.
    template<typename FT1, typename FT2>
    inline
    Matrix<FT1> operator*(const FT2 &s, const Matrix<FT1> &A) {
        return A * s;
    }

    /// Divide every matrix element by a scalar.
    /// \param A Input matrix.
    /// \param s Divisor.
    /// \return Scaled matrix.
    template<typename FT1, typename FT2>
    inline
    Matrix<FT1> operator/(const Matrix<FT1> &A, const FT2 &s) {
        Matrix<FT1> tmp(A);
        return tmp /= s;
    }

    /// Divide a scalar by each matrix element.
    /// \param s Scalar numerator.
    /// \param A Matrix of divisors.
    /// \return Elementwise quotient matrix.
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

    /**
     * This is an optimized version of matrix-matrix multiplication,
     * where the destination matrix has already been allocated.
     * \param A Left matrix.
     * \param B Right matrix; row count must equal A's column count.
     * \param C Receives the product and is resized to the result dimensions.
     */
    template<typename FT>
    void mult(const Matrix<FT> &A, const Matrix<FT> &B, Matrix<FT> &C) {
        int M = A.rows();
        int N = B.cols();
        int K = A.cols();

        assert(B.rows() == K);

        C.resize(M, N);
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
    }


    /**
     * This is an optimized version of matrix-vector multiplication,
     * where the destination vector has already been allocated.
     * \param A Matrix.
     * \param b Vector with one value per matrix column.
     * \param c Receives the product and is resized to the row count.
     */
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


    /**
     * This is an optimized version of matrix-matrix multiplication,
     * that returns a newly allocated result.
     * \param A Left matrix.
     * \param B Right matrix; row count must equal A's column count.
     * \return Product matrix.
     */
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


    /**
     * This is an optimized version of matrix-vector multiplication,
     * that returns a newly allocated result.
     * \param A Matrix.
     * \param b Vector with one value per matrix column.
     * \return Product vector with one value per matrix row.
     */
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


    /**
     * \brief Multiply two matrices elementwise.
     * \param A1 First matrix.
     * \param A2 Second matrix with matching dimensions.
     * \return Elementwise product.
     */
    template<typename FT>
    inline Matrix<FT> elem_mult(const Matrix<FT> &A1, const Matrix<FT> &A2) {
        Matrix<FT> tmp(A1);
        return tmp *= A2;
    }

    /**
     * \brief Multiply a matrix elementwise in place.
     * \param A1 Matrix to update.
     * \param A2 Elementwise factors with matching dimensions.
     * \return Reference to the updated matrix.
     */
    template<typename FT>
    inline Matrix<FT> &elem_mult_eq(Matrix<FT> &A1, const Matrix<FT> &A2) {
        return A1 *= A2;
    }


    /**
     * \brief Divide two matrices elementwise.
     * \param A1 Numerator matrix.
     * \param A2 Divisor matrix with matching dimensions.
     * \return Elementwise quotient.
     */
    template<typename FT>
    inline Matrix<FT> elem_divd(const Matrix<FT> &A1, const Matrix<FT> &A2) {
        Matrix<FT> tmp(A1);
        return tmp /= A2;
    }

    /**
     * \brief Divide a matrix elementwise in place.
     * \param A1 Matrix to update.
     * \param A2 Elementwise divisors with matching dimensions.
     * \return Reference to the updated matrix.
     */
    template<typename FT>
    inline Matrix<FT> &elem_divd_eq(Matrix<FT> &A1, const Matrix<FT> &A2) {
        return A1 /= A2;
    }


    /**
     * \brief Return the transpose of a matrix.
     * \param A Input matrix.
     * \return Transposed matrix.
     */
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


    /**
     * \brief Return the conjugate transpose of a matrix.
     * \param A Input matrix.
     * \return Conjugate-transposed matrix.
     */
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


    /**
     * \brief Compute A^T * B.
     * \param A1 Left operand before transposition.
     * \param A2 Right operand; row count must match A1.
     * \return Product matrix.
     */
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


    /**
     * \brief Compute A^T * v.
     * \param A Matrix operand.
     * \param v Vector whose length equals A's row count.
     * \return Product vector with one value per column of A.
     */
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


    /**
     * \brief Compute A * B^T.
     * \param A1 Left matrix.
     * \param A2 Right matrix before transposition; column counts must match.
     * \return Product matrix.
     */
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


    /**
     * \brief Compute the outer product a * b^T.
     * \param a First vector.
     * \param b Second vector.
     * \return Matrix with a.size() rows and b.size() columns.
     */
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


    /// Generate a matrix with elements sampled in the range [0, 1].
    /// \param row Number of rows.
    /// \param col Number of columns.
    /// \return Randomly initialized matrix.
    template<typename FT>
    Matrix<FT> random(int row, int col) {
        Matrix<FT> tmp(row, col);
        for (int i = 0; i < row; i++) {
            for (int j = 0; j < col; j++) {
                tmp[i][j] = random_float();
            }
        }
        return tmp;
    }

    /**
     * \brief Generate a square diagonal matrix.
     * \param N Number of rows and columns.
     * \param x Value placed on the main diagonal (defaults to one).
     * \return Matrix with x on the diagonal and zero elsewhere.
     */
    template<typename FT>
    Matrix<FT> identity(int N, const FT& x /*= FT(1)*/) {
        Matrix<FT> tmp(N, N);
        for (int i = 0; i < N; ++i)
            tmp[i][i] = x;

        return tmp;
    }


    /**
     * \brief Extract the main diagonal of a matrix.
     * \param A Input matrix.
     * \return Diagonal entries, up to the smaller matrix dimension.
     */
    template<typename FT>
    std::vector<FT> diagonal(const Matrix<FT> &A) {
        int nColumn_ = A.rows();
        if (nColumn_ > A.cols())
            nColumn_ = A.cols();

        std::vector<FT> tmp(nColumn_);
        for (int i = 0; i < nColumn_; ++i)
            tmp[i] = A[i][i];

        return tmp;
    }


    /**
     * \brief Construct a square matrix from diagonal entries.
     * \param d Values to place on the main diagonal.
     * \return Square diagonal matrix.
     */
    template<typename FT>
    Matrix<FT> diagonal(const std::vector<FT> &d) {
        int N = static_cast<int>(d.size());

        Matrix<FT> tmp(N, N);
        for (int i = 0; i < N; ++i)
            tmp[i][i] = d[i];

        return tmp;
    }


    /**
     * Compute the trace of this matrix, i.e. the sum of the coefficients on the main diagonal.
     * NOTE: the matrix can be any matrix, not necessarily square.
     * \param A Input matrix.
     * \return Sum of the main-diagonal entries up to the smaller dimension.
     */
    template<typename FT>
    FT trace(const Matrix<FT> &A) {
        int range = std::min(A.rows(), A.cols());
        FT trac = 0.0;
        for (int i = 0; i < range; i++) {
            trac += A[i][i];
        }
        return trac;
    }


    /**
     * \brief Compute the Frobenius norm.
     * \param A Input matrix.
     * \return Square root of the sum of squared element magnitudes for real-valued matrices.
     */
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


    /**
     * \brief Swap corresponding elements of equally sized matrices.
     * \param lhs First matrix.
     * \param rhs Second matrix; dimensions must match lhs.
     */
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


    /**
     * \brief Sum each column.
     * \param A Input matrix.
     * \return Vector containing one sum per column.
     */
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


    /**
     * \brief Find the minimum value in each column.
     * \param A Input matrix; must have at least one row.
     * \return Vector containing one minimum per column.
     */
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


    /**
     * \brief Find the maximum value in each column.
     * \param A Input matrix; must have at least one row.
     * \return Vector containing one maximum per column.
     */
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


    /**
     * \brief Compute the mean of each column.
     * \param A Input matrix; must have at least one row.
     * \return Vector containing one mean per column.
     */
    template<typename FT>
    inline std::vector<FT> mean(const Matrix<FT> &A) {
        return sum(A) / FT(A.rows());
    }


    /// Return the magnitude of each complex matrix element.
    /// \param A Input complex matrix.
    /// \return Real-valued matrix of magnitudes.
    template<typename FT>
    Matrix<FT> abs(const Matrix<std::complex<FT> > &A) {
        int m = A.rows(),
                n = A.cols();
        Matrix<FT> tmp(m, n);

        for (int i = 0; i < m; ++i)
            for (int j = 0; j < n; ++j)
                tmp[i][j] = std::abs(A[i][j]);

        return tmp;
    }

    /// Return the phase angle of each complex matrix element, in radians.
    /// \param A Input complex matrix.
    /// \return Real-valued matrix of phase angles.
    template<typename FT>
    Matrix<FT> arg(const Matrix<std::complex<FT> > &A) {
        int m = A.rows(),
                n = A.cols();
        Matrix<FT> tmp(m, n);

        for (int i = 0; i < m; ++i)
            for (int j = 0; j < n; ++j)
                tmp[i][j] = arg(A[i][j]);

        return tmp;
    }

    /// Extract the real part of every complex matrix element.
    /// \param A Input complex matrix.
    /// \return Real-valued matrix.
    template<typename FT>
    Matrix<FT> real(const Matrix<std::complex<FT> > &A) {
        int m = A.rows(),
                n = A.cols();
        Matrix<FT> tmp(m, n);

        for (int i = 0; i < m; ++i)
            for (int j = 0; j < n; ++j)
                tmp[i][j] = A[i][j].real();

        return tmp;
    }


    /// Extract the imaginary part of every complex matrix element.
    /// \param A Input complex matrix.
    /// \return Real-valued matrix.
    template<typename FT>
    Matrix<FT> imag(const Matrix<std::complex<FT> > &A) {
        int m = A.rows(),
                n = A.cols();
        Matrix<FT> tmp(m, n);

        for (int i = 0; i < m; ++i)
            for (int j = 0; j < n; ++j)
                tmp[i][j] = A[i][j].imag();

        return tmp;
    }


    /// Convert a real matrix to a complex matrix with zero imaginary parts.
    /// \param rA Input real matrix.
    /// \return Complex-valued matrix.
    template<typename FT>
    Matrix<std::complex<FT> > complex_matrix(const Matrix<FT> &rA) {
        int rows = rA.rows();
        int cols = rA.cols();

        Matrix<std::complex<FT> > cA(rows, cols);
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < cols; ++j)
                cA[i][j] = rA[i][j];

        return cA;
    }

    /// Combine real and imaginary component matrices into a complex matrix.
    /// \param mR Real components.
    /// \param mI Imaginary components with the same dimensions.
    /// \return Complex-valued matrix.
    template<typename FT>
    Matrix<std::complex<FT> > complex_matrix(const Matrix<FT> &mR, const Matrix<FT> &mI) {
        int rows = mR.rows();
        int cols = mR.cols();

        assert(rows == mI.rows());
        assert(cols == mI.cols());

        Matrix<std::complex<FT> > cA(rows, cols);
        for (int i = 0; i < rows; ++i)
            for (int j = 0; j < cols; ++j)
                cA[i][j] = std::complex<FT>(mR[i][j], mI[i][j]);

        return cA;
    }


    //-----------------------------  std::vector  Implementation ------------------------------------//


    /// Write a vector as its size followed by its elements.
    /// \param out Output stream.
    /// \param A Vector to write.
    /// \return The output stream.
    template<typename FT>
    std::ostream &operator<<(std::ostream &out, const std::vector<FT> &A) {
        out << "size: " << A.size() << "\n";
        for (std::size_t i = 0; i < A.size(); ++i)
            out << A[i] << "\t";
        out << "\n";
        return out;
    }


    /// Read a vector encoded as its size followed by its elements.
    /// \param in Input stream.
    /// \param A Receives the vector contents.
    /// \return The input stream.
    template<typename FT>
    std::istream &operator>>(std::istream &in, std::vector<FT> &A) {
        int size;
        in >> size;
        A.resize(size);
        for (int i = 0; i < size; ++i)
            in >> A[i];
        return in;
    }


    /// Negate every element of a vector.
    /// \param A Input vector.
    /// \return Negated vector.
    template<typename FT>
    std::vector<FT> operator-(const std::vector<FT> &A) {
        std::vector<FT> tmp = A;
        for (std::size_t i = 0; i < tmp.size(); ++i)
            tmp[i] = -tmp[i];

        return tmp;
    }


    /// Add two vectors elementwise.
    /// \param A First vector.
    /// \param B Second vector of equal size.
    /// \return Sum vector.
    template<typename FT>
    std::vector<FT> operator+(const std::vector<FT> &A, const std::vector<FT> &B) {
        assert(A.size() == B.size());

        std::vector<FT> tmp = A;
        for (std::size_t i = 0; i < B.size(); ++i)
            tmp[i] += B[i];

        return tmp;
    }

    /// Add a scalar to every vector element.
    /// \param A Input vector.
    /// \param s Scalar addend.
    /// \return Sum vector.
    template<typename FT>
    std::vector<FT> operator+(const std::vector<FT> &A, const FT &s) {
        std::vector<FT> tmp = A;
        for (std::size_t i = 0; i < A.size(); ++i)
            tmp[i] += s;

        return tmp;
    }

    /// Add a scalar to every vector element.
    /// \param s Scalar addend.
    /// \param A Input vector.
    /// \return Sum vector.
    template<typename FT>
    std::vector<FT> operator+(const FT &s, const std::vector<FT> &A) {
        std::vector<FT> tmp = A;
        for (std::size_t i = 0; i < A.size(); ++i)
            tmp[i] += s;

        return tmp;
    }

    /// Subtract two vectors elementwise.
    /// \param A First vector.
    /// \param B Second vector of equal size.
    /// \return Difference vector.
    template<typename FT>
    std::vector<FT> operator-(const std::vector<FT> &A, const std::vector<FT> &B) {
        assert(A.size() == B.size());

        std::vector<FT> tmp = A;
        for (std::size_t i = 0; i < B.size(); ++i)
            tmp[i] -= B[i];

        return tmp;
    }

    /// Subtract a scalar from every vector element.
    /// \param A Input vector.
    /// \param s Scalar subtrahend.
    /// \return Difference vector.
    template<typename FT>
    std::vector<FT> operator-(const std::vector<FT> &A, const FT &s) {
        std::vector<FT> tmp = A;
        for (std::size_t i = 0; i < A.size(); ++i)
            tmp[i] -= s;

        return tmp;
    }

    /// Subtract each vector element from a scalar.
    /// \param s Scalar minuend.
    /// \param A Input vector.
    /// \return Difference vector.
    template<typename FT>
    std::vector<FT> operator-(const FT &s, const std::vector<FT> &A) {
        std::vector<FT> tmp = A;
        for (std::size_t i = 0; i < A.size(); ++i)
            tmp[i] = s - tmp[i];

        return tmp;
    }

    /// Multiply every vector element by a scalar.
    /// \param A Input vector.
    /// \param s Scalar factor.
    /// \return Scaled vector.
    template<typename FT1, typename FT2>
    std::vector<FT1> operator*(const std::vector<FT1> &A, const FT2 &s) {
        std::vector<FT1> tmp = A;
        for (std::size_t i = 0; i < A.size(); ++i)
            tmp[i] *= s;

        return tmp;
    }

    /// Multiply every vector element by a scalar.
    /// \param s Scalar factor.
    /// \param A Input vector.
    /// \return Scaled vector.
    template<typename FT1, typename FT2>
    std::vector<FT1> operator*(const FT2 &s, const std::vector<FT1> &A) {
        return A * s;
    }

    /// Divide every vector element by a scalar.
    /// \param A Input vector.
    /// \param s Divisor.
    /// \return Quotient vector.
    template<typename FT1, typename FT2>
    std::vector<FT1> operator/(const std::vector<FT1> &A, const FT2 &s) {
        std::vector<FT1> tmp = A;
        for (std::size_t i = 0; i < A.size(); ++i)
            tmp[i] /= s;

        return tmp;
    }

    /// Divide a scalar by every vector element.
    /// \param s Scalar numerator.
    /// \param A Vector of divisors.
    /// \return Quotient vector.
    template<typename FT1, typename FT2>
    std::vector<FT1> operator/(const FT2 &s, const std::vector<FT1> &A) {
        std::vector<FT1> tmp = A;
        for (std::size_t i = 0; i < A.size(); ++i)
            tmp[i] = s / tmp[i];

        return tmp;
    }

    /// Multiply two vectors elementwise.
    /// \param v1 First vector.
    /// \param v2 Second vector of equal size.
    /// \return Elementwise product.
    template<typename FT>
    inline std::vector<FT> elem_mult(const std::vector<FT> &v1, const std::vector<FT> &v2) {
        assert(v1.size() == v2.size());
        std::vector<FT> result = v1;
        for (std::size_t i = 0; i < v2.size(); ++i) {
            result[i] *= v2[i];
        }

        return result;
    }

    /// Multiply a vector elementwise in place.
    /// \param v1 Vector to update.
    /// \param v2 Elementwise factors of equal size.
    /// \return Reference to the updated vector.
    template<typename FT>
    inline std::vector<FT> &elem_mult_eq(std::vector<FT> &v1, const std::vector<FT> &v2) {
        assert(v1.size() == v2.size());
        for (std::size_t i = 0; i < v2.size(); ++i) {
            v1[i] *= v2[i];
        }

        return v1;
    }


    /// Divide two vectors elementwise.
    /// \param v1 Numerator vector.
    /// \param v2 Divisor vector of equal size.
    /// \return Elementwise quotient.
    template<typename FT>
    inline std::vector<FT> elem_divd(const std::vector<FT> &v1, const std::vector<FT> &v2) {
        assert(v1.size() == v2.size());
        std::vector<FT> result = v1;
        for (std::size_t i = 0; i < v2.size(); ++i) {
            result[i] /= v2[i];
        }

        return result;
    }

    /// Divide a vector elementwise in place.
    /// \param v1 Vector to update.
    /// \param v2 Elementwise divisors of equal size.
    /// \return Reference to the updated vector.
    template<typename FT>
    inline std::vector<FT> &elem_divd_eq(std::vector<FT> &v1, const std::vector<FT> &v2) {
        assert(v1.size() == v2.size());
        for (std::size_t i = 0; i < v2.size(); ++i) {
            v1[i] /= v2[i];
        }

        return v1;
    }


    /// Compute the inner product of two vectors.
    /// \param v1 First vector.
    /// \param v2 Second vector of equal size.
    /// \return Sum of corresponding element products.
    template<typename FT>
    FT operator*(const std::vector<FT> &v1, const std::vector<FT> &v2) {
        assert(v1.size() == v2.size());

        FT sum = 0;
        typename std::vector<FT>::const_iterator itr1 = v1.begin();
        typename std::vector<FT>::const_iterator itr2 = v2.begin();

        while (itr1 != v1.end())
            sum += (*itr1++) * (*itr2++);

        return sum;
    }

    /// Compute the inner product of two vectors.
    /// \param v1 First vector.
    /// \param v2 Second vector of equal size.
    /// \return Sum of corresponding element products.
    template<typename FT>
    FT dot(const std::vector<FT> &v1, const std::vector<FT> &v2) {
        assert(v1.size() == v2.size());

        FT sum = 0;
        typename std::vector<FT>::const_iterator itr1 = v1.begin();
        typename std::vector<FT>::const_iterator itr2 = v2.begin();

        while (itr1 != v1.end())
            sum += (*itr1++) * (*itr2++);

        return sum;
    }


    /// Sum all elements of a vector.
    /// \param v Input vector.
    /// \return Sum of its elements.
    template<typename FT>
    FT sum(const std::vector<FT> &v) {
        FT sum = 0;
        typename std::vector<FT>::const_iterator itr = v.begin();

        while (itr != v.end())
            sum += *itr++;

        return sum;
    }

    /// Find the minimum element of a vector.
    /// \param v Nonempty input vector.
    /// \return Minimum element.
    template<typename FT>
    FT min(const std::vector<FT> &v) {
        FT m = v[0];
        for (std::size_t i = 1; i < v.size(); ++i)
            if (m > v[i])
                m = v[i];

        return m;
    }

    /// Find the maximum element of a vector.
    /// \param v Nonempty input vector.
    /// \return Maximum element.
    template<typename FT>
    FT max(const std::vector<FT> &v) {
        FT M = v[0];
        for (std::size_t i = 1; i < v.size(); ++i)
            if (M < v[i])
                M = v[i];

        return M;
    }

    /// Compute the arithmetic mean of a vector.
    /// \param v Nonempty input vector.
    /// \return Sum of elements divided by the vector size.
    template<typename FT>
    inline FT mean(const std::vector<FT> &v) {
        return sum(v) / FT(v.size());
    }

    /// Generate a vector with elements randomly initialized in the range [0, 1].
    template<typename FT>
    inline std::vector<FT> random(int size) {
        std::vector<FT> V(size);
        for (auto& v : V)
            v = random_float();
        return V;
    }

    /// Compute the Euclidean norm of a real-valued vector.
    /// \param v Input vector.
    /// \return Square root of the sum of squared elements.
    template<typename FT>
    inline FT norm(const std::vector<FT> &v) {
        FT sum = 0;
        typename std::vector<FT>::const_iterator itr = v.begin();

        while (itr != v.end()) {
            sum += (*itr) * (*itr);
            itr++;
        }

        return FT(std::sqrt(sum));
    }

    /// Compute the Euclidean norm of a complex-valued vector.
    /// \param v Input vector.
    /// \return Square root of the sum of squared magnitudes.
    template<typename FT>
    FT norm(const std::vector<std::complex<FT> > &v) {
        FT sum = 0;
        typename std::vector<std::complex<FT> >::const_iterator itr = v.begin();

        while (itr != v.end()) {
            sum += std::norm(*itr++);
        }

        return FT(std::sqrt(sum));
    }

    /// Swap corresponding elements of equally sized vectors.
    /// \param lhs First vector.
    /// \param rhs Second vector of the same size.
    template<typename FT>
    void swap(std::vector<FT> &lhs, std::vector<FT> &rhs) {
        typename std::vector<FT>::iterator itrL = lhs.begin(),
                itrR = rhs.begin();

        while (itrL != lhs.end())
            std::swap(*itrL++, *itrR++);
    }

    /// Generate n linearly spaced values from a through b, including endpoints when n > 1.
    /// \param a First endpoint.
    /// \param b Last endpoint.
    /// \param n Number of values; values below one produce an empty vector.
    /// \return Generated sequence (for n == 1, contains a).
    template<typename FT>
    std::vector<FT> linspace(FT a, FT b, int n) {
        if (n < 1)
            return std::vector<FT>();
        else if (n == 1)
            return std::vector<FT>(1, a);
        else {
            FT dx = (b - a) / (n - 1);

            std::vector<FT> tmp(n);
            for (int i = 0; i < n; ++i)
                tmp[i] = a + i * dx;

            return tmp;
        }
    }

    /// Extract the magnitude of every complex vector element.
    /// \param v Input complex vector.
    /// \return Real-valued vector of magnitudes.
    template<typename FT>
    std::vector<FT> abs(const std::vector<std::complex<FT> > &v) {
        std::vector<FT> tmp(v.size());
        typename std::vector<FT>::iterator itrL = tmp.begin();
        typename std::vector<std::complex<FT> >::const_iterator itrR = v.begin();

        while (itrL != tmp.end())
            *itrL++ = std::abs(*itrR++);

        return tmp;
    }

    /// Extract the phase angle of every complex vector element, in radians.
    /// \param v Input complex vector.
    /// \return Real-valued vector of phase angles.
    template<typename FT>
    std::vector<FT> arg(const std::vector<std::complex<FT> > &v) {
        std::vector<FT> tmp(v.size());
        typename std::vector<FT>::iterator itrL = tmp.begin();
        typename std::vector<std::complex<FT> >::const_iterator itrR = v.begin();

        while (itrL != tmp.end())
            *itrL++ = std::arg(*itrR++);

        return tmp;
    }

    /// Extract the real part of every complex vector element.
    /// \param v Input complex vector.
    /// \return Real-valued vector.
    template<typename FT>
    std::vector<FT> real(const std::vector<std::complex<FT> > &v) {
        std::vector<FT> tmp(v.size());
        typename std::vector<FT>::iterator itrL = tmp.begin();
        typename std::vector<std::complex<FT> >::const_iterator itrR = v.begin();

        while (itrL != tmp.end())
            *itrL++ = (*itrR++).real();

        return tmp;
    }

    /// Extract the imaginary part of every complex vector element.
    /// \param v Input complex vector.
    /// \return Real-valued vector.
    template<typename FT>
    std::vector<FT> imag(const std::vector<std::complex<FT> > &v) {
        std::vector<FT> tmp(v.size());
        typename std::vector<FT>::iterator itrL = tmp.begin();
        typename std::vector<std::complex<FT> >::const_iterator itrR = v.begin();

        while (itrL != tmp.end())
            *itrL++ = (*itrR++).imag();

        return tmp;
    }

    /// Convert a real vector to complex values with zero imaginary parts.
    template<typename FT>
    std::vector<std::complex<FT> > complex_vector(const std::vector<FT> &rv) {
        int N = rv.size();

        std::vector<std::complex<FT> > cv(N);
        typename std::vector<std::complex<FT> >::iterator itrL = cv.begin();
        typename std::vector<FT>::const_iterator itrR = rv.begin();

        while (itrR != rv.end())
            *itrL++ = *itrR++;

        return cv;
    }

    /// Combine real and imaginary component vectors into a complex vector.
    /// \param vR Real components.
    /// \param vI Imaginary components with the same size.
    /// \return Complex-valued vector.
    template<typename FT>
    std::vector<std::complex<FT> > complex_vector(const std::vector<FT> &vR, const std::vector<FT> &vI) {
        int N = static_cast<int>(vR.size());

        assert(N == vI.size());

        std::vector<std::complex<FT> > cv(N);
        typename std::vector<std::complex<FT> >::iterator itrC = cv.begin();
        typename std::vector<FT>::const_iterator itrR = vR.begin(),
                itrI = vI.begin();

        while (itrC != cv.end())
            *itrC++ = std::complex<FT>(*itrR++, *itrI++);

        return cv;
    }

}

#endif  // EASY3D_CORE_MATRIX_H
