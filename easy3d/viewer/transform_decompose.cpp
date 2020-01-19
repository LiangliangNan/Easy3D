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

#include <easy3d/viewer/transform_decompose.h>
#include <cmath>


namespace easy3d {

    namespace transform {

        // Code taken from assimp/matrix4x4.inl
        void decompose(const mat4& M, vec3& scaling, mat3& rotation, vec3& translation) {
            const mat4& _this = M;

            // extract translation
            translation.x = _this(0, 3);
            translation.y = _this(1, 3);
            translation.z = _this(2, 3);

            // extract the rows of the matrix
            vec3 vRows[3] = {
                vec3(_this(0, 0), _this(1, 0), _this(2, 0)),
                vec3(_this(0, 1), _this(1, 1), _this(2, 1)),
                vec3(_this(0, 2), _this(1, 2), _this(2, 2))
            };

            // extract the scaling factors
            scaling.x = vRows[0].length();
            scaling.y = vRows[1].length();
            scaling.z = vRows[2].length();

            // and the sign of the scaling
            if (determinant(M) < 0) {
                scaling.x = -scaling.x;
                scaling.y = -scaling.y;
                scaling.z = -scaling.z;
            }

            // and remove all scaling from the matrix
            if (scaling.x)
                vRows[0] /= scaling.x;

            if (scaling.y)
                vRows[1] /= scaling.y;

            if (scaling.z)
                vRows[2] /= scaling.z;

            // build the 3x3 rotation matrix
            rotation = mat3(
                vRows[0].x, vRows[1].x, vRows[2].x,
                vRows[0].y, vRows[1].y, vRows[2].y,
                vRows[0].z, vRows[1].z, vRows[2].z
                );
        }

        // Code taken from assimp/matrix4x4.inl
        void decompose(const mat4& M, vec3& scaling, quat& rotation, vec3& translation) {
            const mat4& _this = M;

            // extract translation
            translation.x = _this(0, 3);
            translation.y = _this(1, 3);
            translation.z = _this(2, 3);

            // extract the rows of the matrix
            vec3 vRows[3] = {
                vec3(_this(0, 0), _this(1, 0), _this(2, 0)),
                vec3(_this(0, 1), _this(1, 1), _this(2, 1)),
                vec3(_this(0, 2), _this(1, 2), _this(2, 2))
            };

            // extract the scaling factors
            scaling.x = vRows[0].length();
            scaling.y = vRows[1].length();
            scaling.z = vRows[2].length();

            // and the sign of the scaling
            if (determinant(M) < 0) {
                scaling.x = -scaling.x;
                scaling.y = -scaling.y;
                scaling.z = -scaling.z;
            }

            // and remove all scaling from the matrix
            if (scaling.x)
                vRows[0] /= scaling.x;

            if (scaling.y)
                vRows[1] /= scaling.y;

            if (scaling.z)
                vRows[2] /= scaling.z;

            // build the 3x3 rotation matrix
            mat3 m(
                vRows[0].x, vRows[1].x, vRows[2].x,
                vRows[0].y, vRows[1].y, vRows[2].y,
                vRows[0].z, vRows[1].z, vRows[2].z);

            // and generate the rotation quaternion from it
            rotation.set_from_rotation_matrix(m);
        }

        // Code taken from assimp/matrix4x4.inl
        void decompose_no_scaling(const mat4& M, mat3& rotation, vec3& translation) {
            const mat4& _this = M;

            // extract translation
            translation.x = _this(0, 3);
            translation.y = _this(1, 3);
            translation.z = _this(2, 3);

            // extract rotation
            rotation = mat3(_this);
        }

        // Code taken from assimp/matrix4x4.inl
        void decompose_no_scaling(const mat4& M, quat& rotation, vec3& translation) {
            const mat4& _this = M;

            // extract translation
            translation.x = _this(0, 3);
            translation.y = _this(1, 3);
            translation.z = _this(2, 3);

            // extract rotation
            rotation.set_from_rotation_matrix(mat3(_this));
        }

        //----------------------- a more general version of decompose() -----------------------------

        // Decomposes a transformation matrix into to its original components(i.e., scaling, rotation, translation, skew and perspective)
        // Code taken from glm/gtx/matrix_decompose.inl (GLM 0.9.8.0 - 2016-09-11)
        // The glm code was original adapted from
        // http://www.opensource.apple.com/source/WebCore/WebCore-514/platform/graphics/transforms/TransformationMatrix.cpp

        namespace detail
        {
            // Make a linear combination of two vectors and return the result.
            // result = (a * ascl) + (b * bscl)
            vec3 combine(const vec3& a, const vec3& b, float ascl, float bscl) {
                return (a * ascl) + (b * bscl);
            }

            vec3 scale(const vec3& v, float desiredLength) {
                return v * desiredLength / length(v);
            }
        }//namespace detail

