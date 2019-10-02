/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work, 
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan. 
*           Easy3D: a lightweight, easy-to-use, and efficient C++ 
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/


/** ----------------------------------------------------------
 *
 * The code is adapted from libQGLViewer with modifications.
 *		- libQGLViewer (version Version 2.7.1, Nov 17th, 2017)
 * The original code is available at
 * http://libqglviewer.com/
 *
 * libQGLViewer is a C++ library based on Qt that eases the
 * creation of OpenGL 3D viewers.
 *
 *----------------------------------------------------------*/

#ifndef EASY3D_QUATERNION_H
#define EASY3D_QUATERNION_H


#include <easy3d/core/constant.h>
#include <easy3d/core/vec.h>
#include <easy3d/core/mat.h>

/**
 * Euler angles VS Quaternion
 *
 * Euler angles are the easiest way to represent rotation. This representation simply
 * stores the three rotation angles around the X, Y and Z axes. These 3 rotations are
 * then applied successively, usually in this order: first Y, then Z, then X (but not
 * necessarily). Using a different order yields different results.
 * Euler angles are usually used to set a character's orientation since game characters
 * only rotate on the vertical axis. Therefore, it is easier to write, understand and
 * maintain "float direction" than 3 different orientations. Another good use of Euler
 * angles is an FPS camera: you have one angle for heading (Y), and one for up/down (X).
 * However, when things get more complex, Euler angle will be hard to work with, eg.,:
 *  - Interpolating smoothly between 2 orientations is hard. Naively interpolating the
 *    X, Y, and Z angles will be ugly.
 *  - Applying several rotations is complicated and unprecise: you have to compute the
 *    final rotation matrix, and guess the Euler angles from this matrix.
 *  - A well-known problem, the "Gimbal Lock", will sometimes block your rotations, and
 *    other singularities which will flip your model upside-down.
 *  - Different angles make the same rotation (-180 and 180 degrees, for instance)
 *  - It is a mess - as said above, usually the right order is YZX, but if you also use
 *    a library with a different order, you'll be in trouble.
 *  - Some operations are complicated, e.g., rotation of N degrees around a specific axis.
 * Quaternions are a tool to represent rotations, which solves these problems.
 *
 *
 * The Quaternion class represents 3D rotations and orientations.
 *
 * The Quaternion is an appropriate (although not very intuitive) representation for
 * 3D rotations and orientations. Many tools are provided to ease the definition of a
 * Quaternion: see constructors, set_axis_angle(), set_from_rotation_matrix(),
 * set_from_rotated_basis().
 *
 * You can apply the rotation represented by the Quaternion to 3D points using rotate()
 * and inverse_rotate().
 *
 * You can apply the Quaternion q rotation to the OpenGL matrices using
 *      glMultMatrixd(q.matrix());
 * which is equvalent to
 *      glRotate(q.angle()*180.0/M_PI, q.axis().x, q.axis().y, q.axis().z);
 *
 * Internal representation
 * The internal representation of a Quaternion is a set of 4 numbers, [x y z w], which
 * represents rotations the following way:
 *  x = axis.x * sin(angle / 2)
 *  y = axis.y * sin(angle / 2)
 *  z = axis.z * sin(angle / 2)
 *  w = cos(angle / 2)
 * NOTE:
 *   - the angle is in radians and the axis is a unit vector.
 *   - certain implementations place the cosine term in the first position (instead of last).
 *
 * A Quaternion is always normalized, so that its inverse() is actually its conjugate.
 */

namespace easy3d {


    template <typename FT>
    class Quat
    {
    public:
        typedef Vec<3, FT>	Vec3;
        typedef Quat<FT>	thisclass;

    public:
        /* Defining a Quat */
        /* Default constructor, builds an identity rotation. */
        Quat()
        {
            _q[0] = _q[1] = _q[2] = FT(0);  _q[3] = FT(1);
        }

        /* Constructor from rotation axis (non null) and angle (in radians). See also set_axis_angle(). */
        Quat(const Vec3& axis, FT angle)
        {
            set_axis_angle(axis, angle);
        }

        /* Constructs a Quaternion that will rotate from the \p from direction to the \p to direction.
        Note that this rotation is not uniquely defined. The selected axis is usually orthogonal to \p from
        and \p to, minimizing the rotation angle. This method is robust and can handle small or almost identical vectors. */
        Quat(const Vec3& from, const Vec3& to);

