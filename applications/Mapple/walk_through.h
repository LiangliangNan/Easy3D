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

#ifndef EASY3D_MAPPLE_WALK_THROUGH_H
#define EASY3D_MAPPLE_WALK_THROUGH_H

#include <memory>
#include <vector>

#include <easy3d/core/types.h>
#include <easy3d/renderer/frame.h>

namespace easy3d {
    class Camera;
    class KeyFrameInterpolator;
    class Model;
}


/**
 * \brief Camera-path creation tools for the two modes that the free keyframe
 *      recording cannot express: walking through a scene and rotating around a
 *      picked axis.
 * \details Ported from the Qt app (Mapple-Qt/walk_through.h). Unlike the Qt
 *      version it borrows the viewer's camera and keyframe interpolator, so the
 *      keyframes it creates are the ones the Camera Path window, playback and
 *      video recording already operate on, and it does not draw the path
 *      (the viewer draws it).
 *
 *      Free mode keyframes are recorded by the app; walking mode turns clicks on
 *      the scene into ground points (walk_to()); rotate-around-axis mode turns a
 *      picked line into a closed orbit (generate_camera_path()).
 *
 * \attention The walking mode assumes the scene is upright, i.e. (0, 0, 1) is up.
 */
class WalkThrough {
public:
    WalkThrough(easy3d::Camera *camera, easy3d::KeyFrameInterpolator *interpolator);
    ~WalkThrough() = default;

    enum Status {
        STOPPED,            // no path tool is active
        FREE_MODE,          // keyframes are recorded from the current camera pose
        WALKING_MODE,       // clicks on the scene walk the character there
        ROTATE_AROUND_AXIS  // a picked line becomes the orbit axis
    };

    Status status() const { return status_; }
    void set_status(Status s) { status_ = s; }

    /// \name Walking mode
    //@{
    /// Bounds of the visible models, used by the character height and the orbit.
    void set_scene(const std::vector<std::shared_ptr<easy3d::Model>> &scene);

    /// Walks the character to the \p ground_point position and records a keyframe.
    void walk_to(const easy3d::vec3 &ground_point);

    void set_follow_up(bool b) { follow_up_ = b; }
    bool follow_up() const { return follow_up_; }

    /// The character height relative to the scene height (default 0.2).
    void set_height_factor(float f);
    float height_factor() const { return height_factor_; }

    /// The distance from the character's eye to the observer, relative to the
    /// character height (default 1.8).
    void set_third_person_forward_factor(float f);
    float third_person_forward_factor() const { return third_person_forward_factor_; }
    //@}

    /// \name Rotate-around-axis mode
    //@{
    /// Generates the orbit around \p axis, replacing nothing: callers clear the
    /// path first when a replacement is intended.
    void generate_camera_path(const easy3d::Line3 &axis);

    /// Distance from the axis relative to the object height (default 0).
    void set_zoom_out_factor(float v, bool re_generate = false);
    float zoom_out_factor() const { return rotate_zoom_out_factor_; }

    /// Vertical camera offset relative to the object height (default 0).
    void set_vertical_offset_factor(float v, bool re_generate = false);
    float vertical_offset_factor() const { return rotate_vertical_offset_factor_; }

    /// Camera pitch in degrees (default 0).
    void set_pitch_angle(float v, bool re_generate = false);
    float pitch_angle() const { return rotate_pitch_angle_; }

    /// Keyframes per orbit loop (default 10).
    void set_keyframe_samples(int v, bool re_generate = false);
    int keyframe_samples() const { return rotate_keyframe_samples_; }

    /// Number of orbit loops (default 2).
    void set_num_loops(int v, bool re_generate = false);
    int num_loops() const { return rotate_num_loops_; }
    //@}

    /// Removes the last keyframe; in walking mode with follow-up, the camera
    /// returns to the new last keyframe.
    void delete_last_keyframe();

    /// Removes every keyframe.
    void clear();

    /// Moves the camera to the \p idx keyframe immediately and updates the pivot.
    /// \return The index of the keyframe the camera is at.
    int move_to(int idx);

    /// Index of the keyframe the camera was last moved to, or -1.
    int current_keyframe_index() const { return current_frame_idx_; }

    /// The scene bounds last passed to set_scene().
    const easy3d::Box3 &scene_box() const { return scene_box_; }

protected:
    void add_keyframe(const easy3d::Frame &frame);
    void add_keyframe(const easy3d::vec3 &cam_pos, const easy3d::vec3 &view_dir);

    // The character's height, i.e. the distance between its eye and the ground.
    float character_height() const;
    float third_person_forward_distance() const;
    easy3d::vec3 character_head(const easy3d::vec3 &foot) const;
    easy3d::quat to_orientation(const easy3d::vec3 &view_dir) const;

protected:
    easy3d::Camera *camera_;
    easy3d::KeyFrameInterpolator *kfi_;

    Status status_;

    // Walking mode
    easy3d::Box3 scene_box_;
    easy3d::vec3 ground_plane_normal_;   // upright scenes only: (0, 0, 1)
    bool follow_up_;
    float height_factor_;
    float third_person_forward_factor_;

    // Rotate-around-axis mode
    easy3d::Line3 rotate_axis_;
    float rotate_zoom_out_factor_;
    float rotate_vertical_offset_factor_;
    float rotate_pitch_angle_;
    int rotate_keyframe_samples_;
    int rotate_num_loops_;

    int current_frame_idx_;
};

#endif // EASY3D_MAPPLE_WALK_THROUGH_H
