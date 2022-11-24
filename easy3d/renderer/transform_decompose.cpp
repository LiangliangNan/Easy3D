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

#include <easy3d/renderer/transform_decompose.h>

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
            if (std::abs(scaling.x) > std::numeric_limits<float>::min())
                vRows[0] /= scaling.x;

            if (std::abs(scaling.y) > std::numeric_limits<float>::min())
                vRows[1] /= scaling.y;

            if (std::abs(scaling.z) > std::numeric_limits<float>::min())
                vRows[2] /= scaling.z;

            // build the 3x3 rotation matrix
            rotation = mat3(
                vRows[0].x, vRows[1].x, vRows[2].x,
                vRows[0].y, vRows[1].y, vRows[2].y,
                vRows[0].z, vRows[1].z, vRows[2].z
                );
        }


        void decompose(const mat4& M, vec3& scaling, quat& rotation, vec3& translation) {
            mat3 R;
            decompose(M, scaling, R, translation);

            // and generate the rotation quaternion from it
            rotation.set_from_rotation_matrix(R);
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


        void decompose_no_scaling(const mat4& M, quat& rotation, vec3& translation) {
            mat3 R;
            decompose_no_scaling(M, R, translation);

            // and generate the rotation quaternion from it
            rotation.set_from_rotation_matrix(R);
        }


        //----------------------- a more general version of decompose() -----------------------------

        // Decomposes a transformation matrix into to its original components(i.e., scaling, rotation, translation, skew and perspective)
        // Code taken from glm/gtx/matrix_decompose.inl (GLM 0.9.8.0 - 2016-09-11)
        //      https://github.com/g-truc/glm/blob/master/glm/gtx/matrix_decompose.inl
        // The glm code was originally adapted from
        //      http://www.opensource.apple.com/source/WebCore/WebCore-514/platform/graphics/transforms/TransformationMatrix.cpp
        bool decompose(const mat4& ModelMatrix, vec3& Scale, quat& Orientation, vec3& Translation, vec3& Skew, vec4& Perspective)
        {
            typedef float T;
            
            // Make a linear combination of two vectors and return the result.
            // result = (a * ascl) + (b * bscl)
            auto combine = [](const vec3& a, const vec3& b, float wa, float wb) ->vec3 {
                return (a * wa) + (b * wb);
            };

            mat4 LocalMatrix(ModelMatrix);

            // Normalize the matrix.
            if(epsilon_equal(LocalMatrix(3, 3), static_cast<T>(0), epsilon<T>()))
                return false;

            for(int i = 0; i < 4; ++i)
                for(int j = 0; j < 4; ++j)
                    LocalMatrix(j, i) /= LocalMatrix(3, 3);

            // perspectiveMatrix is used to solve for perspective, but it also provides
            // an easy way to test for singularity of the upper 3x3 component.
            mat4 PerspectiveMatrix(LocalMatrix);

            for(int i = 0; i < 3; i++)
                PerspectiveMatrix(3, i) = static_cast<T>(0);
            PerspectiveMatrix(3, 3) = static_cast<T>(1);

            /// TODO: Fixme!
            if(epsilon_equal(determinant(PerspectiveMatrix), static_cast<T>(0), epsilon<T>()))
                return false;

            // First, isolate perspective.  This is the messiest.
            if (epsilon_not_equal(LocalMatrix(3, 0), static_cast<T>(0), epsilon<T>()) ||
                epsilon_not_equal(LocalMatrix(3, 1), static_cast<T>(0), epsilon<T>()) ||
                epsilon_not_equal(LocalMatrix(3, 2), static_cast<T>(0), epsilon<T>()))
            {
                // rightHandSide is the right-hand side of the equation.
                vec4 RightHandSide;
                RightHandSide[0] = LocalMatrix(3, 0);
                RightHandSide[1] = LocalMatrix(3, 1);
                RightHandSide[2] = LocalMatrix(3, 2);
                RightHandSide[3] = LocalMatrix(3, 3);

                // Solve the equation by inverting PerspectiveMatrix and multiplying
                // rightHandSide by the inverse.  (This is the easiest way, not
                // necessarily the best.)
                mat4 InversePerspectiveMatrix = inverse(PerspectiveMatrix);//   inverse(PerspectiveMatrix, inversePerspectiveMatrix);
                mat4 TransposedInversePerspectiveMatrix = transpose(InversePerspectiveMatrix);//   transposeMatrix4(inversePerspectiveMatrix, transposedInversePerspectiveMatrix);

                Perspective = TransposedInversePerspectiveMatrix * RightHandSide;
                //  v4MulPointByMatrix(rightHandSide, transposedInversePerspectiveMatrix, perspectivePoint);

                // Clear the perspective partition
                LocalMatrix(3, 0) = LocalMatrix(3, 1) = LocalMatrix(3, 2) = static_cast<T>(0);
                LocalMatrix(3, 3) = static_cast<T>(1);
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
            for(int i = 0; i < 3; ++i)
                for(int j = 0; j < 3; ++j)
                    Row[i][j] = LocalMatrix(i, j);

            // Compute X scale factor and normalize first row.
            Scale.x = length(Row[0]);// v3Length(Row[0]);

            Row[0].normalize();

            // Compute XY shear factor and make 2nd row orthogonal to 1st.
            Skew.z = dot(Row[0], Row[1]);
            Row[1] = combine(Row[1], Row[0], static_cast<T>(1), -Skew.z);

            // Now, compute Y scale and normalize 2nd row.
            Scale.y = length(Row[1]);
            Row[1].normalize();
            Skew.z /= Scale.y;

            // Compute XZ and YZ shears, orthogonalize 3rd row.
            Skew.y = dot(Row[0], Row[2]);
            Row[2] = combine(Row[2], Row[0], static_cast<T>(1), -Skew.y);
            Skew.x = dot(Row[1], Row[2]);
            Row[2] = combine(Row[2], Row[1], static_cast<T>(1), -Skew.x);

            // Next, get Z scale and normalize 3rd row.
            Scale.z = length(Row[2]);
            Row[2].normalize();
            Skew.y /= Scale.z;
            Skew.x /= Scale.z;

            // At this point, the matrix (in rows[]) is orthonormal.
            // Check for a coordinate system flip.  If the determinant
            // is -1, then negate the matrix and the scaling factors.
            Pdum3 = cross(Row[1], Row[2]); // v3Cross(row[1], row[2], Pdum3);
            if(dot(Row[0], Pdum3) < 0)
            {
                for(int i = 0; i < 3; i++)
                {
                    Scale[i] *= static_cast<T>(-1);
                    Row[i] *= static_cast<T>(-1);
                }
            }

            // Now, get the rotations out, as described in the gem.
            // FIXME - Add the ability to return either quaternions (which are
            // easier to recompose with) or Euler angles (rx, ry, rz), which
            // are easier for authors to deal with. The latter will only be useful
            // when we fix https://bugs.webkit.org/show_bug.cgi?id=23799, so I
            // will leave the Euler angle code here for now.
            // ret.rotateY = std::asin(-Row[0][2]);
            // if (std::cos(ret.rotateY) != 0) {
            //     ret.rotateX = std::atan2(Row[1][2], Row[2][2]);
            //     ret.rotateZ = std::atan2(Row[0][1], Row[0][0]);
            // } else {
            //     ret.rotateX = std::atan2(-Row[2][0], Row[1][1]);
            //     ret.rotateZ = 0;
            // }
#if 0
            // build the 3x3 rotation matrix
            mat3 R(Row[0].x, Row[0].y, Row[0].z,
                   Row[1].x, Row[1].y, Row[1].z,
                   Row[2].x, Row[2].y, Row[2].z);
            Orientation.set_from_rotation_matrix(R);

#else       // the result is the same as above

            T root, trace = Row[0].x + Row[1].y + Row[2].z;
            if(trace > static_cast<T>(0))
            {
                root = std::sqrt(trace + static_cast<T>(1.0));
                Orientation[3] = static_cast<T>(0.5) * root;    // w
                root = static_cast<T>(0.5) / root;
                Orientation[0] = root * (Row[2].y - Row[1].z);  // x
                Orientation[1] = root * (Row[0].z - Row[2].x);  // y
                Orientation[2] = root * (Row[1].x - Row[0].y);  // z
            } // End if > 0
            else
            {
                static int Next[3] = {1, 2, 0};
                int i = 0;
                if(Row[1].y > Row[0].x) i = 1;
                if(Row[2].z > Row[i][i]) i = 2;
                int j = Next[i];
                int k = Next[j];

                root = std::sqrt(Row[i][i] - Row[j][j] - Row[k][k] + static_cast<T>(1.0));

                Orientation[i] = static_cast<T>(0.5) * root;
                root = static_cast<T>(0.5) / root;
                Orientation[j] = root * (Row[i][j] + Row[j][i]);
                Orientation[k] = root * (Row[i][k] + Row[k][i]);
                Orientation[3] = root * (Row[j][k] - Row[k][j]);    // w
            } // End if <= 0
#endif
            return true;
        }

    }

} // namespace easy3d
