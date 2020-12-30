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

#ifndef EASY3D_RENDERER_WALK_THROUGH_H
#define EASY3D_RENDERER_WALK_THROUGH_H

#include <easy3d/core/types.h>
#include <easy3d/core/signal.h>
#include <easy3d/renderer/frame.h>


namespace easy3d {

    class Frame;
    class Camera;
    class LinesDrawable;
    class PointsDrawable;
    class KeyFrameInterpolator;

    /// Walk through a scene (or object) by creating a camera path.
    /// \class WalkThrough easy3d/renderer/walk_through.h
    /// \attention Walk through requires that the object is in upright orientation, i.e., (0, 0, 1).
    class WalkThrough : public Signal {
    public:
        WalkThrough(Camera* camera);
        virtual ~WalkThrough();

        void set_active(bool b) { active_ = b; }
        bool is_active() const { return active_; }

        void set_scene_bbox(const Box3 &box);

        // the character will be standing at pos looking in view_dir direction
        void add_position(const vec3 &pos, const vec3 &view_dir);
        // the character will be standing at pos looking from current 'camera_pos' to character head.
        void add_position(const vec3 &pos);

        // removes the last position
        void delete_last_position();

        // returns the number of positions
        int num_positions() const { return path_.size(); }

        // adds a key frame to the camera path (for free mode only)
        void add_key_frame(const Frame &frame);

        void animate_path();

        void delete_path();

        // w.r.t scene height
        // the actual height of the character
        float character_height() const;
        float height_factor() const { return height_factor_; }
        void set_height_factor(float f);

        // w.r.t character height
        // the actual distance from character's current eye pos to camera pos
        float character_distance() const;
        float distance_factor() const { return distance_factor_; }
        void set_distance_factor(float f);

        // Places the person at idx_th position. Does nothing if the position is not valid.
        // Returns the actual current position index.
        int move_to(int idx, bool animation = true, float duration = 0.5f);
        int current_position() const { return current_position_idx_; }

        bool show_path() const { return show_path_; }
        void set_show_path(bool b)  { show_path_ = b; }

        KeyFrameInterpolator* interpolator() const { return kfi_; }

        void draw() const;

    protected:
        void draw_path() const;
        void draw_character() const;
        vec3 character_head(const vec3 &pos) const;
        vec3 camera_position(const vec3 &pos, const vec3 &view_dir) const;

        // convert (character pos, character view direction) to a key frame
        Frame to_frame(const vec3 &pos, const vec3 &view_dir) const;

    protected:
        Camera* camera_;
        KeyFrameInterpolator* kfi_;

        bool active_;
        Box3 scene_box_;

        // the orientation of the ground plane (currently is vec3(0,0,1)).
        // \todo Allow to modify it to handle arbitrarily oriented scenes/objects
        vec3 ground_plane_normal_;

        // the height of the character w.r.t. the scene height
        float height_factor_;
        // make the character step back a bit to simulate 3rd person view
        // w.r.t. the character's height.
        float distance_factor_;

        bool show_path_;
        int current_position_idx_;

        std::vector<std::pair<vec3, vec3> > path_;
        bool buffer_up_to_date_;

        LinesDrawable *character_drawable_;
        PointsDrawable *character_head_drawable_;
        LinesDrawable *view_direction_base_drawable_;
        LinesDrawable *view_direction_arraw_drawable_;
    };

}

#endif // EASY3D_RENDERER_WALK_THROUGH_H