        /* Constructor from the four values of a Quaternion. First three values are axis*sin(angle/2) and
        last one is cos(angle/2).

        \attention The identity Quaternion is Quat(0,0,0,1) and not Quat(0,0,0,0) (which is
        not unitary). The default Quat() creates such identity Quaternion. */
        Quat(FT q0, FT q1, FT q2, FT q3)
        { _q[0]=q0;    _q[1]=q1;    _q[2]=q2;    _q[3]=q3; }

        /* Copy constructor. */
        Quat(const thisclass& Q)
        { for (int i=0; i<4; ++i) _q[i] = Q._q[i]; }

        /* Equal operator. */
        Quat& operator=(const thisclass& Q) {
            for (int i=0; i<4; ++i)
                _q[i] = Q._q[i];
            return (*this);
        }

        /* Sets the Quaternion as a rotation of axis and angle (in radians).
        \p axis does not need to be normalized. A null axis will result in an identity Quaternion. */
        void set_axis_angle(const Vec3& axis, FT angle);

        /* Sets the Quaternion value. */
        void set_value(FT q0, FT q1, FT q2, FT q3)
        { _q[0]=q0;    _q[1]=q1;    _q[2]=q2;    _q[3]=q3; }

        /* Set the Quaternion from a (supposedly correct) 3x3 rotation matrix.
        The matrix is expressed in European format: its three columns are the images by the rotation of
        the three vectors of an orthogonal basis. */
        // see http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
        void set_from_rotation_matrix(const Mat3<FT>& m);

        /* set_from_rotated_basis() sets a Quaternion from the three axis of a rotated frame. It actually fills
        the three columns of a matrix with these rotated basis vectors and calls this method. */
        void set_from_rotated_basis(const Vec3& X, const Vec3& Y, const Vec3& Z);

        /* Returns the normalized axis direction of the rotation represented by the Quaternion.
        It is null for an identity Quaternion. See also angle() and get_axis_angle(). */
        Vec3 axis() const;

        /* Returns the angle (in radians) of the rotation represented by the Quaternion.
        This value is always in the range [0-pi]. Larger rotational angles are obtained by inverting the
        axis() direction. See also axis() and get_axis_angle(). */
        FT angle() const;

        /* Returns the axis vector and the angle (in radians) of the rotation represented by the Quaternion.*/
        void get_axis_angle(Vec3& axis, FT& angle) const;

        /* Bracket operator, with a constant return value. i must range in [0..3]. */
        FT operator[](int i) const { return _q[i]; }

        /* Bracket operator returning an l-value. i must range in [0..3]. */
        FT& operator[](int i) { return _q[i]; }

        /* Rotation computations */

        /* Returns the composition of the a and b rotations.
        The order is important. When applied to a Vec v (see operator*(const Quaternion&, const Vec&)
        and rotate()) the resulting Quaternion acts as if b was applied first and then a was applied.
        This is obvious since the image v' of v by the composited rotation satisfies:
        v'= (a*b) * v = a * (b*v)
        Note that a*b usually differs from b*a.
        \attention For efficiency reasons, the resulting Quaternion is not normalized. Use normalize() in
        case of numerical drift with small rotation composition. */
        friend Quat operator*(const Quat& a, const Quat& b)
        {
            return Quat(
                a._q[3]*b._q[0] + b._q[3]*a._q[0] + a._q[1]*b._q[2] - a._q[2]*b._q[1],
                a._q[3]*b._q[1] + b._q[3]*a._q[1] + a._q[2]*b._q[0] - a._q[0]*b._q[2],
                a._q[3]*b._q[2] + b._q[3]*a._q[2] + a._q[0]*b._q[1] - a._q[1]*b._q[0],
                a._q[3]*b._q[3] - b._q[0]*a._q[0] - a._q[1]*b._q[1] - a._q[2]*b._q[2]
                );
        }

        /* Quaternion rotation is composed with q.
        See operator*(), since this is equivalent to this = this * q.
        \note For efficiency reasons, the resulting Quaternion is not normalized.
        You may normalize() it after each application in case of numerical drift. */
        Quat& operator*=(const Quat &q) {
            *this = (*this)*q;
            return *this;
        }

