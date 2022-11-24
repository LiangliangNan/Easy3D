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
#include <cfloat>
#include <cstring>
#include <limits>


namespace easy3d {

#if 1

    /**
     * \brief Base class for vector types. It provides generic functionality for \p N dimensional vectors.
     * \tparam N The dimension/size of a vector.
     * \tparam T The scalar type of vector elements.
     *
     * \class Vec easy3d/core/vec.h
     */
    template <size_t N, class T>
    class Vec {
    public:
        typedef Vec<N, T>   thisclass;
        typedef T           FT;

        /// \brief Default constructor. All elements will be initialized to zero.
        Vec() { for (size_t i = 0; i < N; i++) { data_[i] = T(0); } }

        /// \brief Constructs a vector from a scalar number \p s. All elements will be initialized to this value.
        explicit Vec(const T& s) { for (size_t i = 0; i < N; i++) { data_[i] = s; } }

        /// \brief Constructs a vector from another vector of the same dimension/size.
        // This one should never be called : a template constructor cannot be a copy constructor
        template<class T2> explicit Vec(const Vec<N, T2>& rhs) {
            for (size_t i = 0; i < N; i++) {
                data_[i] = T(rhs[i]);
            }
        }

        /// \brief Constructs a vector from another vector of the same dimension/size.
        // to avoid compilation problems
        template<class T2, size_t M> explicit Vec(const Vec<M, T2>& rhs) {
            assert(M == N);
            for (size_t i = 0; i < N; i++) {
                data_[i] = T(rhs[i]);
            }
        }

        /// \brief Constructs a vector from an array of values.
        template<class T2> explicit Vec(const T2* rhs) {
            for (size_t i = 0; i < N; i++) {
                data_[i] = T(rhs[i]);
            }
        }

        /// \brief Assignment operator. It assigns the value of this vector from another vector.
        thisclass& operator=(const thisclass& rhs) {
            memcpy(data_, rhs.data(), N*sizeof(T));
            return *this;
        }

        /// \brief Returns the dimension/size of this vector.
        size_t dimension() const { return (size_t)N; }
        /// \brief Returns the dimension/size of this vector.
        size_t size() const { return dimension(); }

        /// \brief Returns the memory address of the vector.
        T* data()             { return data_; }
        /// \brief Returns the constant memory address of the vector.
        const T* data() const { return data_; }

        /// \brief Conversion operator returning the constant memory address of the data. Very convenient to pass this
        /// vector as a data pointer to functions, e.g., \code glVertex3fv(pos); \endcode.
        operator const T*() const { return data_; }
        /// \brief Conversion operator returning the memory address of the data. Very convenient to pass this vector
        /// as a data pointer to functions, e.g., \code glVertex3fv(pos); \endcode.
        operator T*() { return data_; }

		// Liangliang: The compiler can't decide whether to use your overloaded 
		//			   operator[] or the built-in operator[] on the const T*.
		// See https://stackoverflow.com/questions/1726740/c-error-operator-2-overloads-have-similar-conversions 
        //inline T& operator[](size_t idx) {
        //    assert(idx < N);
        //    return data()[idx];
        //}
		//
        //inline const T& operator[](size_t idx) const {
        //    assert(idx < N);
        //    return data()[idx];
        //}

        /// \brief Returns the squared length of this vector.
        inline T length2() const {
            T result = T(0);
            for (size_t i = 0; i < N; i++) {
                result += data_[i] * data_[i];
            }
            return result;
        }

        /// \brief Returns the length of this vector.
        inline T length() const {
            return std::sqrt(length2());
        }

        /// \brief Returns the norm (i.e., length/magnitude0 of this vector.
        inline T norm() const {
            return length();
        }

        /// \brief Returns the squared Euclidean distance to another vector.
        inline T distance2(const thisclass &rhs) const {
            T result = T(0);
            for (size_t i = 0; i < N; i++) {
                T val = rhs.data_[i] - data_[i];
                result += val*val;
            }
            return result;
        }

        /// \brief Normalizes this vector.
        inline thisclass& normalize() {
            T s = length();
            s = (s > std::numeric_limits<T>::min()) ? T(1.0) / s : T(0.0);
            *this *= s;
            return *this;
        }

        /// \brief Compound addition with another vector.
        inline thisclass& operator+=(const thisclass& v) {
            for (size_t i = 0; i < N; i++) {
                data_[i] += v.data_[i];
            }
            return *this;
        }

