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

#include "viewer.h"

#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/opengl.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/texture_manager.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/shader_program.h>
#include <easy3d/renderer/shader_manager.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/util/string.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/resource.h>


using namespace easy3d;


const float scale = 0.3f;

RealCamera::RealCamera(const std::string& title,
                       const std::string& bundler_file,
                       const std::string& cloud_file)
    : Viewer(title, 4, 3, 2, false, false)
    , current_view_(0)
    , texture_(nullptr)
    , cameras_drawable_(nullptr)
    , ray_drawable_(nullptr)
    , cross_drawable_(nullptr)
{
    // Read the point cloud
    if (add_model(cloud_file)) {
        auto drawable = current_model()->renderer()->get_points_drawable("vertices");
        drawable->set_point_size(5.0f);

        // Read the camera parameters from the bundler file.
        if (read_bundler_file(bundler_file))
            update_cameras_drawable(true);
        else
            LOG(ERROR) << "failed to load bundler file";

        camera()->setUpVector(vec3(0, 1, 0));
        camera()->setViewDirection(vec3(0, 0, -1));
        camera_->showEntireScene();
    }
    else
        LOG(ERROR) << "failed to load point cloud";

    usage_string_ =
            "---------------- Real Camera usage ------------------ \n"
            "Press 'Space' to switch views                         \n"
            "Press 'H' to show/hide the cameras                    \n"
            "Move cursor on image to show corresponding 3D ray     \n"
            "Move cursor on scene to show corresponding image point\n"
            "----------------------------------------------------- \n";
}