        /* Returns the image of v by the rotation q.
        Same as q.rotate(v). See rotate() and inverse_rotate(). */
        friend Vec3 operator*(const Quat& q, const Vec3& v) { return q.rotate(v); }

        /* Returns the image of v by the Quaternion rotation.
        See also inverse_rotate() and operator*(const Quat&, const Vec&). */
        Vec3 rotate(const Vec3& v) const;

        /* Returns the image of v by the Quaternion inverse() rotation.
        rotate() performs an inverse transformation. Same as inverse().rotate(v). */
        Vec3 inverse_rotate(const Vec3& v) const {
            return inverse().rotate(v);
        }

        /* Inversion */
        /* Returns the inverse Quaternion (inverse rotation).
        Result has a negated axis() direction and the same angle(). A composition (see operator*()) of a
        Quaternion and its inverse() results in an identity function.
        Use invert() to actually modify the Quaternion. */
        Quat inverse() const { return Quat(-_q[0], -_q[1], -_q[2], _q[3]); }

        /* Inverses the Quaternion (same rotation angle(), but negated axis()).
        See also inverse(). */
        void invert() { _q[0] = -_q[0]; _q[1] = -_q[1]; _q[2] = -_q[2]; }

        /* Negates all the coefficients of the Quaternion.
        This results in an other representation of the same rotation (opposite rotation angle, but with
        a negated axis direction: the two cancel out). However, note that the results of axis() and
        angle() are unchanged after a call to this method since angle() always returns a value in [0,pi].
        This method is mainly useful for Quaternion interpolation, so that the spherical interpolation
        takes the shortest path on the unit sphere. See slerp() for details. */
        void negate() { invert(); _q[3] = -_q[3]; }

        FT length() const {
            return std::sqrt(_q[0] * _q[0] + _q[1] * _q[1] + _q[2] * _q[2] + _q[3] * _q[3]);
        }

        /* Normalizes the Quaternion coefficients.
        This method should not need to be called since we only deal with unit Quaternions. This is however
        useful to prevent numerical drifts, especially with small rotational increments. See also
        normalized(). */
        FT normalize() {
            const FT norm = std::sqrt(_q[0] * _q[0] + _q[1] * _q[1] + _q[2] * _q[2] + _q[3] * _q[3]);
            for (int i=0; i<4; ++i)
                _q[i] /= norm;
            return norm;
        }

        /* Returns a normalized version of the Quaternion.
        See also normalize(). */
        Quat normalized() const {
            FT Q[4];
            const FT norm = std::sqrt(_q[0] * _q[0] + _q[1] * _q[1] + _q[2] * _q[2] + _q[3] * _q[3]);
            for (int i=0; i<4; ++i)
                Q[i] = _q[i] / norm;
            return Quat(Q[0], Q[1], Q[2], Q[3]);
        }

        /* Returns the Quaternion associated 4x4 rotation matrix.
        Use glMultMatrixf(q.matrix()) to apply the rotation represented by Quaternion q to the
        current OpenGL matrix. */
        Mat4<FT> matrix() const;

        /* Returns the associated 4x4 inverse rotation matrix. This is simply the matrix() of the inverse().*/
        Mat4<FT> inverse_matrix() const;

        /* Interpolate between 2 quaternions */
        /* Slerp(Spherical Linear intERPolation) interpolation */
        /* Returns the slerp interpolation of Quaternions a and b, at time t. t should range in [0,1].
        Result is a when t=0 and b when t=1.
        When allowFlip is true (default) the slerp interpolation will always use the "shortest path" between
        the Quaternions' orientations, by "flipping" the source Quaternion if needed (see negate()). */
        static Quat slerp(const Quat<FT>& a, const Quat<FT>& b, FT t, bool allowFlip = true);

        /* Returns the slerp interpolation of the two Quaternions a and b, at time t, using tangents tgA and tgB.
        The resulting Quaternion is "between" a and b (result is a when t=0 and b for t=1).
        Use squad_tangent() to define the Quaternion tangents tgA and tgB. */
        static Quat squad(const Quat<FT>& a, const Quat<FT>& tgA, const Quat<FT>& tgB, const Quat<FT>& b, FT t);

