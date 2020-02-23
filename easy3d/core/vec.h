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


#ifndef EASY3D_VEC_H
#define EASY3D_VEC_H

#include <cassert>
#include <iostream>
#include <cmath>
#include <cfloat>
#include <limits>


namespace easy3d {

#if 1
    template <size_t N, class T>
    class Vec {
    public:
        typedef Vec<N, T> thisclass;

        Vec() { for (size_t i = 0; i < N; i++) { data_[i] = T(0); } }

        explicit Vec(const T& s) { for (size_t i = 0; i < N; i++) { data_[i] = s; } }

        // This one should never be called : a template constructor cannot be a copy constructor
        template<class T2> explicit Vec(const Vec<N, T2>& rhs) {
            for (size_t i = 0; i < N; i++) {
                data_[i] = T(rhs[i]);
            }
        }

        // to avoid compilation problems
        template<class T2, size_t M> explicit Vec(const Vec<M, T2>& rhs) {
            assert(M == N);
            for (size_t i = 0; i < N; i++) {
                data_[i] = T(rhs[i]);
            }
        }
        template<class T2> explicit Vec(const T2* rhs) {
            for (size_t i = 0; i < N; i++) {
                data_[i] = T(rhs[i]);
            }
        }

        thisclass& operator=(const thisclass& rhs) {
            memcpy(data_, rhs.data(), N*sizeof(T));
            return *this;
        }

        size_t dimension() const { return (size_t)N; }
        size_t size() const { return dimension(); }

        // returns the memory address of the vector.
        T* data()             { return data_; }
        const T* data() const { return data_; }

        // Conversion operator returning the memory address of the data.
        // Very convenient to pass the data pointer as a parameter to functions.
        // e.g., glVertex3fv(pos);
        operator const T*() const { return data_; }
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

        inline T length2() const {
            T result = T(0);
            for (size_t i = 0; i < N; i++) {
                result += data_[i] * data_[i];
            }
            return result;
        }

        inline T length() const {
            return sqrt(length2());
        }

        inline T norm() const {
            return length();
        }

        inline T distance2(const thisclass &rhs) const {
            T result = T(0);
            for (size_t i = 0; i < N; i++) {
                T val = rhs.data_[i] - data_[i];
                result += val*val;
            }
            return result;
        }

        inline thisclass& normalize() {
            T s = length();
            s = (s > std::numeric_limits<T>::min()) ? T(1.0) / s : T(0.0);
            *this *= s;
            return *this;
        }

        // operators
        inline thisclass& operator+=(const thisclass& v) {
            for (size_t i = 0; i < N; i++) {
                data_[i] += v.data_[i];
            }
            return *this;
        }

        inline thisclass& operator-=(const thisclass& v) {
            for (size_t i = 0; i < N; i++) {
                data_[i] -= v.data_[i];
            }
            return *this;
        }

        inline thisclass& operator*=(const thisclass& v) {
            for (size_t i = 0; i < N; i++) {
                data_[i] *= v.data_[i];
            }
            return *this;
        }

        inline thisclass& operator/=(const thisclass& v) {
            for (size_t i = 0; i < N; i++) {
                data_[i] /= v.data_[i];
            }
            return *this;
        }

        template <class T2> inline thisclass& operator*=(T2 s) {
            for (size_t i = 0; i < N; i++) {
                data_[i] *= T(s);
            }
            return *this;
        }

        template <class T2> inline thisclass& operator/=(T2 s) {
            for (size_t i = 0; i < N; i++) {
                data_[i] /= T(s);
            }
            return *this;
        }

        inline thisclass operator+ (const thisclass& v) const {
            thisclass result(*this);
            for (size_t i = 0; i < N; i++) {
                result.data_[i] += v.data_[i];
            }
            return result;
        }

        inline thisclass operator- (const thisclass& v) const {
            thisclass result(*this);
            for (size_t i = 0; i < N; i++) {
                result.data_[i] -= v.data_[i];
            }
            return result;
        }

        template <class T2> inline thisclass operator* (T2 s) const {
            thisclass result(*this);
            for (size_t i = 0; i < N; i++) {
                result.data_[i] *= T(s);
            }
            return result;
        }

        template <class T2> inline thisclass operator/ (T2 s) const {
            thisclass result(*this);
            for (size_t i = 0; i < N; i++) {
                result.data_[i] /= T(s);
            }
            return result;
        }

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


    template <size_t N, class T> inline T dot(const Vec<N, T>& v1, const Vec<N, T>& v2) {
        T result = 0;
        for (size_t i = 0; i < N; i++) {
            result += v1[i] * v2[i];
        }
        return result;
    }

    template <size_t N, class T> inline Vec<N, T> operator-(const Vec<N, T>& v1) {
        Vec<N, T> result;
        for (size_t i = 0; i < N; i++) {
            result[i] = -v1[i];
        }
        return result;
    }

