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

#include "key_frame_interpolator.h"
#include <easy3d/renderer/shape.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/util/setting.h>
#include <cmath>
#include <stdexcept>
namespace easy3d {
    namespace vulkan {
        graphics::DrawCommand KeyFrameInterpolatorRenderer::draw_path(KeyFrameInterpolator &path,
                                                                      const Camera &camera,
                                                                      float thickness,
                                                                      const vec4 &color) {
            const auto &samples = path.interpolate();
            if (samples.size() < 2)
                return {};
            const auto mvp = camera.model_view_projection_matrix();
            bool same = path_command_.geometry && samples.size() == path_positions_.size() &&
                        width_ == camera.screen_width() && height_ == camera.screen_height() && thickness_ == thickness;
            for (size_t i = 0; same && i < samples.size(); ++i)
                same = path_positions_[i] == samples[i].position();
            for (unsigned i = 0; same && i < 16; ++i)
                same = path_mvp_[i] == mvp[i];
            if (!same) {
                path_positions_.clear();
                std::vector<vec3> segments;
                for (const auto &sample : samples)
                    path_positions_.push_back(sample.position());
                for (size_t i = 1; i < samples.size(); ++i) {
                    segments.push_back(samples[i - 1].position());
                    segments.push_back(samples[i].position());
                }
                path_command_ =
                    graphics::shape::draw_lines_3d(segments, std::vector<vec3>(segments.size(), vec3(1)), mvp,
                                                   camera.screen_width(), camera.screen_height(), thickness);
                width_ = camera.screen_width();
                height_ = camera.screen_height();
                thickness_ = thickness;
                path_mvp_ = mvp;
            }
            auto result = path_command_;
            result.color = color;
            return result;
        }
        graphics::DrawCommand KeyFrameInterpolatorRenderer::draw_cameras(KeyFrameInterpolator &path,
                                                                         const Camera &camera,
                                                                         float camera_width,
                                                                         const vec4 &color) {
            if (!path.number_of_keyframes())
                return {};
            if (!std::isfinite(camera_width) || camera_width <= 0)
                throw std::invalid_argument("camera marker width must be positive and finite");
            const float aspect = float(camera.screen_height()) / camera.screen_width();
            bool same = camera_command_.geometry && keyframes_.size() == path.number_of_keyframes() &&
                        camera_width_ == camera_width && fov_ == camera.field_of_view() && aspect_ == aspect;
            for (size_t i = 0; same && i < keyframes_.size(); ++i) {
                const auto frame = path.keyframe(i);
                same = keyframes_[i].position() == frame.position();
                for (unsigned j = 0; same && j < 4; ++j)
                    same = keyframes_[i].orientation()[j] == frame.orientation()[j];
            }
            if (!same) {
                keyframes_.clear();
                std::vector<graphics::Vertex> vertices;
                for (size_t i = 0; i < path.number_of_keyframes(); ++i) {
                    const auto frame = path.keyframe(i);
                    keyframes_.push_back(frame);
                    std::vector<vec3> points;
                    std::vector<unsigned> elements;
                    easy3d::shape::create_camera(points, elements, camera_width, camera.field_of_view(), aspect);
                    const auto transform = frame.matrix();
                    // One vertex per facet corner with the facet normal: the markers shade
                    // exactly like the flat-shaded OpenGL triangles.
                    for (size_t t = 0; t + 2 < elements.size(); t += 3) {
                        const vec3 p0 = transform * points[elements[t]];
                        const vec3 p1 = transform * points[elements[t + 1]];
                        const vec3 p2 = transform * points[elements[t + 2]];
                        vec3 normal = cross(p1 - p0, p2 - p0);
                        const float factor = length(normal);
                        normal = factor > 0 ? normal / factor : vec3(0, 0, 1);
                        for (const auto &p : {p0, p1, p2}) {
                            graphics::Vertex v;
                            v.position = p;
                            v.normal = normal;
                            vertices.push_back(v);
                        }
                    }
                }
                camera_command_.geometry = std::make_shared<graphics::Geometry>(std::move(vertices));
                camera_command_.overlay = camera_command_.overlay_depth_test = true;
                camera_width_ = camera_width;
                fov_ = camera.field_of_view();
                aspect_ = aspect;
            }
            auto result = camera_command_;
            result.model_view_projection = camera.model_view_projection_matrix();
            result.color = color;
            result.lighting = result.material_lighting = true;
            result.lighting_two_sides = setting::triangles_drawable_two_side_lighting;
            result.distinct_back_color = true;
            result.back_color = vec4(1, 0, 0, 1);
            result.ambient = setting::material_ambient;
            result.specular = setting::material_specular;
            result.shininess = setting::material_shininess;
            result.eye = camera.position();
            result.light = vec3(inverse(camera.model_view_matrix()) * setting::light_position);
            return result;
        }
    } // namespace vulkan
} // namespace easy3d
