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

#ifndef EASY3D_TUTORIAL_COLLISION_DETECTION_H
#define EASY3D_TUTORIAL_COLLISION_DETECTION_H

#include <easy3d/viewer/viewer.h>
#include <chrono>

// This class demonstrates how to detect intersecting faces of two meshes.

namespace easy3d {
    class Model;
    class SurfaceMesh;
    class Collider;
} // namespace easy3d

class TutorialCollisionDetection : public easy3d::Viewer {
public:
    explicit TutorialCollisionDetection(const std::string &title);
    void clear_scene() override;
    bool delete_model(easy3d::Model *model) override;
    ~TutorialCollisionDetection() override;

protected:
    // Rebuild the collision trees for the two currently attached triangle meshes.
    void initialize_collision();
    bool mouse_press_event(int x, int y, int button, int modifiers) override;
    bool mouse_release_event(int x, int y, int button, int modifiers) override;
    bool focus_event(bool focused) override;
    void post_resize(int width, int height) override;
    bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) override;
    bool key_press_event(int key, int modifiers) override;

    void detect();

private:
    bool collision_models_valid() const;
    void end_model_drag();
    void retire_collision();
    void advance();
    std::unique_ptr<easy3d::Collider> collider_;
    std::weak_ptr<easy3d::SurfaceMesh> collision_models_[2];
    std::weak_ptr<easy3d::SurfaceMesh> drag_model_;
    std::chrono::steady_clock::time_point next_tick_;
    float angle_ = 0;

    easy3d::vec3 model0_color_;
    easy3d::vec3 model1_color_;
};

#endif // EASY3D_TUTORIAL_COLLISION_DETECTION_H
