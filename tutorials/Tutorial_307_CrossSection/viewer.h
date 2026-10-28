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

#ifndef EASY3D_TUTORIAL_CROSS_SECTION_H
#define EASY3D_TUTORIAL_CROSS_SECTION_H

#include "../viewer_backend.h"

#if EASY3D_TUTORIAL_USES_VULKAN
#include <easy3d/renderer/manipulated_frame.h>
#include <easy3d/renderer/constraint.h>
#endif

namespace easy3d {

class CrossSection : public easy3d::TutorialViewer {
public:
#if EASY3D_TUTORIAL_USES_VULKAN
    explicit CrossSection(const std::string& title, const VulkanViewerOptions& options = {});
#else
    explicit CrossSection(const std::string& title);
#endif

protected:
    bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) override;
#if EASY3D_TUTORIAL_USES_VULKAN
    bool key_press_event(int key, int modifiers) override;
    bool mouse_press_event(int x, int y, int button, int modifiers) override;
    bool mouse_release_event(int x, int y, int button, int modifiers) override;
    void pre_draw() override;

private:
    LocalConstraint constraint_;
    ManipulatedFrame plane_;
    bool enabled_ = true, visible_ = true, section_ = false, dirty_ = true;
    float radius_ = 1;
    std::weak_ptr<Model> last_model_;
    size_t last_count_ = 0;
    std::shared_ptr<const graphics::Geometry> guide_, fill_;
#endif
};

} // namespace easy3d

#endif // EASY3D_TUTORIAL_CROSS_SECTION_H
