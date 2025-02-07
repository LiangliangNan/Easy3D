/********************************************************************
 * Copyright (C) 2015-2021 by Liangliang Nan <liangliang.nan@gmail.com>
 * Copyright (C) 2000-2005 INRIA - Project ALICE
 *
 * The code in this file is partly from OGF/Graphite (2.0 alpha-4) with
 * modifications and enhancement:
 *      https://gforge.inria.fr/forum/forum.php?forum_id=11459
 * The original code was distributed under the GNU GPL license.
 ********************************************************************/

#ifndef EASY3D_CORE_VEC_H
#define EASY3D_CORE_VEC_H

#include <cassert>
#include <iostream>
#include <cmath>
#include <cstring>
#include <limits>


namespace easy3d {

    /**
     * @brief Base class for vector types. It provides generic functionality for \p N dimensional vectors.
     * @tparam N The dimension/size of a vector.
     * @tparam T The scalar type of vector elements.
     * @class Vec easy3d/core/vec.h
     */
    template <size_t N, class T>
    class Vec {
    public:
        /// @brief The type of this class.
        /// @details This typedef represents the type of the current class.
        typedef Vec<N, T>   thisclass;

        /// @brief Default constructor. All elements will be initialized to zero.
        Vec() { for (size_t i = 0; i < N; i++) { data_[i] = T(0); } }

        /// @brief Constructs a vector from a scalar number.
        /// @param s The scalar value to initialize all elements of the vector.
        explicit Vec(const T& s) { for (size_t i = 0; i < N; i++) { data_[i] = s; } }

        /// @brief Constructs a vector from another vector of the same dimension/size.
        // This one should never be called : a template constructor cannot be a copy constructor
        template<class T2> explicit Vec(const Vec<N, T2>& rhs) {
            for (size_t i = 0; i < N; i++) {
                data_[i] = T(rhs[i]);
            }
        }

        /// @brief Constructs a vector from another vector of the same dimension/size.
        // to avoid compilation problems
        template<class T2, size_t M> explicit Vec(const Vec<M, T2>& rhs) {
            assert(M == N);
            for (size_t i = 0; i < N; i++) {
                data_[i] = T(rhs[i]);
            }
        }

        /// @brief Constructs a vector from an array of values.
        template<class T2> explicit Vec(const T2* rhs) {
            for (size_t i = 0; i < N; i++) {
                data_[i] = T(rhs[i]);
            }
        }

        /// @brief Assignment operator. It assigns the value of this vector from another vector.
        thisclass& operator=(const thisclass& rhs) {
            // Check for self-assignment
            if (this == &rhs)
                return *this; // Return *this if rhs is the same as *this
            memcpy(data_, rhs.data(), N*sizeof(T));
            return *this;
        }

        /// @brief Returns the dimension/size of this vector.
        static size_t dimension() { return (size_t)N; }
        /// @brief Returns the dimension/size of this vector.
        static size_t size() { return (size_t)N; }

        /// @brief Returns the memory address of the vector.
        T* data()             { return data_; }
        /// @brief Returns the constant memory address of the vector.
        const T* data() const { return data_; }

        /// @brief Conversion operator returning the constant memory address of the data. Very convenient to pass this
        ///     vector as a data pointer to functions.
        operator const T*() const { return data_; }
        /// @brief Conversion operator returning the constant memory address of the data. Very convenient to pass this
        ///     vector as a data pointer to functions.
        operator T*() { return data_; }

		// Liangliang: The compiler can't decide whether to use your overloaded 
		//			   operator[] or the built-in operator[] on the const T*.
		// See https://stackoverflow.com/questions/1726740/c-error-operator-2-overloads-have-similar-conversions 
        //T& operator[](size_t idx) {
        //    assert(idx < N);
        //    return data()[idx];
        //}
        //const T& operator[](size_t idx) const {
        //    assert(idx < N);
        //    return data()[idx];
        //}

        /// @brief Returns the squared length of this vector.
        T length2() const {
            T result = T(0);
            for (size_t i = 0; i < N; i++) {
                result += data_[i] * data_[i];
            }
            return result;
        }

        /// @brief Returns the length of this vector.
        T length() const {
            return std::sqrt(length2());
        }

        /// @brief Returns the norm (i.e., length/magnitude) of this vector.
        T norm() const {
            return length();
        }

        /// @brief Returns the squared Euclidean distance to another vector.
        T distance2(const thisclass &rhs) const {
            T result = T(0);
            for (size_t i = 0; i < N; i++) {
                T val = rhs.data_[i] - data_[i];
                result += val*val;
            }
            return result;
        }

        /// @brief Normalizes this vector.
        thisclass& normalize() {
            T s = length();
            s = (s > std::numeric_limits<T>::min()) ? T(1.0) / s : T(0.0);
            *this *= s;
            return *this;
        }

        /// @brief Compound addition with another vector.
        thisclass& operator+=(const thisclass& v) {
            for (size_t i = 0; i < N; i++) {
                data_[i] += v.data_[i];
            }
            return *this;
        }

        /// @brief Compound subtraction with another vector.
        thisclass& operator-=(const thisclass& v) {
            for (size_t i = 0; i < N; i++) {
                data_[i] -= v.data_[i];
            }
            return *this;
        }

