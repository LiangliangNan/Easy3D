/** \cond LICENSE
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
 \endcond */

#include "viewer.h"

#include <easy3d/core/point_cloud.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/renderer.h>
#include <easy3d/renderer/transform.h>
#include <easy3d/util/string.h>
#include <easy3d/util/file_system.h>
#include <easy3d/util/resource.h>
#include <easy3d/fileio/image_io.h>

using namespace easy3d;

// \cond

// The source photos are shown (and the window sized) at this fraction of their pixel size.
const float scale = 0.3f;

TutorialRealCamera::TutorialRealCamera(const std::string &title,
                                       const std::string &bundler_file,
                                       const std::string &cloud_file)
    : Viewer(title, 800, 600, 1), current_view_(0), cameras_drawable_(nullptr), ray_drawable_(nullptr) {
    // Read the point cloud
    if (add_model(cloud_file)) {
        // Make the reconstructed points large enough to see.
        auto drawable = current_model()->renderer()->get_points_drawable("vertices");
        drawable->set_point_size(5.0f);

        // Read the camera parameters from the bundler file.
        if (read_bundler_file(bundler_file))
            update_cameras_drawable(true); // draw a frustum for every recovered camera
        else
            std::cerr << "failed to load bundler file" << std::endl;

        // Start from a neutral overview of the whole cloud.
        camera()->setUpVector(vec3(0, 1, 0));
        camera()->setViewDirection(vec3(0, 0, -1));
        camera()->showEntireScene();
        load_image();
    } else
        std::cerr << "failed to load point cloud" << std::endl;

    // hint_ is shown in the viewer HUD; it lists the interactive controls.
    hint_ =
        "Press 'Space' to switch views                         \n" "Press 'h' to show/hide the cameras                    \n" "Move cursor on image to show corresponding 3D ray     \n" "Move cursor on scene to show corresponding image point";
}

bool TutorialRealCamera::key_press_event(int key, int modifiers) {
    // Clear any cursor-driven annotations before handling a view change.
    if (ray_drawable_)
        ray_drawable_->set_visible(false);
    cross_visible_ = false;
    pre_draw(); // default snapshot shortcuts must see the cleared screen marker

    if (key == KEY_SPACE) {
        if (!views_.empty()) {
            // Advance to the next recovered view (wrapping around).
            current_view_ = (current_view_ + 1) % static_cast<int>(views_.size());
            const bool ground_truth = true;
            if (KRT_to_camera(current_view_, camera(), ground_truth)) {
                update_cameras_drawable(ground_truth);
                std::cout << "----- view " << current_view_ << ": "
                          << (ground_truth ? "ground truth view" : "calibration view") << std::endl;
                set_title("TutorialRealCamera: View_" + std::to_string(current_view_));
                const CameraPara &c = views_[current_view_];
                // Resize the window to the image's aspect ratio so the overlay is undistorted
                // make sure the aspect ratio (actual size does not matter)
                resize(static_cast<int>(static_cast<float>(c.w) * scale),
                       static_cast<int>(static_cast<float>(c.h) * scale));
            }
        }
        return true;
    } else if (key == KEY_1) {
        if (!views_.empty()) {
            // '1' shows the pose built directly from the calibration parameters.
            const bool ground_truth = false;
            if (KRT_to_camera(current_view_, camera(), ground_truth)) {
                update_cameras_drawable(ground_truth);
                std::cout << "----- view " << current_view_ << ": "
                          << (ground_truth ? "ground truth view" : "calibration view") << std::endl;
                set_title("TutorialRealCamera: View_" + std::to_string(current_view_));
                const CameraPara &c = views_[current_view_];
                // make sure the aspect ratio (actual size does not matter)
                resize(static_cast<int>(static_cast<float>(c.w) * scale),
                       static_cast<int>(static_cast<float>(c.h) * scale));
            }
        }
        return true;
    } else if (key == KEY_2) {
        if (!views_.empty()) {
            // '2' shows the equivalent "ground truth" pose derived from R, t, and the fov.
            const bool ground_truth = true;
            if (KRT_to_camera(current_view_, camera(), ground_truth)) {
                update_cameras_drawable(ground_truth);
                std::cout << "----- view " << current_view_ << ": "
                          << (ground_truth ? "ground truth view" : "calibration view") << std::endl;
                set_title("TutorialRealCamera: View_" + std::to_string(current_view_));
                const CameraPara &c = views_[current_view_];
                // make sure the aspect ratio (actual size does not matter)
                resize(static_cast<int>(static_cast<float>(c.w) * scale),
                       static_cast<int>(static_cast<float>(c.h) * scale));
            }
        }
        return true;
    } else if (key == KEY_H) {
        // Toggle visibility of all the camera frusta.
        if (cameras_drawable_) {
            cameras_drawable_->set_visible(!cameras_drawable_->is_visible());
            update();
        }
        return true;
    } else
        return Viewer::key_press_event(key, modifiers);
}

