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

#ifndef EASY3D_TUTORIAL_POINT_SELECTION_H
#define EASY3D_TUTORIAL_POINT_SELECTION_H

#include <easy3d/viewer/viewer.h>


class PointSelection : public easy3d::Viewer
{
public:
    PointSelection(const std::string& title = "");

protected:
    /// Mouse button press event handler
    bool mouse_press_event(int x, int y, int button, int modifiers) override;
    /// Mouse button release event handler
    bool mouse_release_event(int x, int y, int button, int modifiers) override;
    /// Mouse drag (i.e., a mouse button was pressed) event handler
    bool mouse_drag_event(int x, int y, int dx, int dy, int button, int modifiers) override;

    // This function will be called after the main draw procedure.
    void post_draw() override;

    std::string usage() const override;

private:
    easy3d::Polygon2 polygon_;
};


#endif // EASY3D_TUTORIAL_POINT_SELECTION_H
