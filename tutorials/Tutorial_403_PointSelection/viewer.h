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

#ifndef EASY3D_TUTORIAL_POINT_SELECTION_H
#define EASY3D_TUTORIAL_POINT_SELECTION_H

#include <easy3d/viewer/viewer.h>

namespace easy3d { class PointCloud; }

class TutorialPointSelection : public easy3d::Viewer {
public:
#if EASY3D_TUTORIAL_USES_VULKAN
    explicit TutorialPointSelection(const std::string& title = "PointSelection",
                                    const easy3d::VulkanViewerOptions& options = {});
#else
    explicit TutorialPointSelection(const std::string& title = "PointSelection");
#endif

#if EASY3D_TUTORIAL_USES_VULKAN
protected:
#else
private:
#endif
    bool mouse_press_event(int x, int y, int button, int modifiers) override;
    bool mouse_release_event(int x, int y, int button, int modifiers) override;
    bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) override;
#if EASY3D_TUTORIAL_USES_VULKAN
    bool key_press_event(int key, int modifiers) override;
    bool focus_event(bool focused) override;
    void post_resize(int width, int height) override;
    void pre_draw() override;
#else
    void post_draw() override;
#endif

private:
#if EASY3D_TUTORIAL_USES_VULKAN
    void extend(int x, int y);
    void cancel();
    bool active_ = false, rectangle_ = false, dirty_ = true;
    int button_ = -1;
    std::vector<easy3d::vec2> polygon_;
    std::weak_ptr<easy3d::Model> target_;
    std::shared_ptr<const easy3d::graphics::Geometry> fill_, edge_;
#else
    void mark_selection(easy3d::PointCloud* cloud);
    easy3d::Polygon2 polygon_;
#endif
};

#endif // EASY3D_TUTORIAL_POINT_SELECTION_H