// Loads the photo for the current view as an RGBA texture used by the 2D overlay.
void TutorialRealCamera::load_image() {
    // Images are named by zero-padded view index, e.g. images/0000.jpg.
    const std::string image_file =
        resource::directory() + "/data/fountain/images/" + string::to_string(current_view_, 4, '0') + ".jpg";
    if (image_view_ == current_view_)
        return; // already loaded this view's image
    image_.reset();
    image_view_ = current_view_;
    int w = 0, h = 0, channels = 0;
    std::vector<unsigned char> rgba;
    if (file_system::is_file(image_file) && ImageIO::load(image_file, rgba, w, h, channels, 4, false)) {
        graphics::Sampler sampler;
        sampler.filter = graphics::TextureFilter::linear;
        image_ = std::make_shared<graphics::Image>(w, h, std::move(rgba), sampler);
    }
    update();
}

// Converts a recovered K [R|t] camera into an Easy3D Camera pose. Two equivalent routes are
// shown: an explicit "ground truth" pose, and the convenience set_from_calibration() helper.
bool TutorialRealCamera::KRT_to_camera(int view_index, Camera *c, bool ground_truth) {
    if (view_index < 0 || view_index >= views_.size()) {
        std::cerr << "Error: invalid view index (" << view_index << ")" << std::endl;
        return false;
    }

    const CameraPara &cam = views_[view_index];

    if (ground_truth) {
        // R: rotation matrix of world frame in camera frame
        // The camera orientation is the inverse rotation (camera frame -> world frame).
        const quat q(inverse(cam.R)); // the inverse rotation represented by a quaternion
        c->setOrientation(q);
        c->setPosition(-q.rotate(cam.t));                               // camera position: -inverse(rot) * t
        // Derive the vertical field of view from fy and the image height:
        // proj[1][1] = 2 fy / h, and fov = 2 atan(1 / proj[1][1]).
        const float proj11 = 2.0f * cam.fy / static_cast<float>(cam.h); // proj[1][1]
        const float fov = 2.0f * std::atan(1.0f / proj11);
        c->setFieldOfView(fov);
    } else {
        // Let Easy3D build position, orientation, and fov directly from the calibration.
        c->set_from_calibration(cam.fx, cam.fy, 0.0f, cam.cx, cam.cy, cam.R, cam.t, true);
    }

    if (c == camera())
        load_image(); // the active view changed, so refresh the overlaid photo

    return true;
}

// Builds a line drawable showing a small frustum for every recovered camera, placed in world
// space using each camera's frame. This makes the capture geometry visible in the 3D scene.
void TutorialRealCamera::update_cameras_drawable(bool ground_truth) {
    if (!cameras_drawable_) {
        cameras_drawable_ = new LinesDrawable("cameras");
        add_drawable(std::shared_ptr<LinesDrawable>(cameras_drawable_)); // add the camera drawable to the viewer
        cameras_drawable_->set_uniform_coloring(vec4(0, 0, 1, 1.0f));
        cameras_drawable_->set_line_width(2.0f);
    }

    std::vector<vec3> vertices;
    for (std::size_t i = 0; i < views_.size(); ++i) {
        // Build a temporary camera for this view, then generate its frustum wireframe.
        Camera c;
        KRT_to_camera(static_cast<int>(i), &c, ground_truth);
        std::vector<vec3> points;
        shape::create_camera(points, c.sceneRadius() * 0.03f, c.fieldOfView(),
                             static_cast<float>(views_[i].h) / static_cast<float>(views_[i].w));
        // Transform the frustum from the camera's local frame into world coordinates.
        const mat4 &m = c.frame()->worldMatrix();
        for (auto &p : points)
            vertices.push_back(m * p);
    }

    cameras_drawable_->update_vertex_buffer(vertices);
}

