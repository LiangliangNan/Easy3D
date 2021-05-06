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

#ifndef EASY3D_RENDERER_WALK_THROUGH_H
#define EASY3D_RENDERER_WALK_THROUGH_H

#include <easy3d/core/types.h>
#include <easy3d/core/signal.h>
#include <easy3d/renderer/frame.h>


namespace easy3d {
    class Frame;
    class Camera;
    class Model;
    class KeyFrameInterpolator;
}

/// \brief WalkThrough enables to interactively creat a camera path for visualizing large scenes or demonstrating
///     3D objects.
/// \details WalkThrough uses KeyFrameInterpolator for generating camera paths based on a set of keyframes.
///     The keyframes can be obtained in two different modes:
///     - Free mode: record a set of viewpoints and view directions as keyframes.
///     - Walking mode: simulate a character walking through a scene and record a walking path as keyframes.
/// \attention The walking mode requires that the scene/object has an upright orientation, i.e., (0, 0, 1).
class WalkThrough {
public:
    WalkThrough(easy3d::Camera *camera);

    virtual ~WalkThrough();

    enum Status {
        STOPPED, WALKING_MODE, FREE_MODE
    };

    Status status() const { return status_; }

    void set_status(Status s) { status_ = s; }

    /// \name Keyframe creation for the free mode
    //@{
    /// adds a key frame to the camera path (for free mode only)
    void add_keyframe(const easy3d::Frame &frame);
    //@}


    /// \name Keyframe creation for the walking mode
    //@{
    /// \brief Starts the walking mode.
    /// \param box The bounding box of the scene, which is used to initialize parameters of the character.
    void start_walking(const std::vector<easy3d::Model *> &scene);

    /// \brief Walks the character to the \c ground_point position.
    /// \details Upon return, the character will be standing at the \p ground_point position looking in a direction
    ///     defined by the character's current head position -> character's next head position.
    void walk_to(const easy3d::vec3 &ground_point);
    //@}


    /// \name Parameters for the walking mode
    //@{

    /// Sets follow up the keyframes.
    void set_follow_up(bool b) { follow_up_ = b; }

    /// Returns whether follow up the keyframes.
    bool follow_up() const { return follow_up_; }

    /// Sets the character's height factor.
    /// \details The height factor specifies the character's relative height with respect to the scene height.
    ///     The character height is defined as the distance between the character eye position and the ground plane.
    ///      Default value is 0.1.
    void set_height_factor(float f);

    /// Returns the character's height factor.
    float height_factor() const { return height_factor_; }

    /// \brief Sets the forward factor for visualizing the character from the 3rd person's perspective.
    /// \details The height factor specifies how far the character is in front of the observer (i.e., camera),
    ///     relative to the character's height. This parameter is mainly for visualization purposes.
    ///     Default value is 1.8.
    void set_third_person_forward_factor(float f);

    /// Returns the third person forward factor.
    float third_person_forward_factor() const { return third_person_forward_factor_; }
    //@}


    /// \name Path modification and property query for the walking mode
    //@{

    /// Deletes the last keyframe from the path.
    void delete_last_keyframe();

    /// Clears the entire walking path, i.e., all positions added by walk_to().
    void delete_path();

    /// \name Walking through, animation, and visualization.
    //@{

    /// Previews the animation definded by the keyframes.
    void preview();

    /// Places the character at the idx_th keyframe of the path and modifies the camera view accordingly.
    /// \return The index of current keyframe.
    int move_to(int idx, bool animation = true, float duration = 0.5f);

    /// Returns the index of current keyframe, which is equal to the return value of move_to (if called).
    int current_keyframe_index() const { return current_frame_idx_; }

    /// Shows/Hides the cameras.
    void set_cameras_visible(bool b) { cameras_visible_ = b; }

    /// Returns whether the cameras are visible.
    bool cameras_visible() const { return cameras_visible_; }

    /// Shows/Hides the path.
    void set_path_visible(bool b) { path_visible_ = b; }

    /// Returns whether the path is visible.
    bool path_visible() const { return path_visible_; }

    /// Draws the camera path and the virtual cameras.
    void draw() const;
    //@}

    /// Returns the keyframe interpolator.
    easy3d::KeyFrameInterpolator *interpolator() const { return kfi_; }

protected:
    void add_keyframe(const easy3d::vec3 &cam_pos, const easy3d::vec3 &view_dir);

    // the actual height of the character
    // defined as the distance between the character eye position and the ground plane.
    float character_height() const;

    // the actual distance from character's eye to the observer's eye (i.e., camera)
    float third_person_forward_distance() const;

    easy3d::vec3 character_head(const easy3d::vec3 &foot) const;

    // convert view direction to orientation
    easy3d::quat to_orientation(const easy3d::vec3 &view_dir) const;

protected:
    easy3d::Camera *camera_;
    easy3d::KeyFrameInterpolator *kfi_;

    Status status_;

    easy3d::Box3 scene_box_;

    // the orientation of the ground plane (currently is vec3(0,0,1)).
    // \todo Allow to modify it to handle arbitrarily oriented scenes/objects
    easy3d::vec3 ground_plane_normal_;

    bool follow_up_;

    // character's height factor.
    // It specifies the character's relative height with respect to the scene height.
    float height_factor_;

    // the forward factor for visualizing the character from the 3rd person's perspective.
    // It specifies how far the character is in front of the observer (i.e., camera), relative to its height.
    float third_person_forward_factor_;

    bool cameras_visible_;
    bool path_visible_;
    int current_frame_idx_;

public:
    easy3d::Signal<> path_modified;
};

#endif // EASY3D_RENDERER_WALK_THROUGH_H