        bool decompose(const mat4& ModelMatrix, vec3& Scale, quat& Orientation, vec3& Translation, vec3& Skew, vec4& Perspective)
        {
            mat4 LocalMatrix(ModelMatrix);

            // Normalize the matrix.
            if (LocalMatrix(3, 3) == 0)
                return false;

            for (int i = 0; i < 4; ++i)
                for (int j = 0; j < 4; ++j)
                    LocalMatrix(j, i) /= LocalMatrix(3, 3);

            // perspectiveMatrix is used to solve for perspective, but it also provides
            // an easy way to test for singularity of the upper 3x3 component.
            mat4 PerspectiveMatrix(LocalMatrix);

            for (int i = 0; i < 3; i++)
                PerspectiveMatrix(3, i) = 0;
            PerspectiveMatrix(3, 3) = 1;

            /// TODO: Fixme!
            if (determinant(PerspectiveMatrix) == 0)
                return false;

            // First, isolate perspective.  This is the messiest.
            if (LocalMatrix(3, 0) != 0 || LocalMatrix(3, 1) != 0 || LocalMatrix(3, 2) != 0)
            {
                // rightHandSide is the right hand side of the equation.
                vec4 RightHandSide;
                RightHandSide[0] = LocalMatrix(3, 0);
                RightHandSide[1] = LocalMatrix(3, 1);
                RightHandSide[2] = LocalMatrix(3, 2);
                RightHandSide[3] = LocalMatrix(3, 3);

                // Solve the equation by inverting PerspectiveMatrix and multiplying rightHandSide by the inverse.
                // (This is the easiest way, not necessarily the best.)
                mat4 InversePerspectiveMatrix = inverse(PerspectiveMatrix);
                mat4 TransposedInversePerspectiveMatrix = transpose(InversePerspectiveMatrix);

                Perspective = TransposedInversePerspectiveMatrix * RightHandSide;

                // Clear the perspective partition
                LocalMatrix(3, 0) = LocalMatrix(3, 1) = LocalMatrix(3, 2) = 0;
                LocalMatrix(3, 3) = 1;
            }
            else
            {
                // No perspective.
                Perspective = vec4(0, 0, 0, 1);
            }

            // Next take care of translation (easy).
            Translation = vec3(LocalMatrix.col(3));
            // Now remove the translation from the local matrix
            LocalMatrix(0, 3) = 0;
            LocalMatrix(1, 3) = 0;
            LocalMatrix(2, 3) = 0;

            vec3 Row[3], Pdum3;

            // Now get scale and shear.
            for (int i = 0; i < 3; ++i)
                for (int j = 0; j < 3; ++j)
                    Row[i][j] = LocalMatrix(i, j);

            // Compute X scale factor and normalize first row.
            Scale.x = length(Row[0]);// v3Length(Row[0]);

            Row[0] = detail::scale(Row[0], 1);

            // Compute XY shear factor and make 2nd row orthogonal to 1st.
            Skew.z = dot(Row[0], Row[1]);
            Row[1] = detail::combine(Row[1], Row[0], 1, -Skew.z);

            // Now, compute Y scale and normalize 2nd row.
            Scale.y = length(Row[1]);
            Row[1] = detail::scale(Row[1], 1);
            Skew.z /= Scale.y;

            // Compute XZ and YZ shears, orthogonalize 3rd row.
            Skew.y = dot(Row[0], Row[2]);
            Row[2] = detail::combine(Row[2], Row[0], 1, -Skew.y);
            Skew.x = dot(Row[1], Row[2]);
            Row[2] = detail::combine(Row[2], Row[1], 1, -Skew.x);

            // Next, get Z scale and normalize 3rd row.
            Scale.z = length(Row[2]);
            Row[2] = detail::scale(Row[2], 1);
            Skew.y /= Scale.z;
            Skew.x /= Scale.z;

            // At this point, the matrix (in rows[]) is orthonormal.
            // Check for a coordinate system flip.  If the determinant
            // is -1, then negate the matrix and the scaling factors.
            Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
            if (dot(Row[0], Pdum3) < 0)
            {
                for (int i = 0; i < 3; i++) {
                    Scale.x *= -1;
                    Row[i] *= -1;
                }
            }

            // Now, get the rotations out, as described in the gem.

            // FIXME - Add the ability to return either quaternions (which are
            // easier to recompose with) or Euler angles (rx, ry, rz), which
            // are easier for authors to deal with. The latter will only be useful
            // when we fix https://bugs.webkit.org/show_bug.cgi?id=23799, so I
            // will leave the Euler angle code here for now.

            // ret.rotateY = asin(-Row[0][2]);
            // if (cos(ret.rotateY) != 0) {
            //     ret.rotateX = atan2(Row[1][2], Row[2][2]);
            //     ret.rotateZ = atan2(Row[0][1], Row[0][0]);
            // } else {
            //     ret.rotateX = atan2(-Row[2][0], Row[1][1]);
            //     ret.rotateZ = 0;
            // }

            float s, t, x, y, z, w;

            t = Row[0][0] + Row[1][1] + Row[2][2] + 1;

            if (t > 1e-4)
            {
                s = 0.5f / std::sqrt(t);
                w = 0.25f / s;
                x = (Row[2][1] - Row[1][2]) * s;
                y = (Row[0][2] - Row[2][0]) * s;
                z = (Row[1][0] - Row[0][1]) * s;
            }
            else if (Row[0][0] > Row[1][1] && Row[0][0] > Row[2][2])
            {
                s = std::sqrt(1 + Row[0][0] - Row[1][1] - Row[2][2]) * 2; // S=4*qx
                x = 0.25f * s;
                y = (Row[0][1] + Row[1][0]) / s;
                z = (Row[0][2] + Row[2][0]) / s;
                w = (Row[2][1] - Row[1][2]) / s;
            }
            else if (Row[1][1] > Row[2][2])
            {
                s = std::sqrt(1 + Row[1][1] - Row[0][0] - Row[2][2]) * 2; // S=4*qy
                x = (Row[0][1] + Row[1][0]) / s;
                y = 0.25f * s;
                z = (Row[1][2] + Row[2][1]) / s;
                w = (Row[0][2] - Row[2][0]) / s;
            }
            else
            {
                s = std::sqrt(1 + Row[2][2] - Row[0][0] - Row[1][1]) * 2; // S=4*qz
                x = (Row[0][2] + Row[2][0]) / s;
                y = (Row[1][2] + Row[2][1]) / s;
                z = 0.25f * s;
                w = (Row[1][0] - Row[0][1]) / s;
            }

            Orientation.set_value(x, y, z, w);

            return true;
        }

    }

} // namespace easy3d
