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

#ifndef EASY3D_TUTORIAL_DEPTH_MAP_H
#define EASY3D_TUTORIAL_DEPTH_MAP_H

#include "../viewer_backend.h"


// This DepthMap class visualizes 3D models as depth images.

namespace easy3d {
    class FramebufferObject;
}


class TutorialDepthMap : public easy3d::TutorialViewer
{
public:
#if EASY3D_TUTORIAL_USES_VULKAN
    explicit TutorialDepthMap(const std::string& title = "", const easy3d::VulkanViewerOptions& options = {});
#else
    explicit TutorialDepthMap(const std::string& title = "");
#endif
    ~TutorialDepthMap() override;

protected:
#if EASY3D_TUTORIAL_USES_VULKAN
    void pre_draw() override;
#else
    void draw() const override;
    void generate_depth();
#endif
    void draw_depth() const;

private:
#if EASY3D_TUTORIAL_USES_VULKAN
    mutable int depth_width_ = 0, depth_height_ = 0;
    mutable float depth_dpi_ = 0;
    mutable std::vector<easy3d::graphics::DrawCommand> depth_commands_;
#else
    easy3d::FramebufferObject* fbo_;
#endif
};


#endif // EASY3D_TUTORIAL_DEPTH_MAP_H
