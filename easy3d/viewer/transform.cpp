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


#include <easy3d/viewer/transform.h>

#include <cmath>


namespace easy3d {

    namespace transform {

        mat4 ortho(float left, float right, float bottom, float top, float zNear, float zFar) {
            mat4 proj(1.0f);
            proj(0, 0) = 2.0f / (right - left);
            proj(1, 1) = 2.0f / (top - bottom);
            proj(2, 2) = -2.0f / (zFar - zNear);
            proj(0, 3) = -(right + left) / (right - left);
            proj(1, 3) = -(top + bottom) / (top - bottom);
            proj(2, 3) = -(zFar + zNear) / (zFar - zNear);
            return proj;
        }

        mat4 ortho(float left, float right, float bottom, float top) {
            mat4 proj(1.0f);
            proj(0, 0) = 2.0f / (right - left);
            proj(1, 1) = 2.0f / (top - bottom);
            proj(2, 2) = -1.0f;
            proj(0, 3) = -(right + left) / (right - left);
            proj(1, 3) = -(top + bottom) / (top - bottom);
            return proj;
        }

        mat4 frustum(float left, float right, float bottom, float top, float nearVal, float farVal)	{
            mat4 proj(0.0f);
            proj(0, 0) = (2.0f * nearVal) / (right - left);
            proj(1, 1) = (2.0f * nearVal) / (top - bottom);
            proj(0, 2) = (right + left) / (right - left);
            proj(1, 2) = (top + bottom) / (top - bottom);
            proj(2, 2) = -(farVal + nearVal) / (farVal - nearVal);
            proj(3, 2) = -1.0f;
            proj(2, 3) = -(2.0f * farVal * nearVal) / (farVal - nearVal);
            return proj;
        }

        mat4 perspective(float fov_y, float aspect, float zNear, float zFar) {
            //assert(abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
            float tanHalfFovy = tan(fov_y / 2.0f);

            mat4 proj(0.0f);
            proj(0, 0) = 1.0f / (aspect * tanHalfFovy);
            proj(1, 1) = 1.0f / (tanHalfFovy);
            proj(2, 2) = -(zFar + zNear) / (zFar - zNear);
            proj(3, 2) = -1.0f;
            proj(2, 3) = -(2.0f * zFar * zNear) / (zFar - zNear);
            return proj;
        }

        // Creates a right handed perspective projection matrix based on a field of view..
        mat4 perspective(float fov, float width, float height, float zNear, float zFar) {
            float rad = fov;
            float h = std::cos(0.5f * rad) / std::sin(0.5f * rad);
            float w = h * height / width;

            mat4 proj(0.0f);
            proj(0, 0) = w;
            proj(1, 1) = h;
            proj(2, 2) = -(zFar + zNear) / (zFar - zNear);
            proj(3, 2) = -1.0f;
            proj(2, 3) = -(2.0f * zFar * zNear) / (zFar - zNear);
            return proj;
        }

        /*
        // The equivalent conversion from perspective() to frustum() is described in the following code:
        // This creates a symmetric frustum. It converts to 6 params (l, r, b, t, n, f) for frustum()
        // from given 4 params (fovy, aspect, near, far)
        void makeFrustum(double fovy, double aspect, double near, double far)
        {
            const double DEG2RAD = 3.14159265 / 180;
            double tangent = tan(fovy / 2 * DEG2RAD);   // tangent of half fovy
            double height = near * tangent;				// half height of near plane
            double width = height * aspect;				// half width of near plane
            // params: left, right, bottom, top, near, far
            glFrustum(-width, width, -height, height, near, far);
        }
        */

        mat4 infinite_perspective(float fov_y, float aspect, float zNear) {
            float const range = tan(fov_y / 2.0f) * zNear;
            float const left = -range * aspect;
            float const right = range * aspect;
            float const bottom = -range;
            float const top = range;

            mat4 proj(0.0f);
            proj(0, 0) = (2.0f * zNear) / (right - left);
            proj(1, 1) = (2.0f * zNear) / (top - bottom);
            proj(2, 2) = -1.0f;
            proj(3, 2) = -1.0f;
            proj(2, 3) = -2.0f * zNear;
            return proj;
        }

        mat4 viewport(float w, float h) {
            return (mat4::scale(w / 2.0f, -h / 2.0f, 1, 1) * mat4::translation(1, -1, 0));
        }