        /// \brief Compound subtraction with another vector.
        inline thisclass& operator-=(const thisclass& v) {
            for (size_t i = 0; i < N; i++) {
                data_[i] -= v.data_[i];
            }
            return *this;
        }

        /// \brief Compound component-wise multiplication with another vector.
        inline thisclass& operator*=(const thisclass& v) {
            for (size_t i = 0; i < N; i++) {
                data_[i] *= v.data_[i];
            }
            return *this;
        }

        /// \brief Compound component-wise division with another vector.
        inline thisclass& operator/=(const thisclass& v) {
            for (size_t i = 0; i < N; i++) {
                data_[i] /= v.data_[i];
            }
            return *this;
        }

        /// \brief Compound vector-scalar multiplication.
        template <class T2> inline thisclass& operator*=(T2 s) {
            for (size_t i = 0; i < N; i++) {
                data_[i] *= T(s);
            }
            return *this;
        }

        /// \brief Compound vector-scalar division.
        template <class T2> inline thisclass& operator/=(T2 s) {
            for (size_t i = 0; i < N; i++) {
                data_[i] /= T(s);
            }
            return *this;
        }

        /// \brief Addition with another vector.
        inline thisclass operator+ (const thisclass& v) const {
            thisclass result(*this);
            for (size_t i = 0; i < N; i++) {
                result.data_[i] += v.data_[i];
            }
            return result;
        }

        /// \brief Subtraction with another vector.
        inline thisclass operator- (const thisclass& v) const {
            thisclass result(*this);
            for (size_t i = 0; i < N; i++) {
                result.data_[i] -= v.data_[i];
            }
            return result;
        }

        /// \brief Vector-scalar multiplication.
        template <class T2> inline thisclass operator* (T2 s) const {
            thisclass result(*this);
            for (size_t i = 0; i < N; i++) {
                result.data_[i] *= T(s);
            }
            return result;
        }

        /// \brief Vector-scalar division.
        template <class T2> inline thisclass operator/ (T2 s) const {
            thisclass result(*this);
            for (size_t i = 0; i < N; i++) {
                result.data_[i] /= T(s);
            }
            return result;
        }

