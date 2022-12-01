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

#include <unordered_map>

// This class demonstrates how to manipulate an object in the 3D space using the mouse

namespace easy3d {
    class Model;
}

class ManipulationViewer : public easy3d::Viewer {
public:
    explicit ManipulationViewer(const std::string &title);

protected:
    bool mouse_press_event(int x, int y, int button, int modifiers) override;
    bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) override;
    void draw() const override;

private:
    void mark(easy3d::Model *model);

private:
    easy3d::Model* selected_model_;
};


#endif // EASY3D_TUTORIAL_OBJECT_MANIPULATION_VIEWER_H