        vec3 project(const vec3& obj, const mat4& mv, const mat4& proj, const int viewport[4], bool lowerleft /* = true*/) {
            mat4 mvp = proj * mv;
            vec3 vs = mvp * obj * 0.5f + vec3(0.5f);
            vs.x = vs.x * viewport[2] + viewport[0];
            vs.y = vs.y * viewport[3] + viewport[1];

            return vec3(
                vs.x,
                lowerleft ? vs.y : (viewport[3] - 1 - vs.y),
                vs.z
                );
        }

        vec3 project(const vec3& obj, const mat4& mvp, const int viewport[4], bool lowerleft /* = true*/) {
            vec3 vs = mvp * obj * 0.5f + vec3(0.5f);
            vs.x = vs.x * viewport[2] + viewport[0];
            vs.y = vs.y * viewport[3] + viewport[1];

            return vec3(
                vs.x,
                lowerleft ? vs.y : (viewport[3] - 1 - vs.y),
                vs.z
                );
        }

        vec3 unproject(const vec3& win, const mat4& mv, const mat4& proj, const int viewport[4], bool lowerleft /* = true*/) {
            vec3 vs(
                win.x,
                lowerleft ? win.y : (viewport[3] - 1 - win.y),
                win.z
                );

            vs.x = (vs.x - viewport[0]) / viewport[2] * 2.0f - 1.0f;
            vs.y = (vs.y - viewport[1]) / viewport[3] * 2.0f - 1.0f;
            vs.z = vs.z * 2.0f - 1.0f;
            return inverse(proj * mv) * vs;
        }

        vec3 unproject(const vec3& win, const mat4& mvp, const int viewport[4], bool lowerleft /* = true*/) {
            vec3 vs(
                win.x,
                lowerleft ? win.y : (viewport[3] - 1 - win.y),
                win.z
                );

            vs.x = (vs.x - viewport[0]) / viewport[2] * 2.0f - 1.0f;
            vs.y = (vs.y - viewport[1]) / viewport[3] * 2.0f - 1.0f;
            vs.z = vs.z * 2.0f - 1.0f;
            return inverse(mvp) * vs;
        }

        mat4 look_at(const vec3& eye, const vec3& center, const vec3& up) {
            const vec3 f(normalize(center - eye));
            const vec3 s(normalize(cross(f, up)));
            const vec3 u(cross(s, f));

            mat4 view(1.0f);
            view(0, 0) = s.x;
            view(0, 1) = s.y;
            view(0, 2) = s.z;
            view(1, 0) = u.x;
            view(1, 1) = u.y;
            view(1, 2) = u.z;
            view(2, 0) = -f.x;
            view(2, 1) = -f.y;
            view(2, 2) = -f.z;
            view(0, 3) = -dot(s, eye);
            view(1, 3) = -dot(u, eye);
            view(2, 3) = dot(f, eye);
            return view;
        }

        mat4 pick_matrix(const vec2& center, const vec2& delta, const vec4& viewport) {
            //assert(delta.x > 0.0f && delta.y > 0.0f);
            std::cerr << "Warning: not sure if pick_matrix() works for lowerleft or upperleft origined viewport." << std::endl;

            mat4 pick(1.0f);

            if (!(delta.x > 0.0f && delta.y > 0.0f))
                return pick; // Error

            vec3 trans(
                (viewport[2] - 2.0f * (center.x - viewport[0])) / delta.x,
                (viewport[3] - 2.0f * (center.y - viewport[1])) / delta.y,
                0.0f);

            // Translate and scale the picked region to the entire window
            pick *= mat4::translation(trans);
            return pick * mat4::scale(viewport[2] / delta.x, viewport[3] / delta.y, 1.0f, 1.0f);
        }

        mat3 normal_matrix(const mat4& mat) {
            mat3 submv(mat);
            return transpose(inverse(submv));
        }

        mat43 normal_matrix_padded(const mat4& mat) {
            const mat3& N = normal_matrix(mat);
            mat43 result;
            result(0, 0) = N(0, 0);	result(0, 1) = N(0, 1);	result(0, 2) = N(0, 2);
            result(1, 0) = N(1, 0);	result(1, 1) = N(1, 1);	result(1, 2) = N(1, 2);
            result(2, 0) = N(2, 0);	result(2, 1) = N(2, 1);	result(2, 2) = N(2, 2);
            // Set the last row to be zeros because of the column major storage. Otherwise
            // you need to set the last column to be zeros).
            result(3, 0) = 0;			result(3, 1) = 0;			result(3, 2) = 0;
            return result;
        }

    }	

}//namespace easy3d
