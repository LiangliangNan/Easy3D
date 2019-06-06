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

#include "real_camera.h"

#include <easy3d/core/point_cloud.h>
#include <easy3d/viewer/drawable.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/manipulated_camera_frame.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;


RealCamera::RealCamera(const std::string& title,
                       const std::string& bundler_file,
                       const std::string& cloud_file)
    : Viewer(title)
    , current_view_(0)
{
    // Read the point cloud
    if (open(cloud_file)) {
        auto drawable = current_model()->points_drawable("vertices");
        drawable->set_per_vertex_color(true);
        drawable->set_point_size(5.0f);
        
        // Read the camera parameters from the bundler file.
        if (read_bundler_file(bundler_file))
            create_cameras_drawable(0.2f);
        else
            std::cerr << "Error: failed load bundler file." << std::endl;

        camera()->setViewDirection(vec3(0, 0, -1));
        camera_->showEntireScene();
    }
    else
        std::cerr << "Error: failed load point cloud." << std::endl;

    std::cout << "------------ Real Camera ----------" << std::endl
              << "Press 'Space' to switch views" << std::endl
              << "Press 'H' to show/hide the cameras" << std::endl;
}


bool RealCamera::key_press_event(int key, int modifiers) {
    if (key == GLFW_KEY_SPACE) {
        if (!views_.empty()) {
            current_view_ = (current_view_ + 1) % views_.size();
            if (KRT_to_camera(current_view_, 2, camera())) {
                std::cout << "----- view " << current_view_ << " ------" << std::endl;
                const CameraPara& c = views_[current_view_];
                // make sure the aspect ratio (actual size does not matter)
                resize(c.w * 0.25, c.h * 0.25);
                update();
            }
        }
        return true;
    }
    else if (key == GLFW_KEY_1) {
        if (!views_.empty()) {
            if (KRT_to_camera(current_view_, 1, camera())) {
                std::cout << "----- view " << current_view_ << " ------" << std::endl;
                const CameraPara& c = views_[current_view_];
                // make sure the aspect ratio (actual size does not matter)
                resize(c.w * 0.25, c.h * 0.25);
                update();
            }
        }
        return true;
    }
    else if (key == GLFW_KEY_2) {
        if (!views_.empty()) {
            if (KRT_to_camera(current_view_, 2, camera())) {
                std::cout << "----- view " << current_view_ << " ------" << std::endl;
                const CameraPara& c = views_[current_view_];
                // make sure the aspect ratio (actual size does not matter)
                resize(c.w * 0.25, c.h * 0.25);
                update();
            }
        }
        return true;
    }
    else if (key == GLFW_KEY_H) {
        LinesDrawable* d = current_model()->lines_drawable("cameras");
        if (d) {
            d->set_visible(!d->is_visible());
            update();
        }
        return true;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}


bool RealCamera::KRT_to_camera(std::size_t view_index, int method, Camera* c) {
    if (view_index < 0 || view_index >= views_.size()) {
        std::cerr << "Error: invalid view index (" << view_index << ")" << std::endl;
        return false;
    }
    
    const CameraPara& cam = views_[view_index];
    float fx = cam.fx;
    float fy = cam.fy;
    float cx = cam.cx;
    float cy = cam.cy;
    
    if (method == 1) {
        c->set_from_calibration(
                    fx, fy, 0.0f, cx, cy,
                    vec3(cam.rx, cam.ry, cam.rz),
                    vec3(cam.tx, cam.ty, cam.tz)
                    );
    }
    else if (method == 2) {
        const vec3 rot_vec(-cam.rx, -cam.ry, -cam.rz);
        const float angle = rot_vec.length();
        const quat q(rot_vec / angle, angle);
        c->setOrientation(q);

        const vec3 pos(cam.tx, cam.ty, cam.tz);
        c->setPosition(-q.rotate(pos));

        const float proj_5th = 2.0f * fy / cam.h;
        c->setFieldOfView(2.0f * atan(1.0f / proj_5th));
    }

	return true;
}


void RealCamera::create_cameras_drawable(float scale)
{
    const double halfHeight = scale * 0.07;
    const double halfWidth = halfHeight * 1.3;
    const double dist = halfHeight / tan(double(M_PI) / 8.0);
    
    const double arrowHeight = 1.5 * halfHeight;
    const double baseHeight = 1.2 * halfHeight;
    const double arrowHalfWidth = 0.5 * halfWidth;
    const double baseHalfWidth = 0.3 * halfWidth;
    
    std::vector<vec3> points;
    
    //--------------
    // Frustum outline
    //--------------
    
    // LINE_STRIP
    const vec3 p0(-halfWidth, halfHeight, -dist);
    const vec3 p1(-halfWidth, -halfHeight, -dist);    points.push_back(p0); points.push_back(p1);
    const vec3 p2(0.0, 0.0, 0.0);                    points.push_back(p1); points.push_back(p2);
    const vec3 p3(halfWidth, -halfHeight, -dist);    points.push_back(p2); points.push_back(p3);
    const vec3 p4(-halfWidth, -halfHeight, -dist);    points.push_back(p3); points.push_back(p4);
    
    // LINE_STRIP
    const vec3 q0(halfWidth, -halfHeight, -dist);
    const vec3 q1(halfWidth, halfHeight, -dist);
    const vec3 q2(0.0, 0.0, 0.0);
    const vec3 q3(-halfWidth, halfHeight, -dist);
    const vec3 q4(halfWidth, halfHeight, -dist);
    points.push_back(q0); points.push_back(q1);
    points.push_back(q1); points.push_back(q2);
    points.push_back(q2); points.push_back(q3);
    points.push_back(q3); points.push_back(q4);
    
    //------------------
    // Up arrow
    //------------------
    
    // Base - QUAD
    const vec3 r0(-baseHalfWidth, halfHeight, -dist);
    const vec3 r1(baseHalfWidth, halfHeight, -dist);
    const vec3 r2(baseHalfWidth, baseHeight, -dist);
    const vec3 r3(-baseHalfWidth, baseHeight, -dist);
    points.push_back(r0); points.push_back(r1);
    points.push_back(r1); points.push_back(r2);
    points.push_back(r2); points.push_back(r3);
    points.push_back(r3); points.push_back(r0);
    
    // Arrow - TRIANGLE
    const vec3 a0(0.0, arrowHeight, -dist);
    const vec3 a1(-arrowHalfWidth, baseHeight, -dist);
    const vec3 a2(arrowHalfWidth, baseHeight, -dist);
    points.push_back(a0); points.push_back(a1);
    points.push_back(a1); points.push_back(a2);
    points.push_back(a2); points.push_back(a0);
    
    std::vector<vec3> vertices;
    for (int i = 0; i < views_.size(); ++i) {
        Camera c;
        KRT_to_camera(i, 1, &c);
        const mat4& m = c.frame()->worldMatrix();
        for (auto& p : points) {
            vertices.push_back(m * p);
        }
    }
    LinesDrawable* d = current_model()->lines_drawable("cameras");
    if (!d)
        d = current_model()->add_lines_drawable("cameras");
    d->update_vertex_buffer(vertices);
    d->set_default_color(vec3(0, 0, 1));
}
