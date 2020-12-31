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


#include <easy3d/renderer/walk_throuth.h>
#include <easy3d/core//model.h>
#include <easy3d/renderer/camera.h>
#include <easy3d/renderer/drawable_points.h>
#include <easy3d/renderer/drawable_lines.h>
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/util/logging.h>


namespace easy3d {


    WalkThrough::WalkThrough(Camera* camera)
            : camera_(camera)
            , ground_plane_normal_(0, 0, 1)
            , height_factor_(0.2f)
            , third_person_forward_factor_(1.8f)
            , path_visible_(false)
            , current_position_idx_(-1)
            , buffer_up_to_date_(false)
            , character_drawable_(nullptr)
            , character_head_drawable_(nullptr)
            , view_direction_base_drawable_(nullptr)
            , view_direction_arraw_drawable_(nullptr)
    {
        kfi_ = new KeyFrameInterpolator(camera_->frame());
    }


    WalkThrough::~WalkThrough() {
        delete_path();
        delete kfi_;
    }


    void WalkThrough::add_key_frame(const Frame &frame) {
        kfi_->addKeyFrame(frame);

        // update scene radius to make sure the path is within the view frustum
        float dist = distance(camera_->sceneCenter(), frame.position());
        if (dist > camera_->sceneRadius())
            camera_->setSceneRadius(dist);

        LOG(INFO) << "a key frame added to camera path";
    }


    void WalkThrough::start_walking(const std::vector<Model *> &scene) {
        Box3 box;
        for (const auto& m : scene)
            box += m->bounding_box();
        scene_box_ = box;
        camera_->setSceneRadius(box.radius() * 1.1f);
        buffer_up_to_date_ = false;
    }


    void WalkThrough::walk_to(const vec3 &pos) {
        const vec3 &head = character_head(pos);
        const vec3 view_dir = head - camera_->position();
        add_position(pos, view_dir);

        // set the pivot point ahead of he new position.
        camera_->setPivotPoint(pos + view_dir);
    }


    void WalkThrough::set_height_factor(float f) {
        height_factor_ = f;
        buffer_up_to_date_ = false;
        move_to(current_position_idx_);
    }


    void WalkThrough::set_third_person_forward_factor(float f) {
        third_person_forward_factor_ = f;
        buffer_up_to_date_ = false;
        move_to(current_position_idx_);
    }


    void WalkThrough::delete_last_position() {
        if (path_.empty())
            return;

        if (current_position_idx_ == path_.size() - 1)
            --current_position_idx_;
        path_.pop_back();
        buffer_up_to_date_ = false;
    }


    void WalkThrough::delete_path() {
        kfi_->deletePath();
        path_.clear();

        if (character_drawable_) {
            delete character_drawable_;
            character_drawable_ = 0;
        }
        if (character_head_drawable_) {
            delete character_head_drawable_;
            character_head_drawable_ = 0;
        }
        if (view_direction_base_drawable_) {
            delete view_direction_base_drawable_;
            view_direction_base_drawable_ = 0;
        }
        if (view_direction_arraw_drawable_) {
            delete view_direction_arraw_drawable_;
            view_direction_arraw_drawable_ = 0;
        }

        current_position_idx_ = -1;
        buffer_up_to_date_ = false;
    }


    void WalkThrough::animate() {
        if (kfi_->interpolationIsStarted())
            kfi_->stopInterpolation();
        else {
            if (kfi_->numberOfKeyFrames() == 0 && !path_.empty()) {
                kfi_->deletePath();
                for (const auto &s : path_)
                    kfi_->addKeyFrame(to_frame(s.first, s.second));
            }
            kfi_->startInterpolation();
        }
    }


    int WalkThrough::move_to(int idx, bool animation /* = true */, float duration /* = 0.5f */) {
        if (path_.empty() || idx < 0 || idx >= path_.size())
            return current_position_idx_;

        const vec3 &pos = path_[idx].first;
        const vec3 &view_dir = path_[idx].second;

        if (animation) {
            const Frame &frame = to_frame(pos, view_dir);
            camera_->interpolateTo(frame, duration);
        } else {
            const vec3 &cam_pos = camera_position(pos, view_dir);
            const vec3 &up_dir = ground_plane_normal_;

            camera_->setPosition(cam_pos);
            camera_->setViewDirection(view_dir);
            camera_->setUpVector(up_dir);
        }

        current_position_idx_ = idx;
        path_modified.send();

        camera_->setPivotPoint(pos);

        // set the pivot point ahead of he new position.
        if (idx + 1 < path_.size()) {
            // set it to be the next position in the front
            camera_->setPivotPoint(path_[idx + 1].first);
        }
        else {
            // set it to be the character height ahead
            camera_->setPivotPoint(pos + normalize(view_dir) * character_height());
        }

        return current_position_idx_;
    }


    void WalkThrough::draw() const {
        if (!path_.empty())
            draw_path();

        if (kfi_->numberOfKeyFrames() > 0)
            kfi_->draw_path(camera_, character_height() * 0.1f);
    }