bool RealCamera::key_press_event(int key, int modifiers) {
    if (ray_drawable_)
        ray_drawable_->set_visible(false);
    if (cross_drawable_)
        cross_drawable_->set_visible(false);

    if (key == KEY_SPACE) {
        if (!views_.empty()) {
            current_view_ = (current_view_ + 1) % static_cast<int>(views_.size());
            const bool ground_truth = true;
            if (KRT_to_camera(current_view_, camera(), ground_truth)) {
                update_cameras_drawable(ground_truth);
                std::cout << "----- view " << current_view_ << ": " << (ground_truth ? "ground truth view" : "calibration view") << std::endl;
                set_title("RealCamera: View_" + std::to_string(current_view_));
                const CameraPara &c = views_[current_view_];
                // make sure the aspect ratio (actual size does not matter)
                resize(static_cast<int>(static_cast<float>(c.w) * scale), static_cast<int>(static_cast<float>(c.h) * scale));
            }
        }
        return true;
    }
    else if (key == KEY_1) {
        if (!views_.empty()) {
            const bool ground_truth = false;
            if (KRT_to_camera(current_view_, camera(), ground_truth)) {
                update_cameras_drawable(ground_truth);
                std::cout << "----- view " << current_view_ << ": " << (ground_truth ? "ground truth view" : "calibration view") << std::endl;
                set_title("RealCamera: View_" + std::to_string(current_view_));
                const CameraPara &c = views_[current_view_];
                // make sure the aspect ratio (actual size does not matter)
                resize(static_cast<int>(static_cast<float>(c.w) * scale), static_cast<int>(static_cast<float>(c.h) * scale));
            }
        }
        return true;
    }
    else if (key == KEY_2) {
        if (!views_.empty()) {
            const bool ground_truth = true;
            if (KRT_to_camera(current_view_, camera(), ground_truth)) {
                update_cameras_drawable(ground_truth);
                std::cout << "----- view " << current_view_ << ": " << (ground_truth ? "ground truth view" : "calibration view") << std::endl;
                set_title("RealCamera: View_" + std::to_string(current_view_));
                const CameraPara &c = views_[current_view_];
                // make sure the aspect ratio (actual size does not matter)
                resize(static_cast<int>(static_cast<float>(c.w) * scale), static_cast<int>(static_cast<float>(c.h) * scale));
            }
        }
        return true;
    }
    else if (key == KEY_H) {
        if (cameras_drawable_) {
            cameras_drawable_->set_visible(!cameras_drawable_->is_visible());
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
        // R: rotation matrix of world frame in camera frame
        const quat q(inverse(cam.R));  // the inverse rotation represented by a quaternion
        c->setOrientation(q);
        c->setPosition(-q.rotate(cam.t));    // camera position: -inverse(rot) * t
        const float proj11 = 2.0f * cam.fy / static_cast<float>(cam.h); // proj[1][1]
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
    if (!cameras_drawable_) {
        cameras_drawable_ = new LinesDrawable("cameras");
        add_drawable(cameras_drawable_); // add the camera drawable to the viewer
        cameras_drawable_->set_uniform_coloring(vec4(0, 0, 1, 1.0f));
        cameras_drawable_->set_line_width(2.0f);
    }

    std::vector<vec3> vertices;
    for (std::size_t i = 0; i < views_.size(); ++i) {
        Camera c;
        KRT_to_camera(static_cast<int>(i), &c, ground_truth);
        std::vector<vec3> points;
        shape::create_camera(points, c.sceneRadius() * 0.03f, c.fieldOfView(), static_cast<float>(views_[i].h)/static_cast<float>(views_[i].w));
        const mat4& m = c.frame()->worldMatrix();
        for (auto& p : points)
            vertices.push_back(m * p);
    }

    cameras_drawable_->update_vertex_buffer(vertices);
}


Rect RealCamera::calculate_image_rect() const {
    if (texture_ == nullptr) {
        LOG_N_TIMES(3, ERROR) << "image not shown";
        return {0, 0, 0, 0};
    }

    int tex_w = texture_->width();
    int tex_h = texture_->height();
    const float image_as = static_cast<float>(tex_w) / static_cast<float>(tex_h);
    const float viewer_as = static_cast<float>(width()) / static_cast<float>(height());
    if (image_as < viewer_as) {// thin
        tex_h = static_cast<int>(static_cast<float>(height()) * scale);
        tex_w = static_cast<int>(static_cast<float>(tex_h) * image_as);
    }
    else {
        tex_w = static_cast<int>(static_cast<float>(width()) * scale);
        tex_h = static_cast<int>(static_cast<float>(tex_w) / image_as);
    }

    return Rect(static_cast<float>(20), static_cast<float>(20 + tex_w), 40.0f, static_cast<float>(40 + tex_h));
}


void RealCamera::post_draw() {
    Viewer::post_draw();

    if (texture_ == nullptr)
        return;

    const Rect image_rect = calculate_image_rect();
    const Rect quad(image_rect.x_min() * dpi_scaling(), image_rect.x_max() * dpi_scaling(),
                    image_rect.y_min() * dpi_scaling(), image_rect.y_max() * dpi_scaling());

    const int w = static_cast<int>(static_cast<float>(width()) * dpi_scaling());
    const int h = static_cast<int>(static_cast<float>(height()) * dpi_scaling());
    shape::draw_quad_filled(quad, texture_->id(), w, h, -0.9f);
    shape::draw_quad_wire(quad, vec4(1.0f, 0.0f, 0.0f, 1.0f), w, h, -0.99f);



    if (cross_drawable_ && cross_drawable_->is_visible()) {
        ShaderProgram *program = ShaderManager::get_program("lines/lines_plain_color");
        if (!program) {
            std::vector<ShaderProgram::Attribute> attributes;
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::POSITION, "vtx_position"));
            attributes.emplace_back(ShaderProgram::Attribute(ShaderProgram::COLOR, "vtx_color"));
            program = ShaderManager::create_program_from_files("lines/lines_plain_color", attributes);
        }
        if (!program)
            return;

        const mat4 &proj = transform::ortho(0.0f, static_cast<float>(width()), static_cast<float>(height()), 0.0f,
                                            0.0f, -1.0f);
        glDisable(GL_DEPTH_TEST);   // always on top
        program->bind();
        program->set_uniform("MVP", proj);
        program->set_uniform("per_vertex_color", false);
        program->set_uniform("default_color", vec4(0.0f, 1.0f, 0.0f, 1.0f));
        cross_drawable_->gl_draw();
        program->release();
        glEnable(GL_DEPTH_TEST);   // restore
    }
}


