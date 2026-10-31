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

#ifndef EASY3D_TUTORIAL_OBJECT_MANIPULATION_VIEWER_H
#define EASY3D_TUTORIAL_OBJECT_MANIPULATION_VIEWER_H

#include <easy3d/viewer/viewer.h>
#if EASY3D_TUTORIAL_USES_VULKAN
#include <easy3d/renderer/backends/vulkan/manipulator.h>
#endif


// This class demonstrates how to manipulate an object in the 3D space using the mouse

namespace easy3d {
    class Model;
}

class TutorialObjectManipulation : public easy3d::Viewer {
public:
#if EASY3D_TUTORIAL_USES_VULKAN
    explicit TutorialObjectManipulation(const std::string& title,const easy3d::VulkanViewerOptions& options = {});
#else
    explicit TutorialObjectManipulation(const std::string &title);
#endif

protected:
    bool mouse_press_event(int x, int y, int button, int modifiers) override;
    bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) override;
#if EASY3D_TUTORIAL_USES_VULKAN
    bool mouse_release_event(int x,int y,int button,int modifiers) override;
    bool focus_event(bool focused) override;
    void post_resize(int width,int height) override;
    void pre_draw() override;
#else
    bool draw() override;
#endif

private:
    void mark(easy3d::Model *model);

#if EASY3D_TUTORIAL_USES_VULKAN
    std::shared_ptr<easy3d::Model> selected_model() const;
    void end_model_drag();
    std::weak_ptr<easy3d::Model> selected_model_, drag_model_;
    easy3d::vulkan::ManipulatorRenderer guide_;
#else
    easy3d::Model* selected_model_;
#endif
};


#endif // EASY3D_TUTORIAL_OBJECT_MANIPULATION_VIEWER_H
