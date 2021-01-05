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
#include <easy3d/renderer/key_frame_interpolator.h>
#include <easy3d/renderer/manipulated_camera_frame.h>
#include <easy3d/util/logging.h>



namespace easy3d {


    WalkThrough::WalkThrough(Camera* camera)
            : camera_(camera)
            , status_(STOPPED)
            , ground_plane_normal_(0, 0, 1)
            , height_factor_(0.2f)
            , third_person_forward_factor_(1.8f)
            , path_visible_(false)
            , current_frame_idx_(-1)
    {
        kfi_ = new KeyFrameInterpolator(camera_->frame());
    }


    WalkThrough::~WalkThrough() {
        delete_path();
        delete kfi_;
    }


    void WalkThrough::add_keyframe(const Frame &frame) {
        kfi_->add_keyframe(frame);

        // update scene radius to make sure the path is within the view frustum
        float dist = distance(camera_->sceneCenter(), frame.position());
        if (dist > camera_->sceneRadius())
            camera_->setSceneRadius(dist);

        LOG(INFO) << "a key frame added to camera path";
        path_modified.send();
    }


    void WalkThrough::start_walking(const std::vector<Model *> &scene) {
        if (scene.empty())
            return;
        Box3 box;
        for (const auto& m : scene)
            box += m->bounding_box();
        scene_box_ = box;
        camera_->setSceneRadius(box.radius() * 1.1f);
    }


    void WalkThrough::walk_to(const vec3 &ground_point) {
        if (status_ != WALKING_MODE) {
            LOG(WARNING) << "wrong status: walk_to() is for the walking mode only";
            return;
        }

        const vec3 head = character_head(ground_point);
        vec3 view_dir = head - camera_->position();
        view_dir.z = 0;
        view_dir.normalize();
        const vec3 &cam_pos = head - view_dir * third_person_forward_distance();
        add_keyframe(cam_pos, view_dir);

        // set the pivot point ahead of the character.
        camera_->setPivotPoint(ground_point + view_dir * third_person_forward_distance());
    }


    void WalkThrough::set_height_factor(float f) {
        height_factor_ = f;
        move_to(current_frame_idx_);
    }


    void WalkThrough::set_third_person_forward_factor(float f) {
        third_person_forward_factor_ = f;
        move_to(current_frame_idx_);
    }


    void WalkThrough::delete_last_position() {
        if (kfi_->number_of_keyframes() == 0)
            return;

        kfi_->delete_last_keyframe();
        move_to(kfi_->number_of_keyframes() -1);
    }


    void WalkThrough::delete_path() {
        kfi_->delete_path();
        current_frame_idx_ = -1;
    }


    void WalkThrough::preview() {
        if (kfi_->is_interpolation_started())
            kfi_->stop_interpolation();
        else
            kfi_->start_interpolation();
    }


    int WalkThrough::move_to(int idx, bool animation /* = true */, float duration /* = 0.5f */) {
        if (kfi_->number_of_keyframes() == 0 || idx < 0 || idx >= kfi_->number_of_keyframes())
            return current_frame_idx_;

        const auto frame = kfi_->keyframe(idx);
        if (animation)
            camera_->interpolateTo(frame, duration);
        else
            camera_->frame()->setPositionAndOrientation(frame.position(), frame.orientation());

        current_frame_idx_ = idx;
        path_modified.send();

        vec3 view_dir = camera_->viewDirection();
        view_dir.normalize();
        const vec3 foot = frame.position() + view_dir * third_person_forward_distance() * 2 - vec3(0, 0, 1) * character_height();
        camera_->setPivotPoint(foot);

        return current_frame_idx_;
    }


    void WalkThrough::draw() const {
        if (is_path_visible())
            kfi_->draw_path(camera_, character_height() * 0.1f);
    }


    void WalkThrough::add_keyframe(const vec3 &cam_pos, const vec3 &view_dir) {
        vec3 dir = view_dir;
        dir.z = 0; // force looking at horizontal direction
        dir.normalize();

        add_keyframe(to_frame(cam_pos, dir));
        move_to(kfi_->number_of_keyframes() - 1); // move to the new view point.
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
        return pos + ground_plane_normal_ * character_height();
    }


    Frame WalkThrough::to_frame(const vec3 &pos, const vec3 &view_dir) const {
        const vec3 &up_dir = ground_plane_normal_;
        vec3 xAxis = cross(view_dir, up_dir);
        if (xAxis.length2() < epsilon_sqr<float>()) {    // target is aligned with upVector, this means a rotation around X axis
            // X axis is then unchanged, let's keep it !
            // rightVector() == camera_->frame()->inverseTransformOf(vec3(1.0, 0.0, 0.0));
            xAxis = camera_->rightVector();
        }

        quat orient;
        orient.set_from_rotated_basis(xAxis, up_dir, -view_dir);

        return Frame(pos, orient);
    }

}