    template <class T2, size_t N, class T> inline Vec<N, T> operator*(T2 s, const Vec<N, T>& v) {
        Vec<N, T> result;
        for (size_t i = 0; i < N; i++) {
            result[i] = T(s)*v[i];
        }
        return result;
    }

    template <size_t N, class T> inline Vec<N, T> operator+(const Vec<N, T>& v1, const Vec<N, T>& v2) {
        Vec<N, T> result;
        for (size_t i = 0; i < N; i++) {
            result[i] = v1[i] + v2[i];
        }
        return result;
    }

    template <size_t N, class T> inline Vec<N, T> operator-(const Vec<N, T>& v1, const Vec<N, T>& v2) {
        Vec<N, T> result;
        for (size_t i = 0; i < N; i++) {
            result[i] = v1[i] - v2[i];
        }
        return result;
    }

    // Compatibility with GLSL
    template <size_t N, class T> inline T length(const Vec<N, T>& v) { return v.length(); }
    template <size_t N, class T> inline T norm(const Vec<N, T>& v) { return v.length(); }
    template <size_t N, class T> inline T length2(const Vec<N, T>& v) { return v.length2(); }
    template <size_t N, class T> inline T distance(const Vec<N, T>& v1, const Vec<N, T>& v2)	{ return length(v2 - v1); }
    template <size_t N, class T> inline T distance2(const Vec<N, T>& v1, const Vec<N, T>& v2)	{ return v2.distance2(v1); }
    template <size_t N, class T> inline Vec<N, T> normalize(const Vec<N, T>& v) {
        T s = v.length();
        s = (s > std::numeric_limits<T>::min()) ? T(1.0) / s : T(0.0);
        return v * s;
    }

    // linear interpolation between x and y using a to weight between them.
    // The return value is computed as (1 − w) * v1 + w * v2.
    template <size_t N, class T> inline Vec<N, T> mix(const Vec<N, T>& v1, const Vec<N, T>& v2, T w) {
        return (T(1) - w) * v1 + w * v2;
    }

    //-------------------- vec2 -------------------------------------------------------------------

    template <class T>
    class Vec<2, T> {
    public:
        typedef Vec<2, T> thisclass;

        Vec() : x(0), y(0) { }
        Vec(T x_in, T y_in) : x(x_in), y(y_in) { }
        Vec(const Vec<3, T>& v) : x(v.x), y(v.y) { } // very useful for inverse promoting from homogeneous coordinates

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
            struct { T x, y; };	// standard names for components
            struct { T u, v; };	// standard names for components
            T _array[2];		// array access
        };
    };

    template <class T> inline T dot(const Vec<2, T>& v1, const Vec<2, T>& v2) { return v1.x*v2.x + v1.y*v2.y; }

    template <class T> inline  T det(const Vec<2, T>& v1, const Vec<2, T>& v2) {
        return v1.x*v2.y - v1.y*v2.x;
    }


    template <class T> inline Vec<2, T> operator-(const Vec<2, T>& v1) {
        return Vec<2, T>(-v1.x, -v1.y);
    }
    template <class T2, class T> inline Vec<2, T> operator*(T2 s, const Vec<2, T>& v) {
        return Vec<2, T>(T(s)*v.x, T(s)*v.y);
    }

    template <class T> inline Vec<2, T> operator+(const Vec<2, T>& v1, const Vec<2, T>& v2) {
        return Vec<2, T>(v1.x + v2.x, v1.y + v2.y);
    }

    template <class T> inline Vec<2, T> operator-(const Vec<2, T>& v1, const Vec<2, T>& v2) {
        return Vec<2, T>(v1.x - v2.x, v1.y - v2.y);
    }


    //---------------- vec3 ------------------------------------------------------------------------

    template <class T>
    class Vec<3, T> {
    public:
        typedef Vec<3, T> thisclass;

