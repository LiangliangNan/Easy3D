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


#include <easy3d/viewer/frustum.h>
#include <easy3d/viewer/transform.h>


namespace easy3d {

    Frustum::Frustum(ProjectionType type)
        : type_(type)
    {
        orient(vec3(0.0f, 0.0f, 1.0f), vec3(0.0f, 0.0f, 0.0f), vec3(0.0f, 1.0f, 0.0f));
        if (type == ORTHO)
            set_ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
        else
            set_perspective(float(M_PI / 4.0f), 1.0f, 0.5f, 100.0f);
    }


    void Frustum::orient(const vec3 &pos, const vec3& at, const vec3& up)
    {
        pos_ = pos;
        at_ = at;
        up_ = up;
    }


    void Frustum::set_ortho(float xmin, float xmax, float ymin, float ymax, float znear, float zfar)
    {
        xmin_ = xmin;
        xmax_ = xmax;
        ymin_ = ymin;
        ymax_ = ymax;
        near_ = znear;
        far_ = zfar;
    }


    void Frustum::set_perspective(float fovy, float aspect, float znear, float zfar)
    {
        fovy_ = fovy;
        ar_ = aspect;
        near_ = znear;
        far_ = zfar;

        //////////////////////////////////////////////////////////////////////////
        // I also set the frustum

        ymax_= near_ * tanf(fovy * 0.5f);
        ymin_ = -ymax_;
        xmin_ = ymin_ * aspect;
        xmax_ = ymax_ * aspect;
    }


    void Frustum::set_frustum(float frustum_width, float frustum_height, float znear, float zfar) {
        ymax_ = frustum_width / 2.0f;
        ymin_ = -ymax_;

        ar_ = frustum_width / frustum_height;
        fovy_ = atanf(ymax_ / znear) * 2.0f;

        xmin_ = ymin_ * ar_;
        xmax_ = ymax_ * ar_;
        near_ = znear;
        far_ = zfar;
    }


    mat4 Frustum::view_matrix() const
    {
        return transform::look_at(pos_, at_, up_);
    }


    mat4 Frustum::projection_matrix() const
    {
        if (type_ == PERSPECTIVE) {
            // These two is equal since I have set the ranges in set_perspective()
            //return transform::perspective(fovy_, ar_, near_, far_);
            return transform::frustum(xmin_, xmax_, ymin_, ymax_, near_, far_);
        }
        else
            return transform::ortho(xmin_, xmax_, ymin_, ymax_, near_, far_);
    }



    vec3 Frustum::near_center() const {
        vec3 view_dir = at_ - pos_;
        view_dir = normalize(view_dir);
        vec3 nc = pos_ + view_dir * near_;	// near center
        return nc;
    }


    vec3 Frustum::far_center() const {
        vec3 view_dir = at_ - pos_;
        view_dir = normalize(view_dir);
        vec3 fc = pos_ + view_dir * far_;	// far center
        return fc;
    }


    // the up/right vector in world space
    vec3 Frustum::up_vector() const {
        vec3 view_dir = at_ - pos_;			view_dir = normalize(view_dir);
        vec3 right = cross(view_dir, up_);	right = normalize(right);
        return right;
    }


    vec3 Frustum::right_vector() const {
        vec3 view_dir = at_ - pos_;			view_dir = normalize(view_dir);
        vec3 right = cross(view_dir, up_);	right = normalize(right);
        vec3 up = cross(right, view_dir);	up = normalize(up);
        return up;
    }


    float Frustum::near_width() const {
        if (type_ == PERSPECTIVE) {
            return near_height() * ar_;
        }
        else
            return xmax_;
    }


    float Frustum::near_height() const {
        if (type_ == PERSPECTIVE) {
            return tanf(fovy_ / 2.0f) * near_;
        }
        else
            return ymax_;
    }


    float Frustum::far_width() const {
        if (type_ == PERSPECTIVE) {
            return far_height() * ar_;
        }
        else
            return xmax_;
    }


    float Frustum::far_height() const {
        if (type_ == PERSPECTIVE) {
            return tanf(fovy_ / 2.0f) * far_;
        }
        else
            return ymax_;
    }


    std::vector<vec3> Frustum::vertices() const
    {
    #ifdef compute_in_local_system
        vec3 view_dir(0, 0, -1);
        vec3 right(1, 0, 0);
        vec3 up(0, 1, 0);
        vec3 nc = view_dir * near_;	// near center
        vec3 fc = view_dir * far_;	// far center
    #else
        vec3 view_dir = at_ - pos_;			view_dir = normalize(view_dir);
        vec3 right = cross(view_dir, up_);	right = normalize(right);
        vec3 up = cross(right, view_dir);	up = normalize(up);
        vec3 nc = pos_ + view_dir * near_;	// near center
        vec3 fc = pos_ + view_dir * far_;	// far center

    #endif

        std::vector<vec3> points(8);
        if (type_ == PERSPECTIVE) {
            // these heights and widths are half the heights and widths of
            // the near and far plane rectangles
            float near_height = tanf(fovy_ / 2.0f) * near_;
            float near_width = near_height * ar_;
            float far_height = tanf(fovy_ / 2.0f) * far_;
            float far_width = far_height * ar_;
            points[0] = nc - right * near_width - up * near_height; // bottom left
            points[1] = nc + right * near_width - up * near_height; // bottom right
            points[2] = nc + right * near_width + up * near_height; // top right
            points[3] = nc - right * near_width + up * near_height;	// top left
            points[4] = fc - right * far_width - up * far_height;	// bottom left
            points[5] = fc + right * far_width - up * far_height;	// bottom right
            points[6] = fc + right * far_width + up * far_height;	// top right
            points[7] = fc - right * far_width + up * far_height;	// top left
        }
        else {
            points[0] = nc + right * xmin_ + up * ymin_;	// bottom left
            points[1] = nc + right * xmax_ + up * ymin_;	// bottom right
            points[2] = nc + right * xmax_ + up * ymax_;	// top right
            points[3] = nc + right * xmin_ + up * ymax_;	// top left
            points[4] = fc + right * xmin_ + up * ymin_;	// bottom left
            points[5] = fc + right * xmax_ + up * ymin_;	// bottom right
            points[6] = fc + right * xmax_ + up * ymax_;	// top right
            points[7] = fc + right * xmin_ + up * ymax_;	// top left
        }

    #ifdef compute_in_local_system
        const mat4& m = inverse(view_matrix());
        for (int i = 0; i < 8; ++i) {
            points[i] = m * points[i];
        }
    #endif

        return points;
    }

}