        /* Returns the "dot" product of a and b: a[0]*b[0] + a[1]*b[1] + a[2]*b[2] + a[3]*b[3]. */
        static FT dot(const Quat<FT>& a, const Quat<FT>& b) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2] + a[3] * b[3]; }

        /* Returns the logarithm of the Quaternion. See also exp(). */
        Quat log();
        /* Returns the exponential of the Quaternion. See also log(). */
        Quat exp();

        /* Returns log(a. inverse() * b). Useful for squad_tangent(). */
        static Quat ln_dif(const Quat<FT>& a, const Quat<FT>& b);
        /* Returns a tangent Quaternion for \p center, defined by \p before and \p after Quaternions.
        Useful for smooth spline interpolation of Quaternion with squad() and slerp(). */
        static Quat squad_tangent(const Quat<FT>& before, const Quat<FT>& center, const Quat<FT>& after);

        /* Returns a random unit Quaternion.
        You can create a randomly directed unit vector using:
        Vec randomDir = Quat::random_quat() * Vec(1.0, 0.0, 0.0); // or any other Vec
        \note This function uses rand() to create pseudo-random numbers and the random number generator can
        be initialized using srand().*/
        static thisclass random_quat();

        //data intentionally left public to allow q.x ...
    public:
        /* The internal data representation is private, use operator[] to access values. */
        union {
            struct { FT x; FT y; FT z; FT w; };
            FT _q[4];
        };
    };

    template <typename FT>	std::ostream& operator<<(std::ostream& os, const Quat<FT>& q);
    template <typename FT>	std::istream& operator>>(std::istream& is, Quat<FT>& q);


    //-------------- implementation -----------------


    template <typename FT>
    Quat<FT>::Quat(const Vec3& from, const Vec3& to)
    {
        const FT epsilon = FT(1E-10);

        const FT fromSqNorm = from.length2();
        const FT toSqNorm = to.length2();
        // Identity Quaternion when one vector is null
        if ((fromSqNorm < epsilon) || (toSqNorm < epsilon))
        {
            _q[0] = _q[1] = _q[2] = 0.0;
            _q[3] = 1.0;
        }
        else
        {
            Vec3 axis = cross(from, to);
            const FT axisSqNorm = axis.length2();

            // Aligned vectors, pick any axis, not aligned with from or to
            if (axisSqNorm < epsilon)
                axis = orthogonal(from);

            FT angle = std::asin(std::sqrt(axisSqNorm / (fromSqNorm * toSqNorm)));

            if (easy3d::dot(from, to) < 0.0)
                angle = FT(M_PI - angle);

            set_axis_angle(axis, angle);
        }
    }

    template <typename FT>
    void Quat<FT>::set_axis_angle(const Vec3& axis, FT angle)
    {
        const FT norm = axis.length();
        if (norm < 1E-8)
        {
            // Null rotation
            _q[0] = FT(0);      _q[1] = FT(0);       _q[2] = FT(0);      _q[3] = FT(1);
        }
        else
        {
            const FT sin_half_angle = std::sin(angle / 2.0f);
            _q[0] = sin_half_angle*axis[0] / norm;
            _q[1] = sin_half_angle*axis[1] / norm;
            _q[2] = sin_half_angle*axis[2] / norm;
            _q[3] = (FT)std::cos(angle / 2.0f);
        }
    }


    template <typename FT>
    typename Quat<FT>::Vec3 Quat<FT>::rotate(const Vec3& v) const
    {
        const FT q00 = FT(2.0l * _q[0] * _q[0]);
        const FT q11 = FT(2.0l * _q[1] * _q[1]);
        const FT q22 = FT(2.0l * _q[2] * _q[2]);

        const FT q01 = FT(2.0l * _q[0] * _q[1]);
        const FT q02 = FT(2.0l * _q[0] * _q[2]);
        const FT q03 = FT(2.0l * _q[0] * _q[3]);

        const FT q12 = FT(2.0l * _q[1] * _q[2]);
        const FT q13 = FT(2.0l * _q[1] * _q[3]);

        const FT q23 = FT(2.0l * _q[2] * _q[3]);

        return Vec3(
            FT((1.0 - q11 - q22)*v[0] + (q01 - q23)*v[1] + (q02 + q13)*v[2]),
            FT((q01 + q23)*v[0] + (1.0 - q22 - q00)*v[1] + (q12 - q03)*v[2]),
            FT((q02 - q13)*v[0] + (q12 + q03)*v[1] + (1.0 - q11 - q00)*v[2]));
    }

    template <typename FT>
    void Quat<FT>::set_from_rotation_matrix(const Mat3<FT>& m)
    {
        //	see http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
        // Compute one plus the trace of the matrix
        const FT onePlusTrace = FT(1.0) + m(0, 0) + m(1, 1) + m(2, 2);

        if (onePlusTrace > 1E-5) {
            // Direct computation
            const FT s = std::sqrt(onePlusTrace) * FT(2.0);
            _q[0] = (m(2, 1) - m(1, 2)) / s;
            _q[1] = (m(0, 2) - m(2, 0)) / s;
            _q[2] = (m(1, 0) - m(0, 1)) / s;
            _q[3] = FT(0.25) * s;
        }
        else
        {
            // Computation depends on major diagonal term
            if ((m(0, 0) > m(1, 1)) && (m(0, 0) > m(2, 2))) {
                const FT s = std::sqrt(FT(1.0) + m(0, 0) - m(1, 1) - m(2, 2)) * FT(2.0);
                _q[0] = FT(0.25) * s;
                _q[1] = (m(0, 1) + m(1, 0)) / s;
                _q[2] = (m(0, 2) + m(2, 0)) / s;
                _q[3] = (m(1, 2) - m(2, 1)) / s;
            }
            else if (m(1, 1) > m(2, 2)) {
                const FT s = std::sqrt(FT(1.0) + m(1, 1) - m(0, 0) - m(2, 2)) * FT(2.0);
                _q[0] = (m(0, 1) + m(1, 0)) / s;
                _q[1] = FT(0.25) * s;
                _q[2] = (m(1, 2) + m(2, 1)) / s;
                _q[3] = (m(0, 2) - m(2, 0)) / s;
            }
            else {
                const FT s = std::sqrt(FT(1.0) + m(2, 2) - m(0, 0) - m(1, 1)) * FT(2.0);
                _q[0] = (m(0, 2) + m(2, 0)) / s;
                _q[1] = (m(1, 2) + m(2, 1)) / s;
                _q[2] = FT(0.25) * s;
                _q[3] = (m(0, 1) - m(1, 0)) / s;
            }
        }
        normalize();
    }

    template <typename FT>
    void Quat<FT>::set_from_rotated_basis(const Vec3& X, const Vec3& Y, const Vec3& Z)
    {
        Mat3<FT> m;
        FT normX = X.length();
        FT normY = Y.length();
        FT normZ = Z.length();

        for (int i = 0; i < 3; ++i) {
            m(i, 0) = X[i] / normX;
            m(i, 1) = Y[i] / normY;
            m(i, 2) = Z[i] / normZ;
        }

        set_from_rotation_matrix(m);
    }

    template <typename FT>
    void Quat<FT>::get_axis_angle(Vec3& axis, FT& angle) const
    {
        // The normalize() is here to prevent failure introduced by numerical error.
        // We call std::acos(_q[3]), but _q[3] equaling to 1 can actually be e.g., 1.00000012.
        if (_q[3] > FT(1)) {
            const_cast<Quat*>(this)->normalize();
        }
        angle = FT(2.0) * std::acos(_q[3]);
        axis = Vec3(_q[0], _q[1], _q[2]);
        const FT sinus = axis.length();
        if (sinus > 1E-8)
            axis = axis / sinus;

        if (angle > M_PI) {
            angle = 2.0*M_PI - angle;
            axis = -axis;
        }
    }

    template <typename FT>
    typename Quat<FT>::Vec3 Quat<FT>::axis() const
    {
        Vec3 res(_q[0], _q[1], _q[2]);
        const FT sinus = res.length();
        if (sinus > 1E-8)
            res = res / sinus;

        // The normalize() is here to prevent failure introduced by numerical error.
        // We call std::acos(_q[3]), but _q[3] equaling to 1 can actually be e.g., 1.00000012.
        if (_q[3] > FT(1)) {
            const_cast<Quat*>(this)->normalize();
        }
        return (std::acos(_q[3]) <= M_PI / 2.0) ? res : -res;
    }

    template <typename FT>
    FT Quat<FT>::angle() const
    {
        // The normalize() is here to prevent failure introduced by numerical error.
        // We call std::acos(_q[3]), but _q[3] equaling to 1 can actually be e.g., 1.00000012.
        if (_q[3] > FT(1)) {
            const_cast<Quat*>(this)->normalize();
        }
        const FT angle = FT(2.0) * std::acos(_q[3]);
        return (angle <= M_PI) ? angle : FT(2.0*M_PI - angle);
    }


    //Mat4 quat_to_matrix(const Quat& q)
    //{
    //	Mat4 M;
    //
    //	const double x = q.vector()[0];
    //	const double y = q.vector()[1];
    //	const double z = q.vector()[2];
    //	const double w = q.scalar();
    //	const double s = 2 / norm(q);
    //
    //	M(0, 0) = 1 - s*(y*y + z*z); M(0, 1) = s*(x*y - w*z);   M(0, 2) = s*(x*z + w*y);   M(0, 3) = 0;
    //	M(1, 0) = s*(x*y + w*z);   M(1, 1) = 1 - s*(x*x + z*z); M(1, 2) = s*(y*z - w*x);   M(1, 3) = 0;
    //	M(2, 0) = s*(x*z - w*y);   M(2, 1) = s*(y*z + w*x);   M(2, 2) = 1 - s*(x*x + y*y); M(2, 3) = 0;
    //	M(3, 0) = 0;             M(3, 1) = 0;             M(3, 2) = 0;             M(3, 3) = 1;
    //
    //	return M;
    //}
    //
    //Mat4 unit_quat_to_matrix(const Quat& q)
    //{
    //	Mat4 M;
    //
    //	const double x = q.vector()[0];
    //	const double y = q.vector()[1];
    //	const double z = q.vector()[2];
    //	const double w = q.scalar();
    //
    //	M(0, 0) = 1 - 2 * (y*y + z*z); M(0, 1) = 2 * (x*y - w*z);   M(0, 2) = 2 * (x*z + w*y);   M(0, 3) = 0;
    //	M(1, 0) = 2 * (x*y + w*z);   M(1, 1) = 1 - 2 * (x*x + z*z); M(1, 2) = 2 * (y*z - w*x);   M(1, 3) = 0;
    //	M(2, 0) = 2 * (x*z - w*y);   M(2, 1) = 2 * (y*z + w*x);   M(2, 2) = 1 - 2 * (x*x + y*y); M(2, 3) = 0;
    //	M(3, 0) = 0;             M(3, 1) = 0;             M(3, 2) = 0;             M(3, 3) = 1;
    //
    //	return M;
    //}


    template <typename FT>
    Mat4<FT> Quat<FT>::matrix() const
    {
        const FT q00 = FT(2.0l * _q[0] * _q[0]);
        const FT q11 = FT(2.0l * _q[1] * _q[1]);
        const FT q22 = FT(2.0l * _q[2] * _q[2]);

        const FT q01 = FT(2.0l * _q[0] * _q[1]);
        const FT q02 = FT(2.0l * _q[0] * _q[2]);
        const FT q03 = FT(2.0l * _q[0] * _q[3]);

        const FT q12 = FT(2.0l * _q[1] * _q[2]);
        const FT q13 = FT(2.0l * _q[1] * _q[3]);

        const FT q23 = FT(2.0l * _q[2] * _q[3]);

        Mat4<FT> m;
        m(0, 0) = FT(1.0) - q11 - q22;
        m(0, 1) = q01 - q23;
        m(0, 2) = q02 + q13;

        m(1, 0) = q01 + q23;
        m(1, 1) = FT(1.0) - q22 - q00;
        m(1, 2) = q12 - q03;

        m(2, 0) = q02 - q13;
        m(2, 1) = q12 + q03;
        m(2, 2) = FT(1.0) - q11 - q00;

        m(3, 0) = FT(0.0);
        m(3, 1) = FT(0.0);
        m(3, 2) = FT(0.0);

        m(0, 3) = FT(0.0);
        m(1, 3) = FT(0.0);
        m(2, 3) = FT(0.0);
        m(3, 3) = FT(1.0);

        return m;
    }

    template <typename FT>
    Mat4<FT> Quat<FT>::inverse_matrix() const
    {
        return inverse().matrix();
    }

    template <typename FT>
    Quat<FT> Quat<FT>::slerp(const Quat<FT>& a, const Quat<FT>& b, FT t, bool allowFlip)
    {
        FT cosAngle = Quat<FT>::dot(a, b);

        FT c1, c2;
        // Linear interpolation for close orientations
        if ((1.0 - std::fabs(cosAngle)) < 0.01)
        {
            c1 = FT(1.0) - t;
            c2 = t;
        }
        else
        {
            // Spherical interpolation
            FT angle = std::acos(std::fabs(cosAngle));
            FT sinAngle = std::sin(angle);
            c1 = std::sin(angle * (1.0 - t)) / sinAngle;
            c2 = std::sin(angle * t) / sinAngle;
        }

        // Use the shortest path
        if (allowFlip && (cosAngle < 0.0))
            c1 = -c1;

        return Quat(c1*a[0] + c2*b[0], c1*a[1] + c2*b[1], c1*a[2] + c2*b[2], c1*a[3] + c2*b[3]);
    }

    template <typename FT>
    Quat<FT> Quat<FT>::squad(const Quat<FT>& a, const Quat<FT>& tgA, const Quat<FT>& tgB, const Quat<FT>& b, FT t)
    {
        Quat<FT> ab = Quat<FT>::slerp(a, b, t);
        Quat<FT> tg = Quat<FT>::slerp(tgA, tgB, t, false);
        return Quat<FT>::slerp(ab, tg, 2.0*t*(1.0 - t), false);
    }

    template <typename FT>
    Quat<FT> Quat<FT>::log()
    {
        FT len = std::sqrt(_q[0] * _q[0] + _q[1] * _q[1] + _q[2] * _q[2]);

        if (len < 1E-6)
            return Quat(_q[0], _q[1], _q[2], 0.0);
        else
        {
            // The normalize() is here to prevent failure introduced by numerical error.
            // We call std::acos(_q[3]), but _q[3] equaling to 1 can actually be e.g., 1.00000012.
            if (_q[3] > FT(1)) {
                const_cast<Quat*>(this)->normalize();
            }		FT coef = std::acos(_q[3]) / len;
            return Quat(_q[0] * coef, _q[1] * coef, _q[2] * coef, 0.0);
        }
    }

    template <typename FT>
    Quat<FT> Quat<FT>::exp()
    {
        FT theta = std::sqrt(_q[0] * _q[0] + _q[1] * _q[1] + _q[2] * _q[2]);

        if (theta < 1E-6)
            return Quat(_q[0], _q[1], _q[2], std::cos(theta));
        else
        {
            FT coef = std::sin(theta) / theta;
            return Quat(_q[0] * coef, _q[1] * coef, _q[2] * coef, std::cos(theta));
        }
    }

    template <typename FT>
    Quat<FT> Quat<FT>::ln_dif(const Quat& a, const Quat& b)
    {
        Quat<FT> dif = a.inverse()*b;
        dif.normalize();
        return dif.log();
    }

    template <typename FT>
    Quat<FT> Quat<FT>::squad_tangent(const Quat<FT>& before, const Quat<FT>& center, const Quat<FT>& after)
    {
        Quat<FT> l1 = Quat<FT>::ln_dif(center, before);
        Quat<FT> l2 = Quat<FT>::ln_dif(center, after);
        Quat<FT> e;
        for (int i = 0; i < 4; ++i)
            e._q[i] = -0.25 * (l1._q[i] + l2._q[i]);
        e = center*(e.exp());

        // if (Quat<FT>::dot(e,b) < 0.0)
        // e.negate();

        return e;
    }

    template <typename FT>
    Quat<FT> Quat<FT>::random_quat()
    {
        // The rand() function is not very portable and may not be available on your system.
        // Add the appropriate include or replace by an other random function in case of problem.
        FT seed = rand() / (FT)RAND_MAX;
        FT r1 = std::sqrt(1.0f - seed);
        FT r2 = std::sqrt(seed);
        FT t1 = 2.0f * M_PI * (rand() / (FT)RAND_MAX);
        FT t2 = 2.0f * M_PI * (rand() / (FT)RAND_MAX);
        return Quat(std::sin(t1)*r1, std::cos(t1)*r1, std::sin(t2)*r2, std::cos(t2)*r2);
    }

    template <typename FT> inline
    std::ostream& operator<<(std::ostream& os, const Quat<FT>& Q)
    {
        return os << Q[0] << ' ' << Q[1] << ' ' << Q[2] << ' ' << Q[3];
    }

    template <typename FT> inline
    std::istream& operator>>(std::istream& is, Quat<FT>& Q) {
        return is >> Q[0] >> Q[1] >> Q[2] >> Q[3];
    }

}


#endif // EASY3D_QUATERNION_H