// Fits the source photo into the window preserving its aspect ratio, anchored near the
// top-left with a DPI-scaled margin. Returns the on-screen rectangle the overlay occupies.
Rect TutorialRealCamera::calculate_image_rect() const {
    if (!image_)
        return {0, 0, 0, 0};
    int tex_w = int(image_->width()), tex_h = int(image_->height());
    const float image_as = static_cast<float>(tex_w) / static_cast<float>(tex_h);
    const float viewer_as = static_cast<float>(framebuffer_width()) / static_cast<float>(framebuffer_height());
    if (image_as < viewer_as) { // thin
        // Image is taller than the window: constrain by height, derive width.
        tex_h = static_cast<int>(static_cast<float>(framebuffer_height()) * scale);
        tex_w = static_cast<int>(static_cast<float>(tex_h) * image_as);
    } else {
        // Image is wider than the window: constrain by width, derive height.
        tex_w = static_cast<int>(static_cast<float>(framebuffer_width()) * scale);
        tex_h = static_cast<int>(static_cast<float>(tex_w) / image_as);
    }

    const float x = 20 * dpi_scaling(), y = 120 * dpi_scaling();
    return Rect(x, x + static_cast<float>(tex_w), y, y + static_cast<float>(tex_h));
}

void TutorialRealCamera::post_resize(int, int) {
    // The cursor marker is in screen space, so it is meaningless after a resize.
    cross_visible_ = false;
}
// Builds the 2D overlays (the source photo, its red border, and an optional green crosshair).
// Geometry is cached and only rebuilt when the image, framebuffer size, rectangle, or marker
// changes, to avoid re-uploading buffers every frame.
void TutorialRealCamera::pre_draw() {
    Viewer::pre_draw();
    if (!image_) {
        if (overlay_image_) {
            set_overlays({});
            image_commands_.clear();
            overlay_image_ = nullptr;
        }
        return;
    }
    const Rect rect = calculate_image_rect();
    const bool changed = overlay_image_ != image_.get() || overlay_width_ != framebuffer_width() ||
                         overlay_height_ != framebuffer_height() || overlay_rect_.x_min() != rect.x_min() ||
                         overlay_rect_.x_max() != rect.x_max() || overlay_rect_.y_min() != rect.y_min() ||
                         overlay_rect_.y_max() != rect.y_max();
    if (changed) {
        std::vector<graphics::Vertex> vertices(4);
        const vec2 corners[] = {vec2(rect.x_min(), rect.y_min()), vec2(rect.x_max(), rect.y_min()),
                                vec2(rect.x_max(), rect.y_max()), vec2(rect.x_min(), rect.y_max())};
        const vec2 uv[] = {vec2(0, 0), vec2(1, 0), vec2(1, 1), vec2(0, 1)};
        for (unsigned i = 0; i < 4; ++i) {
            vertices[i].position = vec3(corners[i], 0);
            vertices[i].texcoord = uv[i];
        }
        graphics::DrawCommand image;
        image.geometry = std::make_shared<graphics::Geometry>(vertices, std::vector<uint32_t>{0, 1, 2, 0, 2, 3});
        image.texture = image_;
        image.model_view_projection = graphics::screen_space_projection(framebuffer_width(), framebuffer_height());
        image_commands_ = {image, graphics::shape::draw_quad_wire(rect, vec4(1, 0, 0, 1), framebuffer_width(),
                                                                  framebuffer_height(), dpi_scaling())};
        overlay_image_ = image_.get();
        overlay_rect_ = rect;
        overlay_width_ = framebuffer_width();
        overlay_height_ = framebuffer_height();
    }
    const bool visible = cross_visible_;
    const bool cross_changed = visible != cross_overlay_visible_ || cross_geometry_dirty_ || changed;
    if (visible && cross_changed) {
        std::vector<vec2> segments;
        for (const auto &p : cross_points_)
            segments.emplace_back(p.x, p.y);
        cross_command_ = graphics::shape::draw_lines_2d(segments, vec4(0, 1, 0, 1), framebuffer_width(),
                                                        framebuffer_height(), 3 * dpi_scaling());
    }
    if (changed || cross_changed) {
        auto commands = image_commands_;
        if (visible)
            commands.push_back(cross_command_);
        set_overlays(std::move(commands));
        cross_overlay_visible_ = visible;
        cross_geometry_dirty_ = false;
    }
}