    // the character will be standing at pos looking in view_dir direction
    void WalkThrough::add_position(const vec3 &pos, const vec3 &view_dir) {
        vec3 dir = view_dir;
        dir.z = 0; // force looking at horizontal direction
        dir.normalize();

        if (!path_.empty())
            path_.back().second = dir;

        path_.push_back({pos, dir});
        buffer_up_to_date_ = false;

        LOG(INFO) << "a standing point added to walk through";

        move_to(path_.size() - 1);
        path_modified.send();
    }


    float WalkThrough::character_height() const {
        if (scene_box_.is_valid())
            return scene_box_.range(2) * height_factor_;
        else
            return camera_->sceneRadius() * height_factor_;
    }


    float WalkThrough::third_person_forward_distance() const {
        return character_height() * third_person_forward_factor_;
    }


    vec3 WalkThrough::character_head(const vec3 &pos) const {
        const vec3 &head = pos + ground_plane_normal_ * character_height();
        return head;
    }


    vec3 WalkThrough::camera_position(const vec3 &pos, const vec3 &view_dir) const {
        const vec3 &cam_pos = character_head(pos) - view_dir * third_person_forward_distance();
        return cam_pos;
    }


    Frame WalkThrough::to_frame(const vec3 &pos, const vec3 &view_dir) const {
        const vec3 &cam_pos = camera_position(pos, view_dir);

        const vec3 &up_dir = ground_plane_normal_;
        vec3 xAxis = cross(view_dir, up_dir);
        if (xAxis.length2() < 1E-10) {    // target is aligned with upVector, this means a rotation around X axis
            // X axis is then unchanged, let's keep it !
            // rightVector() == camera_->frame()->inverseTransformOf(vec3(1.0, 0.0, 0.0));
            xAxis = camera_->rightVector();
        }

        quat orient;
        orient.set_from_rotated_basis(xAxis, up_dir, -view_dir);

        return Frame(cam_pos, orient);
    }


    void WalkThrough::draw_path() const {
        const float character_width = 6.0f;
        if (!character_drawable_) {
            auto d = const_cast<WalkThrough *>(this)->character_drawable_ = new LinesDrawable;
            d->set_impostor_type(LinesDrawable::CYLINDER);
            d->set_line_width(character_width);
            d->set_uniform_coloring(vec4(0.5f, 1.0f, 0.5f, 1.0f));
        }
        if (!character_head_drawable_) {
            auto d = const_cast<WalkThrough *>(this)->character_head_drawable_ = new PointsDrawable;
            d->set_impostor_type(PointsDrawable::SPHERE);
            d->set_point_size(character_width * 2.0f);
            d->set_uniform_coloring(vec4(0.5f, 0.5f, 1.0f, 1.0f));
        }
        if (!view_direction_base_drawable_) {
            auto d = const_cast<WalkThrough *>(this)->view_direction_base_drawable_ = new LinesDrawable;
            d->set_impostor_type(LinesDrawable::CYLINDER);
            d->set_line_width(character_width);
            d->set_uniform_coloring(vec4(1.0f, 0.5f, 0.5f, 1.0f));
        }
        if (!view_direction_arraw_drawable_) {
            auto d = const_cast<WalkThrough *>(this)->view_direction_arraw_drawable_ = new LinesDrawable;
            d->set_impostor_type(LinesDrawable::CONE);
            d->set_line_width(character_width * 2.0f);
            d->set_uniform_coloring(vec4(1.0f, 0.5f, 0.5f, 1.0f));
        }

        if (!buffer_up_to_date_) {
            float height = character_height();
            std::vector<vec3> character_points, head_points, dir_base_points, dir_arrow_points;
            for (std::size_t i = 0; i < path_.size(); ++i) {
                const vec3 &feet = path_[i].first;
                const vec3 &head = character_head(feet);
                character_points.push_back(feet);
                character_points.push_back(head);

                head_points.push_back(head);

                const vec3 &dir = path_[i].second;
                const vec3 &base_end = head + dir * height * 0.3f;
                const vec3 &arrow_end = base_end + dir * height * 0.2f;
                dir_base_points.push_back(head);
                dir_base_points.push_back(base_end);
                dir_arrow_points.push_back(base_end);
                dir_arrow_points.push_back(arrow_end);
            }
            const_cast<WalkThrough *>(this)->character_drawable_->update_vertex_buffer(character_points);
            const_cast<WalkThrough *>(this)->character_head_drawable_->update_vertex_buffer(head_points);
            const_cast<WalkThrough *>(this)->view_direction_base_drawable_->update_vertex_buffer(dir_base_points);
            const_cast<WalkThrough *>(this)->view_direction_arraw_drawable_->update_vertex_buffer(dir_arrow_points);
            const_cast<WalkThrough *>(this)->buffer_up_to_date_ = true;
        }

        // characters
        character_drawable_->draw(camera_, false);
        // heads
        character_head_drawable_->draw(camera_, false);
        // view direction base
        view_direction_base_drawable_->draw(camera_, false);
        // view direction arrow
        view_direction_arraw_drawable_->draw(camera_, false);
    }

}