        /// @brief Compound component-wise multiplication with another vector.
        thisclass& operator*=(const thisclass& v) {
            for (size_t i = 0; i < N; i++) {
                data_[i] *= v.data_[i];
            }
            return *this;
        }

        /// @brief Compound component-wise division with another vector.
        thisclass& operator/=(const thisclass& v) {
            for (size_t i = 0; i < N; i++) {
                data_[i] /= v.data_[i];
            }
            return *this;
        }

        /// @brief Compound vector-scalar multiplication.
        template <class T2> thisclass& operator*=(T2 s) {
            for (size_t i = 0; i < N; i++) {
                data_[i] *= T(s);
            }
            return *this;
        }

        /// @brief Compound vector-scalar division.
        template <class T2> thisclass& operator/=(T2 s) {
            for (size_t i = 0; i < N; i++) {
                data_[i] /= T(s);
            }
            return *this;
        }

        /// @brief Addition with another vector.
        thisclass operator+ (const thisclass& v) const {
            thisclass result(*this);
            for (size_t i = 0; i < N; i++) {
                result.data_[i] += v.data_[i];
            }
            return result;
        }

        /// @brief Subtraction with another vector.
        thisclass operator- (const thisclass& v) const {
            thisclass result(*this);
            for (size_t i = 0; i < N; i++) {
                result.data_[i] -= v.data_[i];
            }
            return result;
        }

        /// @brief Vector-scalar multiplication.
        template <class T2> thisclass operator* (T2 s) const {
            thisclass result(*this);
            for (size_t i = 0; i < N; i++) {
                result.data_[i] *= T(s);
            }
            return result;
        }

        /// @brief Vector-scalar division.
        template <class T2> thisclass operator/ (T2 s) const {
            thisclass result(*this);
            for (size_t i = 0; i < N; i++) {
                result.data_[i] /= T(s);
            }
            return result;
        }

        /// @brief Negates this vector (i.e., adds a \c minus sign).
        thisclass operator- () const {
            thisclass result;
            for (size_t i = 0; i < N; i++) {
                result.data_[i] = -data_[i];
            }
            return result;
        }


    private:
        T data_[N];
    };


    /// Computes the dot product of two vectors
    template <size_t N, class T> T dot(const Vec<N, T>& v1, const Vec<N, T>& v2) {
        T result = 0;
        for (size_t i = 0; i < N; i++) {
            result += v1[i] * v2[i];
        }
        return result;
    }

    /// Computes the 'negative' vector
    template <size_t N, class T> Vec<N, T> operator-(const Vec<N, T>& v1) {
        Vec<N, T> result;
        for (size_t i = 0; i < N; i++) {
            result[i] = -v1[i];
        }
        return result;
    }

    /// Computes the scalar-vector product
    template <class T2, size_t N, class T> Vec<N, T> operator*(T2 s, const Vec<N, T>& v) {
        Vec<N, T> result;
        for (size_t i = 0; i < N; i++) {
            result[i] = T(s)*v[i];
        }
        return result;
    }

    /// Computes the addition of two vectors
    template <size_t N, class T> Vec<N, T> operator+(const Vec<N, T>& v1, const Vec<N, T>& v2) {
        Vec<N, T> result;
        for (size_t i = 0; i < N; i++) {
            result[i] = v1[i] + v2[i];
        }
        return result;
    }

    /// Computes the subtraction of two vectors
    template <size_t N, class T> Vec<N, T> operator-(const Vec<N, T>& v1, const Vec<N, T>& v2) {
        Vec<N, T> result;
        for (size_t i = 0; i < N; i++) {
            result[i] = v1[i] - v2[i];
        }
        return result;
    }

    // Global functions for vectors (compatible with GLSL)

    /// Computes the length/magnitude of a vector
    template <size_t N, class T> T length(const Vec<N, T>& v) { return v.length(); }
    /// Computes the length/magnitude of a vector
    template <size_t N, class T> T norm(const Vec<N, T>& v) { return v.length(); }
    /// Computes the squared length/magnitude of a vector
    template <size_t N, class T> T length2(const Vec<N, T>& v) { return v.length2(); }
    /// Computes the distance between two vectors/points
    template <size_t N, class T> T distance(const Vec<N, T>& v1, const Vec<N, T>& v2)	{ return length(v2 - v1); }
    /// Computes the squared distance between two vectors/points
    template <size_t N, class T> T distance2(const Vec<N, T>& v1, const Vec<N, T>& v2)	{ return v2.distance2(v1); }
    /// Computes and returns the normalized vector (Note: the input vector is not modified).
    template <size_t N, class T> Vec<N, T> normalize(const Vec<N, T>& v) {
        T s = v.length();
        s = (s > std::numeric_limits<T>::min()) ? T(1.0) / s : T(0.0);
        return v * s;
    }

    /// linear interpolation between two vectors (x and y).
    /// The return value is computed as (1 − w) * v1 + w * v2.
    template <size_t N, class T> Vec<N, T> mix(const Vec<N, T>& v1, const Vec<N, T>& v2, T w) {
        return (T(1) - w) * v1 + w * v2;
    }

    //-------------------- vec2 -------------------------------------------------------------------

