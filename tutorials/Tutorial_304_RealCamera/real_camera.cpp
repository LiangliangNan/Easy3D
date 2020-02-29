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

#include "real_camera.h"

#include <easy3d/core/point_cloud.h>
#include <easy3d/viewer/drawable_points.h>
#include <easy3d/viewer/drawable_lines.h>
#include <easy3d/viewer/camera.h>
#include <easy3d/viewer/manipulated_camera_frame.h>
#include <easy3d/viewer/texture.h>
#include <easy3d/viewer/primitives.h>
#include <easy3d/util/string.h>
#include <easy3d/util/file_system.h>
#include <easy3d/fileio/resources.h>

#include <3rd_party/glfw/include/GLFW/glfw3.h>	// for the KEYs


using namespace easy3d;


RealCamera::RealCamera(const std::string& title,
                       const std::string& bundler_file,
                       const std::string& cloud_file)
    : Viewer(title, 4, 3, 2, false, false)
    , current_view_(0)
    , texture_(nullptr)
{
    // Read the point cloud
    if (add_model(cloud_file)) {
        auto drawable = current_model()->points_drawable("vertices");
        drawable->set_per_vertex_color(true);
        drawable->set_point_size(5.0f);
        
        // Read the camera parameters from the bundler file.
        if (read_bundler_file(bundler_file))
            create_cameras_drawable();
        else
            std::cerr << "Error: failed load bundler file." << std::endl;

        camera()->setUpVector(vec3(0, 1, 0));
        camera()->setViewDirection(vec3(0, 0, -1));
        camera_->showEntireScene();
    }
    else
        std::cerr << "Error: failed load point cloud." << std::endl;
}


std::string RealCamera::usage() const {
    return ("------------ Real Camera usage ---------- \n"
            "Press 'Space' to switch views\n"
            "Press 'H' to show/hide the cameras\n"
            "----------------------------------------- \n");
}


bool RealCamera::key_press_event(int key, int modifiers) {
    if (key == GLFW_KEY_SPACE) {
        if (!views_.empty()) {
            current_view_ = (current_view_ + 1) % views_.size();
            if (KRT_to_camera(current_view_, 2, camera())) {
                std::cout << "----- view " << current_view_ << " ------" << std::endl;
                set_title("RealCamera: View_" + std::to_string(current_view_));
                const CameraPara& c = views_[current_view_];
                // make sure the aspect ratio (actual size does not matter)
                resize(c.w * 0.3, c.h * 0.3);
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
                resize(c.w * 0.3, c.h * 0.3);
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
                resize(c.w * 0.3, c.h * 0.3);
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


void RealCamera::load_image() {
    const std::string image_file = resource::directory() + "/data/fountain/images/" + string::from_integer(current_view_, 4, '0') + ".jpg";
    if (file_system::is_file(image_file)) {
            if (texture_)
                delete texture_;
        texture_ = Texture::create(image_file);
    }
    update();
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
        const mat3 K(
                fx, 0.0, cx,
                0,  fy,   cy,
                0,  0,    1);
        const mat4 R = mat4::rotation(cam.rx, cam.ry, cam.rz);
        const mat4 T = mat4::translation(cam.tx, cam.ty, cam.tz);
        mat34 M(1.0);
        M(1, 1) = -1;// invert the y axis
        M(2, 2) = -1;// invert the z axis
        const mat34& proj = K * M * T * R;
        c->set_from_projection_matrix(proj);
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

    load_image();

	return true;
}


void RealCamera::create_cameras_drawable()
{    
    std::vector<vec3> vertices;
    for (std::size_t i = 0; i < views_.size(); ++i) {
        Camera c;
        KRT_to_camera(i, 1, &c);
        std::vector<vec3> points;
        opengl::prepare_camera(points, camera()->sceneRadius() * 0.03f, static_cast<float>(views_[i].h)/views_[i].w);
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
    d->set_line_width(2.0f);
}


void RealCamera::draw() const {
    Viewer::draw();
    draw_image();
}


void RealCamera::draw_image() const {
    if (texture_ == nullptr)
        return;

    int w = texture_->width();
    int h = texture_->height();
    float image_as = w / static_cast<float>(h);
    float viewer_as = width() / static_cast<float>(height());
    if (image_as < viewer_as) {// thin
        h = static_cast<int>(height() * 0.3f);
        w = static_cast<int>(h * image_as);
    }
    else {
        w = static_cast<int>(width() * 0.3f);
        h = static_cast<int>(w / image_as);
    }

    const Rect quad(20, w, 20, h);
    opengl::draw_quad_filled(quad, texture_->id(), width(), height(), -0.9f);
}
