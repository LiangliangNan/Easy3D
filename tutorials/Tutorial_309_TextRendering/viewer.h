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

#ifndef EASY3D_TUTORIAL_TEXT_RENDERING_H
#define EASY3D_TUTORIAL_TEXT_RENDERING_H

#include <easy3d/viewer/viewer.h>

#if EASY3D_TUTORIAL_USES_VULKAN
#include <easy3d/renderer/text_layout.h>
#endif

namespace easy3d { class TextRenderer; }

class TutorialTextRendering : public easy3d::Viewer {
public:
#if EASY3D_TUTORIAL_USES_VULKAN
    explicit TutorialTextRendering(const std::string& title, const easy3d::VulkanViewerOptions& options = {});
#else
    explicit TutorialTextRendering(const std::string& title);
#endif
    ~TutorialTextRendering() override;

protected:
    bool key_press_event(int key, int modifiers) override;
#if EASY3D_TUTORIAL_USES_VULKAN
    void post_resize(int width, int height) override;
    void pre_draw() override;
#else
    bool draw() override;
    void init() override;
#endif

private:
#if EASY3D_TUTORIAL_USES_VULKAN
    easy3d::graphics::TextLayoutEngine text_{1024};
    float size_ = 28, spacing_ = 0, line_spacing_ = 0, dpi_ = 0;
    easy3d::graphics::TextAlign align_ = easy3d::graphics::TextAlign::center;
    bool upper_left_ = true, kerning_ = false, dirty_ = true;
#else
    easy3d::TextRenderer* texter_;
    std::vector<easy3d::vec3> colors_;
    float font_size_delta_;
    float line_spacing_;
    int alignment_;
    bool upper_left_;
#endif
};

#endif // EASY3D_TUTORIAL_TEXT_RENDERING_H