    /**
     * @brief Specialization of Vec for 2D vectors (used for representing 2D points or vectors).
     * @tparam T The scalar type of vector elements.
     * @class Vec<2, T> easy3d/core/vec.h
     */
    template <class T>
    class Vec<2, T> {
    public:
        /// @brief The type of this class.
        /// @details This typedef represents the type of the current class.
        typedef Vec<2, T>   thisclass;

        /// @brief Default constructor. Initializes x and y to zero.
        Vec() : x(0), y(0) { }

        /// @brief Constructs a 2D vector with the given x and y values.
        /// @param x_in The x-coordinate.
        /// @param y_in The y-coordinate.
        Vec(T x_in, T y_in) : x(x_in), y(y_in) { }

        /// @brief Constructs a 2D vector from a 3D vector (drops the z-coordinate).
        /// @param v The 3D vector to promote from.
        explicit Vec(const Vec<3, T>& v) : x(v.x), y(v.y) { }

        /// @brief Constructs a 2D vector with all elements initialized to a scalar value.
        /// @param s The scalar value to initialize all elements of the vector.
        explicit Vec(const T& s) : x(s), y(s) { }

        /// @brief Constructs a 2D vector from another 2D vector of a different scalar type.
        /// @tparam T2 The scalar type of the source vector.
        /// @param v The source vector.
        template<class T2> explicit Vec(const Vec<2, T2> & v)
            : x(v.x), y(v.y) {}

        /// @brief Constructs a 2D vector from an array of values.
        /// @tparam T2 The scalar type of the source array.
        /// @param v The source array.
        template<class T2> explicit Vec(const T2* v)
            : x(T(v[0])), y(T(v[1])) {}

        /// @brief Returns the squared length of this vector.
        /// @return The squared length of the vector.
        T length2() const { return x*x + y*y; }

        /// @brief Returns the length of this vector.
        /// @return The length of the vector.
        T length() const { return std::sqrt(x*x + y*y); }

        /// @brief Returns the norm (i.e., length/magnitude) of this vector.
        /// @return The norm of the vector.
        T norm() const {	return length(); }

        /// @brief Returns the squared Euclidean distance to another vector.
        /// @param rhs The other vector.
        /// @return The squared distance between this vector and `rhs`.
        T distance2(const thisclass& rhs) const {
            T dx = rhs.x - x;
            T dy = rhs.y - y;
            return dx*dx + dy*dy;
        }

        /// @brief Normalizes this vector.
        /// @return A reference to this vector after normalization.
        thisclass& normalize() {
            T s = length();
            s = (s > std::numeric_limits<T>::min()) ? T(1.0) / s : T(0.0);
            *this *= s;
            return *this;
        }

        /// @brief Compound addition with another vector.
        /// @param v The vector to add.
        /// @return A reference to this vector after addition.
        thisclass& operator+=(const thisclass& v) { x += v.x; y += v.y; return *this; }
        /// @brief Compound subtraction with another vector.
        /// @param v The vector to subtract.
        /// @return A reference to this vector after subtraction.
        thisclass& operator-=(const thisclass& v) { x -= v.x; y -= v.y; return *this; }
        /// @brief Compound component-wise multiplication with another vector.
        /// @param v The vector to multiply.
        /// @return A reference to this vector after multiplication.
        thisclass& operator*=(const thisclass& v) { x *= v.x; y *= v.y; return *this; }
        /// @brief Compound component-wise division with another vector.
        /// @param v The vector to divide.
        /// @return A reference to this vector after division.
        thisclass& operator/=(const thisclass& v) { x /= v.x; y /= v.y; return *this; }
        /// @brief Compound vector-scalar multiplication.
        /// @tparam T2 The scalar type.
        /// @param s The scalar to multiply.
        /// @return A reference to this vector after multiplication.
        template <class T2> thisclass& operator*=(T2 s) { x *= T(s); y *= T(s); return *this; }
        /// @brief Compound vector-scalar division.
        /// @tparam T2 The scalar type.
        /// @param s The scalar to divide.
        /// @return A reference to this vector after division.
        template <class T2> thisclass& operator/=(T2 s) { x /= T(s); y /= T(s); return *this; }

        /// @brief Addition with another vector.
        /// @param v The vector to add.
        /// @return The resulting vector after addition.
        thisclass operator+ (const thisclass& v) const { return thisclass(x + v.x, y + v.y); }
        /// @brief Subtraction with another vector.
        /// @param v The vector to subtract.
        /// @return The resulting vector after subtraction.
        thisclass operator- (const thisclass& v) const { return thisclass(x - v.x, y - v.y); }

        /// @brief Vector-scalar multiplication.
        /// @tparam T2 The scalar type.
        /// @param s The scalar to multiply.
        /// @return The resulting vector after multiplication.
        template <class T2> thisclass operator* (T2 s) const { return thisclass(x*T(s), y*T(s)); }
        /// @brief Vector-scalar division.
        /// @tparam T2 The scalar type.
        /// @param s The scalar to divide.
        /// @return The resulting vector after division.
        template <class T2> thisclass operator/ (T2 s) const { return thisclass(x / T(s), y / T(s)); }

        /// @brief Negates this vector (i.e., adds a minus sign).
        /// @return The negated vector.
        thisclass operator- () const { return thisclass(-x, -y); }