        Vec() : x(0), y(0), z(0) {}
        Vec(const Vec<2, T>& v, const T& s = 0) : x(v.x), y(v.y), z(s) {} // very useful for promoting to homogeneous coordinates
        Vec(const Vec<4, T>& v) : x(v.x), y(v.y), z(v.z) {} // very useful for inverse promoting from homogeneous coordinates

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
            struct { T x, y, z; };	// standard names for components
            struct { T u, v, w; };	// standard names for components
            struct { T r, g, b; };	// standard names for components
            T _array[3];			// array access
        };
    };

    template <class T> inline T dot(const Vec<3, T>& v1, const Vec<3, T>& v2) {
        return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z;
    }

    template <class T> inline  Vec<3, T> cross(const Vec<3, T>& v1, const Vec<3, T>& v2) {
        return Vec<3, T>(
            v1.y*v2.z - v1.z*v2.y,
            v1.z*v2.x - v1.x*v2.z,
            v1.x*v2.y - v1.y*v2.x
            );
    }

    template <class T> inline Vec<3, T> operator-(const Vec<3, T>& v1) { return Vec<3, T>(-v1.x, -v1.y, -v1.z); }
    template <class T2, class T> inline Vec<3, T> operator*(T2 s, const Vec<3, T>& v) {
        return Vec<3, T>(T(s)*v.x, T(s)*v.y, T(s)*v.z);
    }

    template <class T> inline Vec<3, T> operator+(const Vec<3, T>& v1, const Vec<3, T>& v2) {
        return Vec<3, T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
    }

    template <class T> inline Vec<3, T> operator-(const Vec<3, T>& v1, const Vec<3, T>& v2) {
        return Vec<3, T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
    }

    template <class T> inline Vec<3, T> orthogonal(const Vec<3, T>& v) {
        T absx = ::fabs(v.x);
        T absy = ::fabs(v.y);
        T absz = ::fabs(v.z);
        // Find smallest component. Keep equal case for null values.
        if ((absy >= absx) && (absz >= absx))
            return Vec<3, T>(0.0, -v.z, v.y);
        else
            if ((absx >= absy) && (absz >= absy))
                return Vec<3, T>(-v.z, 0.0f, v.x);
            else
                return Vec<3, T>(-v.y, v.x, 0.0f);
    }

    // ----------------- vec4 ----------------------------------------------------------------------------------

    template <class T>
    class Vec<4, T> {
    public:
        typedef Vec<4, T> thisclass;

        Vec() : x(0), y(0), z(0), w(0) {}
        Vec(const Vec<3, T>& v, const T& s = 0) : x(v.x), y(v.y), z(v.z), w(s) {} // very useful for promoting to homogeneous coordinates
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
            struct { T x, y, z, w; };	// standard names for components
            struct { T r, g, b, a; };	// standard names for components
            T _array[4];				// array access
        };
    };

    template <class T> inline T dot(const Vec<4, T>& v1, const Vec<4, T>& v2) {
        return v1.x*v2.x + v1.y*v2.y + v1.z*v2.z + v1.w*v2.w;
    }

    template <class T> inline Vec<4, T> operator-(const Vec<4, T>& v1) { return Vec<4, T>(-v1.x, -v1.y, -v1.z, -v1.w); }
    template <class T2, class T> inline Vec<4, T> operator*(T2 s, const Vec<4, T>& v) {
        return Vec<4, T>(T(s)*v.x, T(s)*v.y, T(s)*v.z, T(s)*v.w);
    }

    template <class T> inline Vec<4, T> operator+(const Vec<4, T>& v1, const Vec<4, T>& v2) {
        return Vec<4, T>(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
    }

    template <class T> inline Vec<4, T> operator-(const Vec<4, T>& v1, const Vec<4, T>& v2) {
        return Vec<4, T>(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
    }



    //------------------------------- IO (input/output) ----------------------------

    template <size_t N, class T> inline std::ostream& operator<<(std::ostream& out, const Vec<N, T>& v) {
        for (size_t i = 0; i < N; i++) {
            out << v[i] << " ";
        }
        return out;
    }

    template <size_t N, class T> inline std::istream& operator>>(std::istream& in, Vec<N, T>& v) {
        for (size_t i = 0; i < N; i++) {
            in >> v[i];
        }
        return in;
    }

    template <class T> inline std::ostream& operator<<(std::ostream& out, const Vec<2, T>& v) {
        return out << v.x << " " << v.y;
    }

    template <class T> inline std::istream& operator>>(std::istream& in, Vec<2, T>& v) {
        return in >> v.x >> v.y;
    }

    template <class T> inline std::ostream& operator<<(std::ostream& out, const Vec<3, T>& v) {
        return out << v.x << " " << v.y << " " << v.z;
    }

    template <class T> inline std::istream& operator>>(std::istream& in, Vec<3, T>& v) {
        return in >> v.x >> v.y >> v.z;
    }

    template <class T> inline std::ostream& operator<<(std::ostream& out, const Vec<4, T>& v) {
        return out << v.x << " " << v.y << " " << v.z << " " << v.w;
    }

    template <class T> inline std::istream& operator>>(std::istream& in, Vec<4, T>& v) {
        return in >> v.x >> v.y >> v.z >> v.w;
    }

    //----------------------------------------------------------------------

    template <class Vec> inline
    bool has_nan(const Vec& v) {
        for (std::size_t i=0; i<v.size(); ++i) {
            if (std::isnan(v[i]) || std::isinf(v[i]))
                return true;
        }
        return false;
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

#endif  // EASY3D_VEC_H