bool TutorialRealCamera::mouse_free_move_event(int x, int y, int dx, int dy, int modifiers) {
    (void) dx;
    (void) dy;
    (void) modifiers;

    if (current_view_ < 0 || current_view_ >= views_.size()) {
        std::cerr << "Error: invalid view index (" << current_view_ << ")" << std::endl;
        return false;
    }

    const float pixel_x = float(x) * framebuffer_width() / width();
    const float pixel_y = float(y) * framebuffer_height() / height();
    const CameraPara &cam = views_[current_view_];
    const Rect image_rect = calculate_image_rect();
    cross_visible_ = false;
    // cursor is inside the image rectangle
    if (pixel_x >= image_rect.x_min() && pixel_x <= image_rect.x_max() &&
        pixel_y >= image_rect.y_min() && pixel_y <= image_rect.y_max()) {
        const float image_x =
            (pixel_x - image_rect.x_min()) / image_rect.width() * static_cast<float>(cam.w);
        const float image_y =
            (pixel_y - image_rect.y_min()) / image_rect.height() * static_cast<float>(cam.h);
        if (!ray_drawable_) {
            ray_drawable_ = new LinesDrawable("ray");
            add_drawable(std::shared_ptr<LinesDrawable>(ray_drawable_)); // add the ray drawable to the viewer
            ray_drawable_->set_uniform_coloring(vec4(1, 0, 0, 1.0f));
            ray_drawable_->set_line_width(3.0f);
            ray_drawable_->set_impostor_type(easy3d::LinesDrawable::CYLINDER);
        }
        const vec3 pos = camera_pos(cam.R, cam.t);
        const vec3 dir = pixel_to_ray(static_cast<int>(image_x), static_cast<int>(image_y), cam.fx, cam.fy, 0, cam.cx,
                                      cam.cy, cam.R, cam.t, true);
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

                const float size = 10 * dpi_scaling();
                const std::vector<vec3> points = {
                    vec3(screen_x - size, screen_y, 0.5f), vec3(screen_x + size, screen_y, 0.5f),
                    vec3(screen_x, screen_y - size, 0.5f), vec3(screen_x, screen_y + size, 0.5f)};
                cross_points_ = points;
                cross_geometry_dirty_ = true;
                cross_visible_ = true;
            }
        } else {
            cross_visible_ = false;
        }

        update();
    }

    return false;
}

vec3 TutorialRealCamera::camera_pos(const mat3 &R, const vec3 &t) {
    return -inverse(R) * t; // inverse(R) * (vec3(0, 0, 0) - cam.t);
}

vec3 TutorialRealCamera::pixel_to_ray(int image_x,
                                      int image_y,
                                      float fx,
                                      float fy,
                                      float skew,
                                      float cx,
                                      float cy,
                                      const mat3 &R,
                                      const vec3 &t,
                                      bool convert) {
    mat3 K(fx, skew, cx, 0, fy, cy, 0, 0, 1);

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

vec2 TutorialRealCamera::point_to_pixel(const easy3d::vec3 &p,
                                        float fx,
                                        float fy,
                                        float skew,
                                        float cx,
                                        float cy,
                                        const easy3d::mat3 &R,
                                        const easy3d::vec3 &t,
                                        bool convert) {
    mat3 K(fx, skew, cx, 0, fy, cy, 0, 0, 1);
    mat34 Rt;
    Rt.set_col(0, R.col(0));
    Rt.set_col(1, R.col(1));
    Rt.set_col(2, R.col(2));
    Rt.set_col(3, t);

    if (convert) {
        /// @attention The camera coordinates in computer vision goes X right, Y down, Z forward,
        ///               while the camera coordinates in OpenGL goes X right, Y up, Z inward.
        mat3 flip(1.0);
        flip(1, 1) = -1; // invert the Y axis
        flip(2, 2) = -1; // invert the Z axis
        Rt = flip * Rt;
    }

    vec3 q = K * (Rt * vec4(p, 1.0));
    q /= q.z;

    return vec2(q.x, q.y);
}

// \endcond