        /// @brief Returns the dimension/size of this vector.
        /// @return The dimension of the vector (always 2).
        static size_t dimension() { return (size_t)2; }
        /// @brief Returns the dimension/size of this vector.
        /// @return The size of the vector (always 2).
        static size_t size() { return (size_t)2; }

        /// @brief Returns the memory address of the vector.
        /// @return A pointer to the underlying data array.
        T* data() { return _array; }
        /// @brief Returns the constant memory address of the vector.
        /// @return A const pointer to the underlying data array.
        const T* data() const { return _array; }

        /// @brief Conversion operator returning the constant memory address of the data. Very convenient to pass this
        ///     vector as a data pointer to functions.
        /// @return A const pointer to the underlying data array.
        operator const T*() const { return _array; }
        /// @brief Conversion operator returning the memory address of the data. Very convenient to pass this vector as
        ///     a data pointer to functions.
        /// @return A pointer to the underlying data array.
        operator T*() { return _array; }

		// Liangliang: The compiler can't decide whether to use your overloaded 
		//			   operator[] or the built-in operator[] on the const T*.
		// See https://stackoverflow.com/questions/1726740/c-error-operator-2-overloads-have-similar-conversions 
        //T& operator[](size_t idx) {
        //    assert(idx < 2);
        //    return _array[idx];
        //}
        //const T& operator[](size_t idx) const {
        //    assert(idx < 2);
        //    return _array[idx];
        //}

        // Data intentionally left public to allow direct access (e.g., vec.x).
        union {
            T _array[2];		// Array access
            struct { T x, y; };	// Standard names for components
            struct { T u, v; };	// Alternative names for components (e.g., for texture coordinates).
        };
    };

    /// Compute the dot product of two 2D vectors.
    template <class T> T dot(const Vec<2, T>& v1, const Vec<2, T>& v2) { return v1.x*v2.x + v1.y*v2.y; }

    /// Compute the determinant of the 2x2 matrix formed by the two 2D vectors as the two rows.
    template <class T> T det(const Vec<2, T>& v1, const Vec<2, T>& v2) {
        return v1.x*v2.y - v1.y*v2.x;
    }

    /// The negative vector.
    template <class T> Vec<2, T> operator-(const Vec<2, T>& v1) {
        return Vec<2, T>(-v1.x, -v1.y);
    }

    /// Scalar-vector multiplication.
    template <class T2, class T> Vec<2, T> operator*(T2 s, const Vec<2, T>& v) {
        return Vec<2, T>(T(s)*v.x, T(s)*v.y);
    }

    /// Vector-vector addition.
    template <class T> Vec<2, T> operator+(const Vec<2, T>& v1, const Vec<2, T>& v2) {
        return Vec<2, T>(v1.x + v2.x, v1.y + v2.y);
    }

    /// Vector-vector subtraction.
    template <class T> Vec<2, T> operator-(const Vec<2, T>& v1, const Vec<2, T>& v2) {
        return Vec<2, T>(v1.x - v2.x, v1.y - v2.y);
    }


    //---------------- vec3 ------------------------------------------------------------------------

    /**
     * @brief Specialization of Vec for 3D vectors (used for representing 3D points or vectors, or RGB colors).
     * @tparam T The scalar type of vector elements.
     * @class Vec<3, T> easy3d/core/vec.h
     */
    template <class T>
    class Vec<3, T> {
    public:
        /// @brief The type of this class.
        /// @details This typedef represents the type of the current class.
        typedef Vec<3, T>   thisclass;

        /// @brief Default constructor. Initializes x, y, and z to zero.
        Vec() : x(0), y(0), z(0) {}

        /// @brief Constructs a 3D vector from a 2D vector and a scalar (useful for promoting to homogeneous coordinates).
        /// @param v The 2D vector.
        /// @param s The scalar value for the z-coordinate (default is 1).
        explicit Vec(const Vec<2, T>& v, const T& s = 1) : x(v.x), y(v.y), z(s) {}

        /// @brief Constructs a 3D vector from a 4D vector (drops the w-coordinate).
        /// @param v The 4D vector.
        explicit Vec(const Vec<4, T>& v) : x(v.x), y(v.y), z(v.z) {}

        /// @brief Constructs a 3D vector with the given x, y, and z values.
        /// @param x_in The x-coordinate.
        /// @param y_in The y-coordinate.
        /// @param z_in The z-coordinate.
        Vec(T x_in, T y_in, T z_in) : x(x_in), y(y_in), z(z_in) {}

        /// @brief Constructs a 3D vector with all elements initialized to a scalar value.
        /// @param s The scalar value to initialize all elements of the vector.
        explicit Vec(const T& s) : x(s), y(s), z(s) {  }

        /// @brief Constructs a 3D vector from another 3D vector of a different scalar type.
        /// @tparam T2 The scalar type of the source vector.
        /// @param v The source vector.
        template<class T2> explicit Vec(const Vec<3, T2> & v) : x(v.x), y(v.y), z(v.z) {}

        /// @brief Constructs a 3D vector from an array of values.
        /// @tparam T2 The scalar type of the source array.
        /// @param v The source array.
        template<class T2> explicit Vec(const T2* v)
            : x(T(v[0])), y(T(v[1])), z(T(v[2])) {}

