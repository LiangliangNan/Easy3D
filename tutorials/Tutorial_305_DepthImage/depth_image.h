/*
*	Copyright (C) 2015 by Liangliang Nan (liangliang.nan@gmail.com)
*	https://3d.bk.tudelft.nl/liangliang/
*
*	This file is part of Easy3D. If it is useful in your research/work,
*   I would be grateful if you show your appreciation by citing it:
*   ------------------------------------------------------------------
*           Liangliang Nan.
*           Easy3D: a lightweight, easy-to-use, and efficient C++
*           library for processing and rendering 3D data. 2018.
*   ------------------------------------------------------------------
*
*	Easy3D is free software; you can redistribute it and/or modify
*	it under the terms of the GNU General Public License Version 3
*	as published by the Free Software Foundation.
*
*	Easy3D is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef EASY3D_TUTORIAL_DEPTH_IMAGE_H
#define EASY3D_TUTORIAL_DEPTH_IMAGE_H

#include <easy3d/viewer/viewer.h>


// This DepthImage class visualizes 3D models as depth images.

namespace easy3d {
    class FramebufferObject;
}


class DepthImage : public easy3d::Viewer
{
public:
    DepthImage(const std::string& title, const std::string& model_file);
    
protected:
    bool key_press_event(int key, int modifiers) override;
    void draw() override;
    void cleanup() override;

    void generate_depth();
    void draw_depth();

private:
    bool depth_rendering_;

    easy3d::FramebufferObject*	fbo_;
};


#endif // EASY3D_TUTORIAL_DEPTH_IMAGE_H