bool RealCamera::mouse_free_move_event(int x, int y, int dx, int dy, int modifiers) {
    (void) dx;
    (void) dy;
    (void) modifiers;

    if (current_view_ < 0 || current_view_ >= views_.size()) {
        std::cerr << "Error: invalid view index (" << current_view_ << ")" << std::endl;
        return false;
    }

    const CameraPara &cam = views_[current_view_];
    const Rect image_rect = calculate_image_rect();
    // cursor is inside the image rectangle
    if (static_cast<float>(x) >= image_rect.x_min() && static_cast<float>(x) <= image_rect.x_max() && static_cast<float>(y) >= image_rect.y_min() && static_cast<float>(y) <= image_rect.y_max()) {
        const float image_x = (static_cast<float>(x) - image_rect.x_min()) / image_rect.width() * static_cast<float>(cam.w);
        const float image_y = (static_cast<float>(y) - image_rect.y_min()) / image_rect.height() * static_cast<float>(cam.h);
        if (!ray_drawable_) {
            ray_drawable_ = new LinesDrawable("ray");
            add_drawable(ray_drawable_); // add the ray drawable to the viewer
            ray_drawable_->set_uniform_coloring(vec4(0, 1, 0, 1.0f));
            ray_drawable_->set_line_width(3.0f);
            ray_drawable_->set_impostor_type(easy3d::LinesDrawable::CYLINDER);
        }
        const vec3 pos = camera_pos(cam.R, cam.t);
        const vec3 dir = pixel_to_ray(static_cast<int>(image_x), static_cast<int>(image_y), cam.fx, cam.fy, 0, cam.cx, cam.cy, cam.R, cam.t, true);
        const std::vector<vec3> points = {pos, pos + dir};
        ray_drawable_->update_vertex_buffer(points);
        ray_drawable_->set_visible(true);
        update();
    } else {
        if (ray_drawable_)
            ray_drawable_->set_visible(false);

        bool found(false);
        const vec3 p = point_under_pixel(x, y, found);
        if (found) {
            const vec2 q = point_to_pixel(p, cam.fx, cam.fy, 0, cam.cx, cam.cy, cam.R, cam.t);

            // visualize the image point (that must be within the image)
            if (q.x >= 0 && q.x <= static_cast<float>(cam.w) && q.y >= 0 && q.y <= static_cast<float>(cam.h)) {
                const float screen_x = q.x / static_cast<float>(cam.w) * image_rect.width() + image_rect.x_min();
                const float screen_y = q.y / static_cast<float>(cam.h) * image_rect.height() + image_rect.y_min();
                if (!cross_drawable_) {
                    cross_drawable_ = new LinesDrawable("cross");
                    add_drawable(cross_drawable_); // add the cross drawable to the viewer
                    cross_drawable_->set_line_width(3.0f);
                }

#if defined(__APPLE__)
                const float size = 10;
#else
                const float size = static_cast<float>(10 * dpi_scaling());
#endif
                const std::vector<vec3> points = {
                        vec3(screen_x - size, screen_y, 0.5f), vec3(screen_x + size, screen_y, 0.5f),
                        vec3(screen_x, screen_y - size, 0.5f), vec3(screen_x, screen_y + size, 0.5f)
                };
                cross_drawable_->update_vertex_buffer(points);
                cross_drawable_->set_visible(true);
            }
        } else {
            if (cross_drawable_)
                cross_drawable_->set_visible(false);
        }

        update();
    }

    return false;
}


vec3 RealCamera::camera_pos(const mat3 &R, const vec3 &t) {
    return -inverse(R) * t; // inverse(R) * (vec3(0, 0, 0) - cam.t);
}


vec3 RealCamera::pixel_to_ray(int image_x, int image_y, float fx, float fy, float skew, float cx, float cy,
                          const mat3& R, const vec3& t, bool convert) {
    mat3 K(fx, skew, cx,
           0, fy, cy,
           0, 0, 1);

    // image point in the camera coordinate system (because p_image = K * p_cam).
    vec3 P = inverse(K) * vec3(static_cast<float>(image_x), static_cast<float>(image_y), 1);
    if (convert) {
        /// @attention The camera coordinates in computer vision goes X right, Y down, Z forward,
        ///               while the camera coordinates in OpenGL goes X right, Y up, Z inward.
        P.y *= -1;
        P.z *= -1;
    }

    // in the world coordinate system (because p_cam = R * p_world + t)
    P = transpose(R) * (P - t);

    return P - camera_pos(R, t);
}


vec2 RealCamera::point_to_pixel(const easy3d::vec3 &p,
                                float fx, float fy, float skew, float cx, float cy,
                                const easy3d::mat3 &R, const easy3d::vec3 &t, bool convert) {
    mat3 K(fx, skew, cx,
           0, fy, cy,
           0, 0, 1);
    mat34 Rt;
    Rt.set_col(0, R.col(0));
    Rt.set_col(1, R.col(1));
    Rt.set_col(2, R.col(2));
    Rt.set_col(3, t);

    if (convert) {
        /// @attention The camera coordinates in computer vision goes X right, Y down, Z forward,
        ///               while the camera coordinates in OpenGL goes X right, Y up, Z inward.
        mat3 flip(1.0);
        flip(1, 1) = -1;   // invert the Y axis
        flip(2, 2) = -1;   // invert the Z axis
        Rt = flip * Rt;
    }

    vec3 q = K * (Rt * vec4(p, 1.0));
    q /= q.z;

    return vec2(q.x, q.y);
}