/********************************************************************
 * Copyright (C) 2015 Liangliang Nan <liangliang.nan@gmail.com>
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
 ********************************************************************/

#include "viewer.h"

#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/shapes.h>
#include <easy3d/renderer/renderer.h>
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
    , cameras_drwable_(nullptr)
{
    // Read the point cloud
    if (add_model(cloud_file)) {
        auto drawable = current_model()->renderer()->get_points_drawable("vertices");
        drawable->set_point_size(5.0f);

        // Read the camera parameters from the bundler file.
        if (read_bundler_file(bundler_file))
            update_cameras_drawable(true);
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
            const bool ground_truth = true;
            if (KRT_to_camera(current_view_, camera(), ground_truth)) {
                update_cameras_drawable(ground_truth);
                std::cout << "----- view " << current_view_ << ": " << (ground_truth ? "ground truth view" : "calibration view") << std::endl;
                set_title("RealCamera: View_" + std::to_string(current_view_));
                const CameraPara &c = views_[current_view_];
                // make sure the aspect ratio (actual size does not matter)
                resize(c.w * 0.3, c.h * 0.3);
            }
        }
        return true;
    }
    else if (key == GLFW_KEY_1) {
        if (!views_.empty()) {
            const bool ground_truth = false;
            if (KRT_to_camera(current_view_, camera(), ground_truth)) {
                update_cameras_drawable(ground_truth);
                std::cout << "----- view " << current_view_ << ": " << (ground_truth ? "ground truth view" : "calibration view") << std::endl;
                set_title("RealCamera: View_" + std::to_string(current_view_));
                const CameraPara &c = views_[current_view_];
                // make sure the aspect ratio (actual size does not matter)
                resize(c.w * 0.3, c.h * 0.3);
            }
        }
        return true;
    }
    else if (key == GLFW_KEY_2) {
        if (!views_.empty()) {
            const bool ground_truth = true;
            if (KRT_to_camera(current_view_, camera(), ground_truth)) {
                update_cameras_drawable(ground_truth);
                std::cout << "----- view " << current_view_ << ": " << (ground_truth ? "ground truth view" : "calibration view") << std::endl;
                set_title("RealCamera: View_" + std::to_string(current_view_));
                const CameraPara &c = views_[current_view_];
                // make sure the aspect ratio (actual size does not matter)
                resize(c.w * 0.3, c.h * 0.3);
            }
        }
        return true;
    }
    else if (key == GLFW_KEY_H) {
        if (cameras_drwable_) {
            cameras_drwable_->set_visible(!cameras_drwable_->is_visible());
            update();
        }
        return true;
    }
    else
        return Viewer::key_press_event(key, modifiers);
}


void RealCamera::load_image() {
    const std::string image_file = resource::directory() + "/data/fountain/images/" + string::to_string(current_view_, 4, '0') + ".jpg";
    if (file_system::is_file(image_file)) {
        texture_ = TextureManager::request(image_file);
    }
    update();
}


bool RealCamera::KRT_to_camera(int view_index, Camera* c, bool ground_truth) {
    if (view_index < 0 || view_index >= views_.size()) {
        std::cerr << "Error: invalid view index (" << view_index << ")" << std::endl;
        return false;
    }

    const CameraPara& cam = views_[view_index];

    if (ground_truth) {
        const quat q(inverse(cam.R));  // the inverse rotation represented by a quaternion
        c->setOrientation(q);
        c->setPosition(-q.rotate(cam.t));    // camera position: -inverse(rot) * t
        const float proj11 = 2.0f * cam.fy / cam.h; // proj[1][1]
        const float fov = 2.0f * std::atan(1.0f / proj11);
        c->setFieldOfView(fov);
    }
    else {
        c->set_from_calibration(
                cam.fx, cam.fy, 0.0f,
                cam.cx, cam.cy,
                cam.R, cam.t, true);
    }

    load_image();

	return true;
}


void RealCamera::update_cameras_drawable(bool ground_truth)
{
    if (!cameras_drwable_) {
        cameras_drwable_ = new LinesDrawable("cameras");
        add_drawable(cameras_drwable_); // add the camera drawables to the viewer
        cameras_drwable_->set_uniform_coloring(vec4(0, 0, 1, 1.0f));
        cameras_drwable_->set_line_width(2.0f);
    }

    std::vector<vec3> vertices;
    for (std::size_t i = 0; i < views_.size(); ++i) {
        Camera c;
        KRT_to_camera(i, &c, ground_truth);
        std::vector<vec3> points;
        shapes::create_camera(points, c.sceneRadius() * 0.03f, c.fieldOfView(), static_cast<float>(views_[i].h)/views_[i].w);
        const mat4& m = c.frame()->worldMatrix();
        for (auto& p : points)
            vertices.push_back(m * p);
    }
    cameras_drwable_->update_vertex_buffer(vertices);
}


void RealCamera::post_draw() {
    Viewer::post_draw();

    if (texture_ == nullptr)
        return;

    int w = width() * dpi_scaling();
    int h = height() * dpi_scaling();

    int tex_w = texture_->width();
    int tex_h = texture_->height();
    float image_as = tex_w / static_cast<float>(tex_h);
    float viewer_as = width() / static_cast<float>(height());
    if (image_as < viewer_as) {// thin
        tex_h = static_cast<int>(height() * 0.3f);
        tex_w = static_cast<int>(tex_h * image_as);
    }
    else {
        tex_w = static_cast<int>(width() * 0.3f);
        tex_h = static_cast<int>(tex_w / image_as);
    }

    const Rect quad(20 * dpi_scaling(), (20 + tex_w) * dpi_scaling(), 40 * dpi_scaling(), (40 + tex_h) * dpi_scaling());
    shapes::draw_quad_filled(quad, texture_->id(), w, h, -0.9f);
    shapes::draw_quad_wire(quad, vec4(1.0f, 0.0f, 0.0f, 1.0f), w, h, -0.99f);
}