        /// @brief Returns the squared length of this vector.
        /// @return The squared length of the vector.
        T length2() const { return x*x + y*y + z*z; }
        /// @brief Returns the length of this vector.
        /// @return The length of the vector.
        T length() const { return std::sqrt(x*x + y*y + z*z); }
        /// @brief Returns the norm (i.e., length/magnitude) of this vector.
        /// @return The norm of the vector.
        T norm() const { return length(); }
        /// @brief Returns the squared Euclidean distance to another vector.
        /// @param rhs The other vector.
        /// @return The squared distance between this vector and `rhs`.
        T distance2(const thisclass& rhs) const {
            T dx = rhs.x - x;
            T dy = rhs.y - y;
            T dz = rhs.z - z;
            return dx*dx + dy*dy + dz*dz;
        }

        /// @brief Normalizes this vector.
        /// @return A reference to this vector after normalization.
        thisclass& normalize() {
            T s = length();
            s = (s > std::numeric_limits<T>::min()) ? T(1.0) / s : T(0.0);
            *this *= s;
            return *this;
        }

        /// @brief Compound addition with another vector.
        /// @param v The vector to add.
        /// @return A reference to this vector after addition.
        thisclass& operator+=(const thisclass& v) { x += v.x; y += v.y; z += v.z; return *this; }
        /// @brief Compound subtraction with another vector.
        /// @param v The vector to subtract.
        /// @return A reference to this vector after subtraction.
        thisclass& operator-=(const thisclass& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
        /// @brief Compound component-wise multiplication with another vector.
        /// @param v The vector to multiply.
        /// @return A reference to this vector after multiplication.
        thisclass& operator*=(const thisclass& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
        /// @brief Compound component-wise division with another vector.
        /// @param v The vector to divide.
        /// @return A reference to this vector after division.
        thisclass& operator/=(const thisclass& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
        /// @brief Compound vector-scalar multiplication.
        /// @tparam T2 The scalar type.
        /// @param s The scalar to multiply.
        /// @return A reference to this vector after multiplication.
        template <class T2> thisclass& operator*=(T2 s) { x *= T(s); y *= T(s); z *= T(s); return *this; }
        /// @brief Compound vector-scalar division.
        /// @tparam T2 The scalar type.
        /// @param s The scalar to divide.
        /// @return A reference to this vector after division.
        template <class T2> thisclass& operator/=(T2 s) { x /= T(s); y /= T(s); z /= T(s); return *this; }

        /// @brief Addition with another vector.
        /// @param v The vector to add.
        /// @return The resulting vector after addition.
        thisclass operator+ (const thisclass& v) const { return thisclass(x + v.x, y + v.y, z + v.z); }
        /// @brief Subtraction with another vector.
        /// @param v The vector to subtract.
        /// @return The resulting vector after subtraction.
        thisclass operator- (const thisclass& v) const { return thisclass(x - v.x, y - v.y, z - v.z); }

        /// @brief Vector-scalar multiplication.
        /// @tparam T2 The scalar type.
        /// @param s The scalar to multiply.
        /// @return The resulting vector after multiplication.
        template <class T2> thisclass operator* (T2 s) const { return thisclass(x*T(s), y*T(s), z*T(s)); }
        /// @brief Vector-scalar division.
        /// @tparam T2 The scalar type.
        /// @param s The scalar to divide.
        /// @return The resulting vector after division.
        template <class T2> thisclass operator/ (T2 s) const { return thisclass(x / T(s), y / T(s), z / T(s)); }

        /// @brief Negates this vector (i.e., adds a minus sign).
        /// @return The negated vector.
        thisclass operator- () const { return thisclass(-x, -y, -z); }

        /// @brief Returns the dimension/size of this vector.
        /// @return The dimension of the vector (always 3).
        static size_t dimension() { return (size_t)3; }
        /// @brief Returns the dimension/size of this vector.
        /// @return The size of the vector (always 3).
        static size_t size() { return (size_t)3; }

        /// @brief Returns the memory address of the vector.
        /// @return A pointer to the underlying data array.
        T* data() { return _array; }
        /// @brief Returns the constant memory address of the vector.
        /// @return A const pointer to the underlying data array.
        const T* data() const { return _array; }

        /// @brief Returns the (x, y) components of this vector as a 2D vector.
        /// @return A 2D vector containing the x and y components.
        Vec<2, T> xy() const { return  Vec<2, T>(x, y); }

        /// @brief Conversion operator returning the constant memory address of the data. Very convenient to pass this
        ///     vector as a data pointer to functions.
        /// @return A const pointer to the underlying data array.
        operator const T*() const { return _array; }
        /// @brief Conversion operator returning the constant memory address of the data. Very convenient to pass this
        ///     vector as a data pointer to functions.
        /// @return A pointer to the underlying data array.
        operator T*() { return _array; }

		// Liangliang: The compiler can't decide whether to use your overloaded 
		//			   operator[] or the built-in operator[] on the const T*.
		// See https://stackoverflow.com/questions/1726740/c-error-operator-2-overloads-have-similar-conversions 
        //T& operator[](size_t idx) {
        //    assert(idx < 3);
        //    return _array[idx];
        //}
        //const T& operator[](size_t idx) const {
        //    assert(idx < 3);
        //    return _array[idx];
        //}

        // Data intentionally left public to allow direct access (e.g., vec.x).
        union {
            T _array[3];			// Array access
            struct { T x, y, z; };	// Standard names for components
            struct { T r, g, b; };	// Alternative names for components (e.g., for RGB colors).
        };
    };

    /// Compute the dot product of two 3D vectors.
    template <class T> T dot(const Vec<3, T>& v1, const Vec<3, T>& v2) {
        return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
    }

    /// Compute the cross product of two 3D vectors.
    template <class T> Vec<3, T> cross(const Vec<3, T>& v1, const Vec<3, T>& v2) {
        return Vec<3, T>(
            v1.y*v2.z - v1.z*v2.y,
            v1.z*v2.x - v1.x*v2.z,
            v1.x*v2.y - v1.y*v2.x
            );
    }

    /// The negative vector.
    template <class T> Vec<3, T> operator-(const Vec<3, T>& v1) { return Vec<3, T>(-v1.x, -v1.y, -v1.z); }

    /// Scalar-vector multiplication.
    template <class T2, class T> Vec<3, T> operator*(T2 s, const Vec<3, T>& v) {
        return Vec<3, T>(T(s)*v.x, T(s)*v.y, T(s)*v.z);
    }

    /// Vector-vector addition.
    template <class T> Vec<3, T> operator+(const Vec<3, T>& v1, const Vec<3, T>& v2) {
        return Vec<3, T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
    }

    /// Vector-vector subtraction.
    template <class T> Vec<3, T> operator-(const Vec<3, T>& v1, const Vec<3, T>& v2) {
        return Vec<3, T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
    }

    /// Compute a vector that is orthogonal to the given vector.
    template <class T> Vec<3, T> orthogonal(const Vec<3, T>& v) {
        T absx = std::fabs(v.x);
        T absy = std::fabs(v.y);
        T absz = std::fabs(v.z);
        // Find the smallest component. Keep equal case for null values.
        if ((absy >= absx) && (absz >= absx))
            return Vec<3, T>(0.0f, -v.z, v.y);
        else
            if ((absx >= absy) && (absz >= absy))
                return Vec<3, T>(-v.z, 0.0f, v.x);
            else
                return Vec<3, T>(-v.y, v.x, 0.0f);
    }

    // ----------------- vec4 ----------------------------------------------------------------------------------

    /**
     * @brief Specialization of Vec for 4D vectors (used for representing 4D points or vectors in homogeneous coordinates, or RGBA colors).
     * @tparam T The scalar type of vector elements.
     * @class Vec<4, T> easy3d/core/vec.h
     */
    template <class T>
    class Vec<4, T> {
    public:
        /// @brief The type of this class.
        /// @details This typedef represents the type of the current class.
        typedef Vec<4, T>   thisclass;

        /// @brief Default constructor. Initializes x, y, z, and w to zero.
        Vec() : x(0), y(0), z(0), w(0) {}

        /// @brief Constructs a 4D vector from a 3D vector and a scalar (useful for promoting to homogeneous coordinates).
        /// @param v The 3D vector.
        /// @param s The scalar value for the w-coordinate (default is 1).
        explicit Vec(const Vec<3, T>& v, const T& s = 1) : x(v.x), y(v.y), z(v.z), w(s) {}

        /// @brief Constructs a 4D vector with the given x, y, z, and w values.
        /// @param x_in The x-coordinate.
        /// @param y_in The y-coordinate.
        /// @param z_in The z-coordinate.
        /// @param w_in The w-coordinate.
        Vec(T x_in, T y_in, T z_in, T w_in) : x(x_in), y(y_in), z(z_in), w(w_in) {}

        /// @brief Constructs a 4D vector with all elements initialized to a scalar value.
        /// @param s The scalar value to initialize all elements of the vector.
        explicit Vec(const T& s) : x(s), y(s), z(s), w(s) {  }

        /// @brief Constructs a 4D vector from another 4D vector of a different scalar type.
        /// @tparam T2 The scalar type of the source vector.
        /// @param v The source vector.
        template<class T2> explicit Vec(const Vec<4, T2> & v)
            : x(v.x), y(v.y), z(v.z), w(v.w) {}

        /// @brief Constructs a 4D vector from an array of values.
        /// @tparam T2 The scalar type of the source array.
        /// @param v The source array.
        template<class T2> explicit Vec(const T2* v)
            : x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}

        /// @brief Returns the squared length of this vector.
        /// @return The squared length of the vector.
        T length2() const { return x*x + y*y + z*z + w*w; }
        /// @brief Returns the length of this vector.
        /// @return The length of the vector.
        T length() const { return std::sqrt(x*x + y*y + z*z + w*w); }
        /// @brief Returns the norm (i.e., length/magnitude) of this vector.
        /// @return The norm of the vector.
        T norm() const { return length(); }
        /// @brief Returns the squared Euclidean distance to another vector.
        /// @param rhs The other vector.
        /// @return The squared distance between this vector and `rhs`.
        T distance2(const thisclass& rhs) const {
            T dx = rhs.x - x;
            T dy = rhs.y - y;
            T dz = rhs.z - z;
            T dw = rhs.w - w;
            return dx*dx + dy*dy + dz*dz + dw*dw;
        }

        /// @brief Normalizes this vector.
        /// @return A reference to this vector after normalization.
        thisclass& normalize() {
            T s = length();
            s = (s > std::numeric_limits<T>::min()) ? T(1.0) / s : T(0.0);
            *this *= s;
            return *this;
        }

        /// @brief Returns the dimension/size of this vector.
        /// @return The dimension of the vector (always 4).
        static size_t dimension() { return (size_t)4; }
        /// @brief Returns the dimension/size of this vector.
        /// @return The size of the vector (always 4).
        static size_t size() { return (size_t)4; }

        /// @brief Compound addition with another vector.
        /// @param v The vector to add.
        /// @return A reference to this vector after addition.
        thisclass& operator+=(const thisclass& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
        /// @brief Compound subtraction with another vector.
        /// @param v The vector to subtract.
        /// @return A reference to this vector after subtraction.
        thisclass& operator-=(const thisclass& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
        /// @brief Compound component-wise multiplication with another vector.
        /// @param v The vector to multiply.
        /// @return A reference to this vector after multiplication.
        thisclass& operator*=(const thisclass& v) { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
        /// @brief Compound component-wise division with another vector.
        /// @param v The vector to divide.
        /// @return A reference to this vector after division.
        thisclass& operator/=(const thisclass& v) { x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }

        /// @brief Compound vector-scalar multiplication.
        /// @tparam T2 The scalar type.
        /// @param s The scalar to multiply.
        /// @return A reference to this vector after multiplication.
        template <class T2> thisclass& operator*=(T2 s) {
            x *= T(s); y *= T(s); z *= T(s); w *= T(s); return *this;
        }
        /// @brief Compound vector-scalar division.
        /// @tparam T2 The scalar type.
        /// @param s The scalar to divide.
        /// @return A reference to this vector after division.
        template <class T2> thisclass& operator/=(T2 s) {
            x /= T(s); y /= T(s); z /= T(s); w /= T(s); return *this;
        }

        /// @brief Addition with another vector.
        /// @param v The vector to add.
        /// @return The resulting vector after addition.
        thisclass operator+ (const thisclass& v) const { return thisclass(x + v.x, y + v.y, z + v.z, w + v.w); }
        /// @brief Subtraction with another vector.
        /// @param v The vector to subtract.
        /// @return The resulting vector after subtraction.
        thisclass operator- (const thisclass& v) const { return thisclass(x - v.x, y - v.y, z - v.z, w - v.w); }

        /// @brief Vector-scalar multiplication.
        /// @tparam T2 The scalar type.
        /// @param s The scalar to multiply.
        /// @return The resulting vector after multiplication.
        template <class T2> thisclass operator* (T2 s) const { return thisclass(x*T(s), y*T(s), z*T(s), w*T(s)); }
        /// @brief Vector-scalar division.
        /// @tparam T2 The scalar type.
        /// @param s The scalar to divide.
        /// @return The resulting vector after division.
        template <class T2> thisclass operator/ (T2 s) const { return thisclass(x / T(s), y / T(s), z / T(s), w / T(s)); }

        /// @brief Negates this vector (i.e., adds a minus sign).
        /// @return The negated vector.
        thisclass operator- () const { return thisclass(-x, -y, -z, -w); }

        /// @brief Returns the memory address of the vector.
        /// @return A pointer to the underlying data array.
        T* data() { return _array; }
        /// @brief Returns the constant memory address of the vector.
        /// @return A const pointer to the underlying data array.
        const T* data() const { return _array; }

        /// @brief Returns the (x, y, z) components of this vector as a 3D vector.
        /// @return A 3D vector containing the x, y, and z components.
        Vec<3, T> xyz() const { return  Vec<3, T>(x, y, z); }

        /// @brief Conversion operator returning the constant memory address of the data. Very convenient to pass this
        ///     vector as a data pointer to functions.
        /// @return A const pointer to the underlying data array.
        operator const T*() const { return _array; }
        /// @brief Conversion operator returning the memory address of the data. Very convenient to pass thisvector as
        ///    a data pointer to functions.
        /// @return A pointer to the underlying data array.
        operator T*() { return _array; }

		// Liangliang: The compiler can't decide whether to use your overloaded 
		//			   operator[] or the built-in operator[] on the const T*.
		// See https://stackoverflow.com/questions/1726740/c-error-operator-2-overloads-have-similar-conversions 
        //T& operator[](size_t idx) {
        //    assert(idx < 4);
        //    return _array[idx];
        //}
        //const T& operator[](size_t idx) const {
        //    assert(idx < 4);
        //    return _array[idx];
        //}

        // Data intentionally left public to allow direct access (e.g., vec.x).
        union {
            T _array[4];				// Array access
            struct { T x, y, z, w; };	// Standard names for components
            struct { T r, g, b, a; };	// Alternative names for components (e.g., for RGBA colors).
        };
    };

    /// Compute the dot product of two 4D vectors.
    template <class T> T dot(const Vec<4, T>& v1, const Vec<4, T>& v2) {
        return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w;
    }

    /// The negative vector.
    template <class T> Vec<4, T> operator-(const Vec<4, T>& v1) { return Vec<4, T>(-v1.x, -v1.y, -v1.z, -v1.w); }

    /// Scalar-vector multiplication.
    template <class T2, class T> Vec<4, T> operator*(T2 s, const Vec<4, T>& v) {
        return Vec<4, T>(T(s)*v.x, T(s)*v.y, T(s)*v.z, T(s)*v.w);
    }

    /// Vector-vector addition.
    template <class T> Vec<4, T> operator+(const Vec<4, T>& v1, const Vec<4, T>& v2) {
        return Vec<4, T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
    }

    /// Vector-vector subtraction.
    template <class T> Vec<4, T> operator-(const Vec<4, T>& v1, const Vec<4, T>& v2) {
        return Vec<4, T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
    }



    //------------------------------- IO (input/output) ----------------------------

    /// Output stream support for Vec.
    template <size_t N, class T> std::ostream& operator<<(std::ostream& out, const Vec<N, T>& v) {
        for (size_t i = 0; i < N; i++) {
            out << v[i] << " ";
        }
        return out;
    }

    /// Input stream support for Vec.
    template <size_t N, class T> std::istream& operator>>(std::istream& in, Vec<N, T>& v) {
        for (size_t i = 0; i < N; i++) {
            in >> v[i];
        }
        return in;
    }

    /// Output stream support specialized for 2D vectors.
    template <class T> std::ostream& operator<<(std::ostream& out, const Vec<2, T>& v) {
        return out << v.x << " " << v.y;
    }

    /// Input stream support specialized for 2D vectors.
    template <class T> std::istream& operator>>(std::istream& in, Vec<2, T>& v) {
        return in >> v.x >> v.y;
    }

    /// Output stream support specialized for 3D vectors.
    template <class T> std::ostream& operator<<(std::ostream& out, const Vec<3, T>& v) {
        return out << v.x << " " << v.y << " " << v.z;
    }

    /// Input stream support specialized for 3D vectors.
    template <class T> std::istream& operator>>(std::istream& in, Vec<3, T>& v) {
        return in >> v.x >> v.y >> v.z;
    }

    /// Output stream support specialized for 4D vectors.
    template <class T> std::ostream& operator<<(std::ostream& out, const Vec<4, T>& v) {
        return out << v.x << " " << v.y << " " << v.z << " " << v.w;
    }

    /// Input stream support specialized for 4D vectors.
    template <class T> std::istream& operator>>(std::istream& in, Vec<4, T>& v) {
        return in >> v.x >> v.y >> v.z >> v.w;
    }

    //----------------------------------------------------------------------

    /// Do the coordinates of a vector have NaN?
    template <size_t N, class T>
    bool has_nan(const Vec<N, T> &v) {
        for (std::size_t i = 0; i < N; ++i) {
            if (std::isnan(v[i]) || std::isinf(v[i]))
                return true;
        }
        return false;
    }

    /// Test if two vectors are strictly identical.
    template <size_t N, class T>
    bool operator==(const Vec<N,T> &a, const Vec<N,T> &b) {
        for (size_t i = 0; i < N; i++) {
            if (a[i] != b[i]) return false;
        }
        return true;
    }

    /// Test if two vectors are strictly not identical.
    template <size_t N, class T>
    bool operator!=(const Vec<N,T> &a, const Vec<N,T> &b) {
        for (size_t i = 0; i < N; i++) {
            if (a[i] != b[i]) return true;
        }
        return false;
    }

    /// Lexicographic comparison of two vectors
    template <size_t N, class T>
    bool operator<(const Vec<N,T> &a, const Vec<N,T> &b) {
        for(unsigned int i=0; i<N; ++i){
            if(a[i]<b[i]) return true;
            if(a[i]>b[i]) return false;
        }
        return false;
    }

    /// Component-wise product of two vectors
    template <size_t N, class T>
    Vec<N, T> comp_product(const Vec<N, T> &v1, const Vec<N, T> &v2) {
        Vec<N, T> result;
        for (int i = 0; i < N; ++i)
            result[i] = v1[i] * v2[i];
        return result;
    }

    /// Component-wise minimum vector
    template <size_t N, class T>
    Vec<N, T> comp_min(const Vec<N, T> &v1, const Vec<N, T> &v2) {
        Vec<N, T> result;
        for (int i = 0; i < N; ++i)
            result[i] = std::min(v1[i], v2[i]);
        return result;
    }

    /// Component-wise maximum vector
    template <size_t N, class T>
    Vec<N, T> comp_max(const Vec<N, T> &v1, const Vec<N, T> &v2) {
        Vec<N, T> result;
        for (int i = 0; i < N; ++i)
            result[i] = std::max(v1[i], v2[i]);
        return result;
    }

    /// The minimum coordinate of elements in a vector
    template<size_t N, class T>
    T min_coord(const Vec<N, T> &a) {
        T result = a[0];
        for (unsigned int i = 1; i < N; ++i) if (a[i] < result) result = a[i];
        return result;
    }

    /// The maximum coordinate of elements in a vector
    template<size_t N, class T>
    T max_coord(const Vec<N, T> &a) {
        T result = a[0];
        for (unsigned int i = 1; i < N; ++i) if (a[i] > result) result = a[i];
        return result;
    }

    /// Component-wise clamp
    template<size_t N, class T>
    Vec<N, T> clamp(const Vec<N, T> &a, const Vec<N, T> &lower, const Vec<N, T> &upper) {
        Vec<N, T> result;
        for (unsigned int i = 0; i < N; ++i) result[i] = std::min(upper[i], std::max(a[i], lower[i]));
        return result;
    }

}

#endif  // EASY3D_CORE_VEC_H