        /// \brief Negates this vector (i.e., adds a \c minus sign).
        inline thisclass operator- () const {
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
    template <size_t N, class T> inline T dot(const Vec<N, T>& v1, const Vec<N, T>& v2) {
        T result = 0;
        for (size_t i = 0; i < N; i++) {
            result += v1[i] * v2[i];
        }
        return result;
    }

    /// Computes the 'negative' vector
    template <size_t N, class T> inline Vec<N, T> operator-(const Vec<N, T>& v1) {
        Vec<N, T> result;
        for (size_t i = 0; i < N; i++) {
            result[i] = -v1[i];
        }
        return result;
    }

    /// Computes the scalar-vector product
    template <class T2, size_t N, class T> inline Vec<N, T> operator*(T2 s, const Vec<N, T>& v) {
        Vec<N, T> result;
        for (size_t i = 0; i < N; i++) {
            result[i] = T(s)*v[i];
        }
        return result;
    }

    /// Computes the addition of two vectors
    template <size_t N, class T> inline Vec<N, T> operator+(const Vec<N, T>& v1, const Vec<N, T>& v2) {
        Vec<N, T> result;
        for (size_t i = 0; i < N; i++) {
            result[i] = v1[i] + v2[i];
        }
        return result;
    }

    /// Computes the subtraction of two vectors
    template <size_t N, class T> inline Vec<N, T> operator-(const Vec<N, T>& v1, const Vec<N, T>& v2) {
        Vec<N, T> result;
        for (size_t i = 0; i < N; i++) {
            result[i] = v1[i] - v2[i];
        }
        return result;
    }

    // Global functions for vectors (compatible with GLSL)

    /// Computes the length/magnitude of a vector
    template <size_t N, class T> inline T length(const Vec<N, T>& v) { return v.length(); }
    /// Computes the length/magnitude of a vector
    template <size_t N, class T> inline T norm(const Vec<N, T>& v) { return v.length(); }
    /// Computes the squared length/magnitude of a vector
    template <size_t N, class T> inline T length2(const Vec<N, T>& v) { return v.length2(); }
    /// Computes the distance between two vectors/points
    template <size_t N, class T> inline T distance(const Vec<N, T>& v1, const Vec<N, T>& v2)	{ return length(v2 - v1); }
    /// Computes the squared distance between two vectors/points
    template <size_t N, class T> inline T distance2(const Vec<N, T>& v1, const Vec<N, T>& v2)	{ return v2.distance2(v1); }
    /// Computes and returns the normalized vector (Note: the input vector is not modified).
    template <size_t N, class T> inline Vec<N, T> normalize(const Vec<N, T>& v) {
        T s = v.length();
        s = (s > std::numeric_limits<T>::min()) ? T(1.0) / s : T(0.0);
        return v * s;
    }

    /// linear interpolation between between two vectors (x and y).
    /// The return value is computed as (1 − w) * v1 + w * v2.
    template <size_t N, class T> inline Vec<N, T> mix(const Vec<N, T>& v1, const Vec<N, T>& v2, T w) {
        return (T(1) - w) * v1 + w * v2;
    }

    //-------------------- vec2 -------------------------------------------------------------------

    /**
     * \brief A 2D vector (used for representing 2D points or vectors).
     * \tparam T The scalar type of vector elements.
     *
     * \class Vec<2, T> easy3d/core/vec.h
     */
    template <class T>
    class Vec<2, T> {
    public:
        typedef Vec<2, T> thisclass;
        typedef T         FT;

        Vec() : x(0), y(0) { }
        Vec(T x_in, T y_in) : x(x_in), y(y_in) { }
        explicit Vec(const Vec<3, T>& v) : x(v.x), y(v.y) { } // very useful for inverse promoting from homogeneous coordinates

        explicit Vec(const T& s) : x(s), y(s) { }

        template<class T2> explicit Vec(const Vec<2, T2> & v)
            : x(v.x), y(v.y) {}

        template<class T2> explicit Vec(const T2* v)
            : x(T(v[0])), y(T(v[1])) {}


        inline T length2() const { return x*x + y*y; }
        inline T length() const { return std::sqrt(x*x + y*y); }
        inline T norm() const {	return length(); }
        inline T distance2(const thisclass& rhs) const {
            T dx = rhs.x - x;
            T dy = rhs.y - y;
            return dx*dx + dy*dy;
        }

        inline thisclass& normalize() {
            T s = length();
            s = (s > std::numeric_limits<T>::min()) ? T(1.0) / s : T(0.0);
            *this *= s;
            return *this;
        }

        // operators
        inline thisclass& operator+=(const thisclass& v) { x += v.x; y += v.y; return *this; }
        inline thisclass& operator-=(const thisclass& v) { x -= v.x; y -= v.y; return *this; }
        inline thisclass& operator*=(const thisclass& v) { x *= v.x; y *= v.y; return *this; }
        inline thisclass& operator/=(const thisclass& v) { x /= v.x; y /= v.y; return *this; }
        template <class T2> inline thisclass& operator*=(T2 s) { x *= T(s); y *= T(s); return *this; }
        template <class T2> inline thisclass& operator/=(T2 s) { x /= T(s); y /= T(s); return *this; }

        inline thisclass operator+ (const thisclass& v) const { return thisclass(x + v.x, y + v.y); }
        inline thisclass operator- (const thisclass& v) const { return thisclass(x - v.x, y - v.y); }
        template <class T2> inline thisclass operator* (T2 s) const { return thisclass(x*T(s), y*T(s)); }
        template <class T2> inline thisclass operator/ (T2 s) const { return thisclass(x / T(s), y / T(s)); }
        inline thisclass operator- () const { return thisclass(-x, -y); }

        size_t dimension() const { return (size_t)2; }
        size_t size() const { return dimension(); }

        T* data() { return _array; }
        const T* data() const { return _array; }

        // Conversion operator returning the memory address of the data.
        // Very convenient to pass the data pointer as a parameter to functions.
        // e.g., glVertex2fv(pos);
        operator const T*() const { return _array; }
        operator T*() { return _array; }

		// Liangliang: The compiler can't decide whether to use your overloaded 
		//			   operator[] or the built-in operator[] on the const T*.
		// See https://stackoverflow.com/questions/1726740/c-error-operator-2-overloads-have-similar-conversions 
        //inline T& operator[](size_t idx) {
        //    assert(idx < 2);
        //    return _array[idx];
        //}
		//
        //inline const T& operator[](size_t idx) const {
        //    assert(idx < 2);
        //    return _array[idx];
        //}

        //data intentionally left public to allow vec.x
        union {
            T _array[2];		// array access
            struct { T x, y; };	// standard names for components
            struct { T u, v; };	// standard names for components
        };
    };

    /// Compute the dot product of two 2D vectors.
    template <class T> inline T dot(const Vec<2, T>& v1, const Vec<2, T>& v2) { return v1.x*v2.x + v1.y*v2.y; }

    /// Compute the determinant of the 2x2 matrix formed by the two 2D vectors as the two rows.
    template <class T> inline  T det(const Vec<2, T>& v1, const Vec<2, T>& v2) {
        return v1.x*v2.y - v1.y*v2.x;
    }

    /// The negative vector.
    template <class T> inline Vec<2, T> operator-(const Vec<2, T>& v1) {
        return Vec<2, T>(-v1.x, -v1.y);
    }

    /// Scalar-vector multiplication.
    template <class T2, class T> inline Vec<2, T> operator*(T2 s, const Vec<2, T>& v) {
        return Vec<2, T>(T(s)*v.x, T(s)*v.y);
    }

    /// Vector-vector addition.
    template <class T> inline Vec<2, T> operator+(const Vec<2, T>& v1, const Vec<2, T>& v2) {
        return Vec<2, T>(v1.x + v2.x, v1.y + v2.y);
    }

    /// Vector-vector subtraction.
    template <class T> inline Vec<2, T> operator-(const Vec<2, T>& v1, const Vec<2, T>& v2) {
        return Vec<2, T>(v1.x - v2.x, v1.y - v2.y);
    }


    //---------------- vec3 ------------------------------------------------------------------------

    /**
     * \brief A 3D vector (used for representing 3D points or vectors).
     * \tparam T The scalar type of vector elements.
     *
     * \class Vec<3, T> easy3d/core/vec.h
     */
    template <class T>
    class Vec<3, T> {
    public:
        typedef Vec<3, T> thisclass;
        typedef T         FT;

        Vec() : x(0), y(0), z(0) {}
        explicit Vec(const Vec<2, T>& v, const T& s = 1) : x(v.x), y(v.y), z(s) {} // very useful for promoting to homogeneous coordinates
        explicit Vec(const Vec<4, T>& v) : x(v.x), y(v.y), z(v.z) {} // very useful for inverse promoting from homogeneous coordinates

        Vec(T x_in, T y_in, T z_in) : x(x_in), y(y_in), z(z_in) {}

        explicit Vec(const T& s) : x(s), y(s), z(s) {  }

        template<class T2> explicit Vec(const Vec<3, T2> & v) : x(v.x), y(v.y), z(v.z) {}
        template<class T2> explicit Vec(const T2* v)
            : x(T(v[0])), y(T(v[1])), z(T(v[2])) {}

        inline T length2() const { return x*x + y*y + z*z; }
        inline T length() const { return std::sqrt(x*x + y*y + z*z); }
        inline T norm() const { return length(); }
        inline T distance2(const thisclass& rhs) const {
            T dx = rhs.x - x;
            T dy = rhs.y - y;
            T dz = rhs.z - z;
            return dx*dx + dy*dy + dz*dz;
        }

        inline thisclass& normalize() {
            T s = length();
            s = (s > std::numeric_limits<T>::min()) ? T(1.0) / s : T(0.0);
            *this *= s;
            return *this;
        }

        // operators
        inline thisclass& operator+=(const thisclass& v) { x += v.x; y += v.y; z += v.z; return *this; }
        inline thisclass& operator-=(const thisclass& v) { x -= v.x; y -= v.y; z -= v.z; return *this; }
        inline thisclass& operator*=(const thisclass& v) { x *= v.x; y *= v.y; z *= v.z; return *this; }
        inline thisclass& operator/=(const thisclass& v) { x /= v.x; y /= v.y; z /= v.z; return *this; }
        template <class T2> inline thisclass& operator*=(T2 s) { x *= T(s); y *= T(s); z *= T(s); return *this; }
        template <class T2> inline thisclass& operator/=(T2 s) { x /= T(s); y /= T(s); z /= T(s); return *this; }

        inline thisclass operator+ (const thisclass& v) const { return thisclass(x + v.x, y + v.y, z + v.z); }
        inline thisclass operator- (const thisclass& v) const { return thisclass(x - v.x, y - v.y, z - v.z); }
        template <class T2> inline thisclass operator* (T2 s) const { return thisclass(x*T(s), y*T(s), z*T(s)); }
        template <class T2> inline thisclass operator/ (T2 s) const { return thisclass(x / T(s), y / T(s), z / T(s)); }

        inline thisclass operator- () const { return thisclass(-x, -y, -z); }

        size_t dimension() const { return (size_t)3; }
        size_t size() const { return dimension(); }

        T* data() { return _array; }
        const T* data() const { return _array; }

        // the (x, y) components
        Vec<2, T> xy() const { return  Vec<2, T>(x, y); }

        // Conversion operator returning the memory address of the data.
        // Very convenient to pass the data pointer as a parameter to functions.
        // ------------
        //		vec3 pos, normal;
        //		glNormal3fv(normal);
        //		glVertex3fv(pos);
        // ------------
        operator const T*() const { return _array; }
        operator T*() { return _array; }

		// Liangliang: The compiler can't decide whether to use your overloaded 
		//			   operator[] or the built-in operator[] on the const T*.
		// See https://stackoverflow.com/questions/1726740/c-error-operator-2-overloads-have-similar-conversions 
        //inline T& operator[](size_t idx) {
        //    assert(idx < 3);
        //    return _array[idx];
        //}
		//
        //inline const T& operator[](size_t idx) const {
        //    assert(idx < 3);
        //    return _array[idx];
        //}

        //data intentionally left public to allow vec.x
        union {
            T _array[3];			// array access
            struct { T x, y, z; };	// standard names for components
            struct { T r, g, b; };	// standard names for components
        };
    };

    /// Compute the dot product of two 3D vectors.
    template <class T> inline T dot(const Vec<3, T>& v1, const Vec<3, T>& v2) {
        return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
    }

    /// Compute the cross product of two 3D vectors.
    template <class T> inline  Vec<3, T> cross(const Vec<3, T>& v1, const Vec<3, T>& v2) {
        return Vec<3, T>(
            v1.y*v2.z - v1.z*v2.y,
            v1.z*v2.x - v1.x*v2.z,
            v1.x*v2.y - v1.y*v2.x
            );
    }

    /// The negative vector.
    template <class T> inline Vec<3, T> operator-(const Vec<3, T>& v1) { return Vec<3, T>(-v1.x, -v1.y, -v1.z); }

    /// Scalar-vector multiplication.
    template <class T2, class T> inline Vec<3, T> operator*(T2 s, const Vec<3, T>& v) {
        return Vec<3, T>(T(s)*v.x, T(s)*v.y, T(s)*v.z);
    }

    /// Vector-vector addition.
    template <class T> inline Vec<3, T> operator+(const Vec<3, T>& v1, const Vec<3, T>& v2) {
        return Vec<3, T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
    }

    /// Vector-vector subtraction.
    template <class T> inline Vec<3, T> operator-(const Vec<3, T>& v1, const Vec<3, T>& v2) {
        return Vec<3, T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
    }

    /// Compute a vector that is orthogonal to the given vector.
    template <class T> inline Vec<3, T> orthogonal(const Vec<3, T>& v) {
        T absx = std::fabs(v.x);
        T absy = std::fabs(v.y);
        T absz = std::fabs(v.z);
        // Find smallest component. Keep equal case for null values.
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
     * \brief A 4D vector (used for representing 3D points or vectors in homogeneous coordinates).
     * \tparam T The scalar type of vector elements.
     *
     * \class Vec<4, T> easy3d/core/vec.h
     */
    template <class T>
    class Vec<4, T> {
    public:
        typedef Vec<4, T> thisclass;
        typedef T         FT;

        Vec() : x(0), y(0), z(0), w(0) {}
        explicit Vec(const Vec<3, T>& v, const T& s = 1) : x(v.x), y(v.y), z(v.z), w(s) {} // very useful for promoting to homogeneous coordinates
        Vec(T x_in, T y_in, T z_in, T w_in) : x(x_in), y(y_in), z(z_in), w(w_in) {}

        explicit Vec(const T& s) : x(s), y(s), z(s), w(s) {  }

        template<class T2> explicit Vec(const Vec<4, T2> & v)
            : x(v.x), y(v.y), z(v.z), w(v.w) {}
        template<class T2> explicit Vec(const T2* v)
            : x(v[0]), y(v[1]), z(v[2]), w(v[3]) {}

        inline T length2() const { return x*x + y*y + z*z + w*w; }
        inline T length() const { return std::sqrt(x*x + y*y + z*z + w*w); }
        inline T norm() const { return length(); }
        inline T distance2(const thisclass& rhs) const {
            T dx = rhs.x - x;
            T dy = rhs.y - y;
            T dz = rhs.z - z;
            T dw = rhs.w - w;
            return dx*dx + dy*dy + dz*dz + dw*dw;
        }

        inline thisclass& normalize() {
            T s = length();
            s = (s > std::numeric_limits<T>::min()) ? T(1.0) / s : T(0.0);
            *this *= s;
            return *this;
        }

        size_t dimension() const { return (size_t)4; }
        size_t size() const { return dimension(); }

        // operators
        inline thisclass& operator+=(const thisclass& v) { x += v.x; y += v.y; z += v.z; w += v.w; return *this; }
        inline thisclass& operator-=(const thisclass& v) { x -= v.x; y -= v.y; z -= v.z; w -= v.w; return *this; }
        inline thisclass& operator*=(const thisclass& v) { x *= v.x; y *= v.y; z *= v.z; w *= v.w; return *this; }
        inline thisclass& operator/=(const thisclass& v) { x /= v.x; y /= v.y; z /= v.z; w /= v.w; return *this; }
        template <class T2> inline thisclass& operator*=(T2 s) {
            x *= T(s); y *= T(s); z *= T(s); w *= T(s); return *this;
        }
        template <class T2> inline thisclass& operator/=(T2 s) {
            x /= T(s); y /= T(s); z /= T(s); w /= T(s); return *this;
        }
        inline thisclass operator+ (const thisclass& v) const { return thisclass(x + v.x, y + v.y, z + v.z, w + v.w); }
        inline thisclass operator- (const thisclass& v) const { return thisclass(x - v.x, y - v.y, z - v.z, w - v.w); }
        template <class T2> inline thisclass operator* (T2 s) const { return thisclass(x*T(s), y*T(s), z*T(s), w*T(s)); }
        template <class T2> inline thisclass operator/ (T2 s) const { return thisclass(x / T(s), y / T(s), z / T(s), w / T(s)); }
        inline thisclass operator- () const { return thisclass(-x, -y, -z, -w); }

        T* data() { return _array; }
        const T* data() const { return _array; }

        // the (x, y, z) components
        Vec<3, T> xyz() const { return  Vec<3, T>(x, y, z); }

        // Conversion operator returning the memory address of the data.
        // Very convenient to pass the data pointer as a parameter to functions.
        // e.g., glColor4fv(c);
        operator const T*() const { return _array; }
        operator T*() { return _array; }

		// Liangliang: The compiler can't decide whether to use your overloaded 
		//			   operator[] or the built-in operator[] on the const T*.
		// See https://stackoverflow.com/questions/1726740/c-error-operator-2-overloads-have-similar-conversions 
        //inline T& operator[](size_t idx) {
        //    assert(idx < 4);
        //    return _array[idx];
        //}
		//
        //inline const T& operator[](size_t idx) const {
        //    assert(idx < 4);
        //    return _array[idx];
        //}

        //data intentionally left public to allow vec.x
        union {
            T _array[4];				// array access
            struct { T x, y, z, w; };	// standard names for components
            struct { T r, g, b, a; };	// standard names for components
        };
    };

    /// Compute the dot product of two 4D vectors.
    template <class T> inline T dot(const Vec<4, T>& v1, const Vec<4, T>& v2) {
        return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w;
    }

    /// The negative vector.
    template <class T> inline Vec<4, T> operator-(const Vec<4, T>& v1) { return Vec<4, T>(-v1.x, -v1.y, -v1.z, -v1.w); }

    /// Scalar-vector multiplication.
    template <class T2, class T> inline Vec<4, T> operator*(T2 s, const Vec<4, T>& v) {
        return Vec<4, T>(T(s)*v.x, T(s)*v.y, T(s)*v.z, T(s)*v.w);
    }

    /// Vector-vector addition.
    template <class T> inline Vec<4, T> operator+(const Vec<4, T>& v1, const Vec<4, T>& v2) {
        return Vec<4, T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
    }

    /// Vector-vector subtraction.
    template <class T> inline Vec<4, T> operator-(const Vec<4, T>& v1, const Vec<4, T>& v2) {
        return Vec<4, T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
    }



    //------------------------------- IO (input/output) ----------------------------

    /// Output stream support for Vec.
    template <size_t N, class T> inline std::ostream& operator<<(std::ostream& out, const Vec<N, T>& v) {
        for (size_t i = 0; i < N; i++) {
            out << v[i] << " ";
        }
        return out;
    }

    /// Input stream support for Vec.
    template <size_t N, class T> inline std::istream& operator>>(std::istream& in, Vec<N, T>& v) {
        for (size_t i = 0; i < N; i++) {
            in >> v[i];
        }
        return in;
    }

    /// Output stream support specialized for 2D vectors.
    template <class T> inline std::ostream& operator<<(std::ostream& out, const Vec<2, T>& v) {
        return out << v.x << " " << v.y;
    }

    /// Input stream support specialized for 2D vectors.
    template <class T> inline std::istream& operator>>(std::istream& in, Vec<2, T>& v) {
        return in >> v.x >> v.y;
    }

    /// Output stream support specialized for 3D vectors.
    template <class T> inline std::ostream& operator<<(std::ostream& out, const Vec<3, T>& v) {
        return out << v.x << " " << v.y << " " << v.z;
    }

    /// Input stream support specialized for 3D vectors.
    template <class T> inline std::istream& operator>>(std::istream& in, Vec<3, T>& v) {
        return in >> v.x >> v.y >> v.z;
    }

    /// Output stream support specialized for 4D vectors.
    template <class T> inline std::ostream& operator<<(std::ostream& out, const Vec<4, T>& v) {
        return out << v.x << " " << v.y << " " << v.z << " " << v.w;
    }

    /// Input stream support specialized for 4D vectors.
    template <class T> inline std::istream& operator>>(std::istream& in, Vec<4, T>& v) {
        return in >> v.x >> v.y >> v.z >> v.w;
    }

    //----------------------------------------------------------------------

    /// Do the coordinates of a vector have NaN?
    template <size_t N, class T>
    inline bool has_nan(const Vec<N, T> &v) {
        for (std::size_t i = 0; i < N; ++i) {
            if (std::isnan(v[i]) || std::isinf(v[i]))
                return true;
        }
        return false;
    }

    /// Test if two vectors are strictly identical.
    template <size_t N, class T>
    inline bool operator==(const Vec<N,T> &a, const Vec<N,T> &b) {
        bool t = (a[0] == b[0]);
        unsigned int i=1;
        while(i<N && t) {
            t = t && (a[i]==b[i]);
            ++i;
        }
        return t;
    }

    /// Test if two vectors are strictly not identical.
    template <size_t N, class T>
    inline bool operator!=(const Vec<N,T> &a, const Vec<N,T> &b) {
        bool t = (a[0] != b[0]);
        unsigned int i=1;
        while(i<N && !t) {
            t = t || (a[i]!=b[i]);
            ++i;
        }
        return t;
    }

    /// Lexicographic comparison of two vectors
    template <size_t N, class T>
    inline bool operator<(const Vec<N,T> &a, const Vec<N,T> &b) {
        for(unsigned int i=0; i<N; ++i){
            if(a[i]<b[i]) return true;
            if(a[i]>b[i]) return false;
        }
        return false;
    }

    /// Component-wise product of two vectors
    template <size_t N, class T>
    inline Vec<N, T> comp_product(const Vec<N, T> &v1, const Vec<N, T> &v2) {
        Vec<N, T> result;
        for (int i = 0; i < N; ++i)
            result[i] = v1[i] * v2[i];
        return result;
    }

    /// Component-wise minimum vector
    template <size_t N, class T>
    inline Vec<N, T> comp_min(const Vec<N, T> &v1, const Vec<N, T> &v2) {
        Vec<N, T> result;
        for (int i = 0; i < N; ++i)
            result[i] = std::min(v1[i], v2[i]);
        return result;
    }

    /// Component-wise maximum vector
    template <size_t N, class T>
    inline Vec<N, T> comp_max(const Vec<N, T> &v1, const Vec<N, T> &v2) {
        Vec<N, T> result;
        for (int i = 0; i < N; ++i)
            result[i] = std::max(v1[i], v2[i]);
        return result;
    }

    /// The minimum coordinate of elements in a vector
    template<size_t N, class T>
    inline T min_coord(const Vec<N, T> &a) {
        T result = a[0];
        for (unsigned int i = 1; i < N; ++i) if (a[i] < result) result = a[i];
        return result;
    }

    /// The maximum coordinate of elements in a vector
    template<size_t N, class T>
    inline T max_coord(const Vec<N, T> &a) {
        T result = a[0];
        for (unsigned int i = 1; i < N; ++i) if (a[i] > result) result = a[i];
        return result;
    }

    /// Component-wise clamp
    template<size_t N, class T>
    inline Vec<N, T> clamp(const Vec<N, T> &a, const Vec<N, T> &lower, const Vec<N, T> &upper) {
        Vec<N, T> result;
        for (unsigned int i = 0; i < N; ++i) result[i] = std::min(upper[i], std::max(a[i], lower[i]));
        return result;
    }

#else


    template <size_t DIM, typename T> struct Vec {
        Vec() { for (size_t i=DIM; i--; data_[i] = T()); }
              T& operator[](const size_t i)       { assert(i<DIM); return data_[i]; }
        const T& operator[](const size_t i) const { assert(i<DIM); return data_[i]; }
    private:
        T data_[DIM];
    };

    typedef Vec<2, float> vec2;
    typedef Vec<3, float> vec3;
    typedef Vec<3, int  > vec3i;
    typedef Vec<4, float> vec4;

    template <typename T> struct Vec<2,T> {
        Vec() : x(T()), y(T()) {}
        Vec(T X, T Y) : x(X), y(Y) {}
        template <class U> Vec<2,T>(const Vec<2,U> &v);
              T& operator[](const size_t i)       { assert(i<2); return i<=0 ? x : y; }
        const T& operator[](const size_t i) const { assert(i<2); return i<=0 ? x : y; }
        T x,y;
    };

    template <typename T> struct Vec<3,T> {
        Vec() : x(T()), y(T()), z(T()) {}
        Vec(T X, T Y, T Z) : x(X), y(Y), z(Z) {}
              T& operator[](const size_t i)       { assert(i<3); return i<=0 ? x : (1==i ? y : z); }
        const T& operator[](const size_t i) const { assert(i<3); return i<=0 ? x : (1==i ? y : z); }
        float norm() { return std::sqrt(x*x+y*y+z*z); }
        Vec<3,T> & normalize(T l=1) { *this = (*this)*(l/norm()); return *this; }
        T x,y,z;
    };

    template <typename T> struct Vec<4,T> {
        Vec() : x(T()), y(T()), z(T()), w(T()) {}
        Vec(T X, T Y, T Z, T W) : x(X), y(Y), z(Z), w(W) {}
              T& operator[](const size_t i)       { assert(i<4); return i<=0 ? x : (1==i ? y : (2==i ? z : w)); }
        const T& operator[](const size_t i) const { assert(i<4); return i<=0 ? x : (1==i ? y : (2==i ? z : w)); }
        T x,y,z,w;
    };

    template<size_t DIM,typename T> T operator*(const Vec<DIM,T>& lhs, const Vec<DIM,T>& rhs) {
        T ret = T();
        for (size_t i=DIM; i--; ret+=lhs[i]*rhs[i]);
        return ret;
    }

    template<size_t DIM,typename T>Vec<DIM,T> operator+(Vec<DIM,T> lhs, const Vec<DIM,T>& rhs) {
        for (size_t i=DIM; i--; lhs[i]+=rhs[i]);
        return lhs;
    }

    template<size_t DIM,typename T>Vec<DIM,T> operator-(Vec<DIM,T> lhs, const Vec<DIM,T>& rhs) {
        for (size_t i=DIM; i--; lhs[i]-=rhs[i]);
        return lhs;
    }

    template<size_t DIM,typename T,typename U> Vec<DIM,T> operator*(const Vec<DIM,T> &lhs, const U& rhs) {
        Vec<DIM,T> ret;
        for (size_t i=DIM; i--; ret[i]=lhs[i]*rhs);
        return ret;
    }

    template<size_t DIM,typename T> Vec<DIM,T> operator-(const Vec<DIM,T> &lhs) {
        return lhs*T(-1);
    }


    template<size_t DIM,typename T> T dot(const Vec<DIM,T>& lhs, const Vec<DIM,T>& rhs) {
        T ret = T();
        for (size_t i=DIM; i--; ret+=lhs[i]*rhs[i]);
        return ret;
    }

    template <typename T> Vec<3,T> cross(Vec<3,T> v1, Vec<3,T> v2) {
        return Vec<3,T>(v1.y*v2.z - v1.z*v2.y, v1.z*v2.x - v1.x*v2.z, v1.x*v2.y - v1.y*v2.x);
    }

    template <size_t DIM, typename T> std::ostream& operator<<(std::ostream& out, const Vec<DIM,T>& v) {
        for(unsigned int i=0; i<DIM; i++) out << v[i] << " " ;
        return out ;
    }

#endif

}

#endif  // EASY3D_CORE_VEC_H

